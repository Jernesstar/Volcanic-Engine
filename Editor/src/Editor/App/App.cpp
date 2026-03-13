#include "App.h"

#include <filesystem>
#include <charconv>

#include <uWebSockets/src/App.h>

#define RAPIDJSON_ASSERT(x) ((void)0)
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "Editor.h"
#include "../Asset/AssetManager.h"

namespace fs = std::filesystem;

u32 FRAME_W = 1280, FRAME_H = 720;

namespace VolcanicEditor {

std::thread s_ServerThread;
void FrontendServer();

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ .Name = "Editor", .TickRate = 120 },
		{
			.Title = "Editor", .Width = FRAME_W, .Height = FRAME_H,
			.Hidden = args.Has("--embedded")
		}
	)
{
	Editor::Init(args);
	s_ServerThread = std::thread(FrontendServer);
}

EditorApp::~EditorApp() {
	Editor::Close();
}

void EditorApp::OnUpdate(TimeStep ts) {
	Editor::Update(ts);
	Editor::Render();
}

static std::string JsonStr(const std::string& s) {
	std::string out = "\"";
	for(char c : s) {
		if(c == '"')       out += "\\\"";
		else if(c == '\\') out += "\\\\";
		else               out += c;
	}
	return out + "\"";
}

static std::string AssetTypeStr(AssetType t) {
	switch(t) {
		case AssetType::Mesh:     return "Mesh";
		case AssetType::Texture:  return "Texture";
		case AssetType::Cubemap:  return "Cubemap";
		case AssetType::Audio:    return "Audio";
		case AssetType::Script:   return "Script";
		case AssetType::Shader:   return "Shader";
		case AssetType::Material: return "Material";
		default:                  return "None";
	}
}

static std::string SerializeAsset(EditorAssetManager* mgr, const Asset& asset) {
	return "{"
		"\"id\":"   + std::to_string((uint64_t)asset.ID) + ","
		"\"type\":" + JsonStr(AssetTypeStr(asset.Type))  + ","
		"\"name\":" + JsonStr(mgr->GetRegistry()->GetAssetName(asset))  + ","
		"\"path\":" + JsonStr(mgr->GetPath(asset.ID))    +
	"}";
}

static std::string SerializeFileTree(const fs::path& root) {
	std::string out = "{"
		"\"name\":"  + JsonStr(root.filename().string()) + ","
		"\"path\":"  + JsonStr(root.generic_string())    + ","
		"\"isDir\":true,"
		"\"children\":[";

	bool first = true;
	std::vector<fs::path> dirs, files;
	for(auto& entry : fs::directory_iterator(root)) {
		if(entry.is_directory()) dirs.push_back(entry.path());
		else                     files.push_back(entry.path());
	}
	for(auto& d : dirs) {
		if(!first) out += ",";
		out  += SerializeFileTree(d);
		first = false;
	}
	for(auto& f : files) {
		if(!first) out += ",";
		out += "{"
			"\"name\":"     + JsonStr(f.filename().string()) + ","
			"\"path\":"     + JsonStr(f.generic_string())    + ","
			"\"isDir\":false,"
			"\"children\":[]"
		"}";
		first = false;
	}
	return out + "]}";
}

static std::string SerializeMaterial(const Material& mat) {
	auto intArr = [](const std::string& key, const Map<std::string, int>& m) {
		std::string out = JsonStr(key) + ":[";
		bool first = true;
		for(auto& [name, val] : m) {
			if(!first) out += ",";
			out  += "{\"name\":" + JsonStr(name) + ",\"value\":" + std::to_string(val) + "}";
			first = false;
		}
		return out + "]";
	};
	auto floatArr = [](const std::string& key, const Map<std::string, float>& m) {
		std::string out = JsonStr(key) + ":[";
		bool first = true;
		for(auto& [name, val] : m) {
			if(!first) out += ",";
			out  += "{\"name\":" + JsonStr(name) + ",\"value\":" + std::to_string(val) + "}";
			first = false;
		}
		return out + "]";
	};
	auto vecArr = [](const std::string& key, const auto& m, auto serialize) {
		std::string out = JsonStr(key) + ":[";
		bool first = true;
		for(auto& [name, val] : m) {
			if(!first) out += ",";
			out  += "{\"name\":" + JsonStr(name) + ",\"value\":" + serialize(val) + "}";
			first = false;
		}
		return out + "]";
	};

	return "{"
		+ intArr("intUniforms", mat.IntUniforms)     + ","
		+ floatArr("floatUniforms", mat.FloatUniforms) + ","
		+ vecArr("vec2Uniforms", mat.Vec2Uniforms,
			[](const Vec2& v) { return "[" + std::to_string(v.x) + "," + std::to_string(v.y) + "]"; }) + ","
		+ vecArr("vec3Uniforms", mat.Vec3Uniforms,
			[](const Vec3& v) { return "[" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "]"; }) + ","
		+ vecArr("vec4Uniforms", mat.Vec4Uniforms,
			[](const Vec4& v) { return "[" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w) + "]"; }) + ","
		+ vecArr("textureUniforms", mat.TextureUniforms,
			[](uint64_t id) { return std::to_string(id); })
	+ "}";
}

// ─── Response helpers ─────────────────────────────────────────────────────────

// uWS requires all headers to be written before the body.
// WriteHeaders must be called first, then res->end(body).
template<typename Res>
static void WriteHeaders(Res* res, int status = 200,
                         std::string_view contentType = "application/json") {
	res->writeStatus(status == 200 ? "200 OK"
	               : status == 204 ? "204 No Content"
	               : status == 400 ? "400 Bad Request"
	               : status == 404 ? "404 Not Found"
	                               : "500 Internal Server Error");
	res->writeHeader("Content-Type",                 contentType);
	res->writeHeader("Access-Control-Allow-Origin",  "*");
	res->writeHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
	res->writeHeader("Access-Control-Allow-Headers", "Content-Type");
}

template<typename Res>
static void JSON(Res* res, const std::string& body) {
	WriteHeaders(res);
	res->end(body);
}

template<typename Res>
static void OK(Res* res) {
	JSON(res, "{\"ok\":true}");
}

template<typename Res>
static void BadRequest(Res* res) {
	WriteHeaders(res, 400);
	res->end("{\"error\":\"bad request\"}");
}

template<typename Res>
static void NotFound(Res* res) {
	WriteHeaders(res, 404);
	res->end("{\"error\":\"not found\"}");
}

// ─── Body reading helper ──────────────────────────────────────────────────────

// uWS streams request bodies in chunks; this accumulates them and fires cb
// with the completed body string.
template<typename Res>
static void ReadBody(Res* res, std::function<void(std::string)> cb) {
	auto buf = std::make_shared<std::string>();
	res->onData([res, buf, cb = std::move(cb)](std::string_view chunk, bool last) mutable {
		buf->append(chunk);
		if(last)
			cb(std::move(*buf));
	});
	res->onAborted([]{});
}

// Parses the last path segment as a uint64 id.
// e.g. "/assets/42/material" -> 42
static bool ParseId(std::string_view url, uint64_t& out) {
	// Walk backwards past any trailing segment, then grab the number
	auto end = url.rfind('/');
	if(end == std::string_view::npos) end = url.size();
	// If the last segment is not a number (e.g. "material"), look one level up
	std::string_view seg = url.substr(url.rfind('/', end - 1) + 1, end - url.rfind('/', end - 1) - 1);
	auto [ptr, ec] = std::from_chars(seg.data(), seg.data() + seg.size(), out);
	return ec == std::errc{};
}

// ─── Server ───────────────────────────────────────────────────────────────────

void FrontendServer() {
	uWS::App()

	// ── CORS preflight ────────────────────────────────────────────────────
	.options("/*", [](auto* res, auto*) {
		WriteHeaders(res, 204);
		res->end();
	})

	// ── Project ───────────────────────────────────────────────────────────
	.get("/project/settings", [](auto* res, auto*) {
		auto& proj = Editor::GetProject();
		JSON(res, "{"
			"\"name\":"        + JsonStr(proj.Name)        + ","
			"\"startScreen\":" + JsonStr(proj.StartScreen) +
		"}");
	})

	.post("/project/settings", [](auto* res, auto*) {
		ReadBody(res, [res](std::string body) {
			rapidjson::Document doc;
			doc.Parse(body.c_str());
			auto& proj = Editor::GetProject();
			if(doc.HasMember("startScreen"))
				proj.StartScreen = doc["startScreen"].GetString();
			OK(res);
		});
	})

	// ── Scene lifecycle ───────────────────────────────────────────────────
	.post("/scene/new", [](auto* res, auto*) {
		ReadBody(res, [res](std::string body) {
			rapidjson::Document doc;
			doc.Parse(body.c_str());
			std::string name = doc.HasMember("name") ? doc["name"].GetString() : "NewScene";
			Editor::NewScene(name);
			OK(res);
		});
	})

	.post("/scene/open", [](auto* res, auto*) {
		ReadBody(res, [res](std::string body) {
			rapidjson::Document doc;
			doc.Parse(body.c_str());
			if(!doc.HasMember("name")) { BadRequest(res); return; }
			Editor::OpenScene(doc["name"].GetString());
			OK(res);
		});
	})

	.post("/scene/save", [](auto* res, auto*) {
		ReadBody(res, [res](std::string body) {
			rapidjson::Document doc;
			doc.Parse(body.c_str());
			Editor::SaveScene(doc.HasMember("name") ? doc["name"].GetString() : "");
			OK(res);
		});
	})

	.post("/scene/save-as", [](auto* res, auto*) {
		ReadBody(res, [res](std::string body) {
			rapidjson::Document doc;
			doc.Parse(body.c_str());
			if(!doc.HasMember("name")) { BadRequest(res); return; }
			Editor::SaveScene(doc["name"].GetString());
			OK(res);
		});
	})

	// ── Playback ──────────────────────────────────────────────────────────
	.post("/scene/play", [](auto* res, auto*) {
		ReadBody(res, [res](std::string body) {
			rapidjson::Document doc;
			doc.Parse(body.c_str());
			bool debug = doc.HasMember("debug") && doc["debug"].GetBool();
			Editor::OnPlay(debug);
			OK(res);
		});
	})

	.post("/scene/pause",  [](auto* res, auto*) { Editor::OnPause();  OK(res); })
	.post("/scene/resume", [](auto* res, auto*) { Editor::OnResume(); OK(res); })
	.post("/scene/stop",   [](auto* res, auto*) { Editor::OnStop();   OK(res); })

	// ── File system tree ──────────────────────────────────────────────────
	.get("/fs/tree", [](auto* res, auto*) {
		auto root = fs::path(Application::GetCurrentDir());
		JSON(res, SerializeFileTree(root));
	})

	// ── Assets ────────────────────────────────────────────────────────────
	.get("/assets", [](auto* res, auto*) {
		auto* mgr = AssetManager::Get()->As<EditorAssetManager>();
		std::string body = "[";
		bool first = true;
		mgr->GetRegistry()->For(
			[](Asset asset)
			{
				if(!asset.Primary)
					continue;
				if(!first)
					body += ",";
				body += SerializeAsset(mgr, asset);
				first = false;
			});
		JSON(res, body + "]");
	})

	.post("/assets/new", [](auto* res, auto*) {
		ReadBody(res, [res](std::string body) {
			rapidjson::Document doc;
			doc.Parse(body.c_str());
			if(!doc.HasMember("type")) { BadRequest(res); return; }

			std::string typeStr = doc["type"].GetString();
			AssetType type = AssetType::None;
			if(typeStr == "Material")    type = AssetType::Material;
			else if(typeStr == "Script") type = AssetType::Script;

			if(type == AssetType::None) { BadRequest(res); return; }

			auto* mgr = AssetManager::Get()->As<EditorAssetManager>();
			Asset asset = mgr->Add(type);
			if(doc.HasMember("name"))
				mgr->GetRegistry()->NameAsset(asset, doc["name"].GetString());

			JSON(res, SerializeAsset(mgr, asset));
		});
	})

	// ── Asset by id ───────────────────────────────────────────────────────
	// uWS wildcard patterns capture everything after the prefix,
	// so /assets/* matches /assets/42, /assets/42/rename, etc.
	// We parse the id and sub-action from the URL manually.

	.post("/assets/:id/rename", [](auto* res, auto* req) {
		std::string url(req->getUrl());
		ReadBody(res, [res, url](std::string body) {
			rapidjson::Document doc;
			doc.Parse(body.c_str());
			if(!doc.HasMember("name")) { BadRequest(res); return; }

			uint64_t id;
			if(!ParseId(url, id)) { BadRequest(res); return; }

			auto* mgr = AssetManager::Get()->As<EditorAssetManager>();
			Asset asset = { id, AssetType::None };
			mgr->GetRegistry()->RemoveName(asset);
			mgr->GetRegistry()->NameAsset(asset, doc["name"].GetString());
			OK(res);
		});
	})

	.del("/assets/:id", [](auto* res, auto* req) {
		std::string url(req->getUrl());
		uint64_t id;
		if(!ParseId(url, id)) { BadRequest(res); return; }
		auto* mgr = AssetManager::Get()->As<EditorAssetManager>();
		mgr->Remove({ id, AssetType::None });
		OK(res);
	})

	// ── Material ──────────────────────────────────────────────────────────
	.get("/assets/:id/material", [](auto* res, auto* req) {
		std::string url(req->getUrl());
		uint64_t id;
		if(!ParseId(url, id)) { BadRequest(res); return; }

		auto* mgr = AssetManager::Get()->As<EditorAssetManager>();
		Asset asset = { id, AssetType::Material };
		mgr->Load(asset);
		auto mat = mgr->Get<Material>(asset);
		if(!mat) { NotFound(res); return; }
		JSON(res, SerializeMaterial(*mat));
	})

	.post("/assets/:id/material", [](auto* res, auto* req) {
		std::string url(req->getUrl());
		ReadBody(res, [res, url](std::string body) {
			uint64_t id;
			if(!ParseId(url, id)) { BadRequest(res); return; }

			auto* mgr = AssetManager::Get()->As<EditorAssetManager>();
			Asset asset = { id, AssetType::Material };
			mgr->Load(asset);
			auto mat = mgr->Get<Material>(asset);
			if(!mat) { NotFound(res); return; }

			rapidjson::Document doc;
			doc.Parse(body.c_str());

			if(doc.HasMember("intUniforms")) {
				mat->IntUniforms.clear();
				for(auto& e : doc["intUniforms"].GetArray())
					mat->IntUniforms[e["name"].GetString()] = e["value"].GetInt();
			}
			if(doc.HasMember("floatUniforms")) {
				mat->FloatUniforms.clear();
				for(auto& e : doc["floatUniforms"].GetArray())
					mat->FloatUniforms[e["name"].GetString()] = e["value"].GetFloat();
			}
			if(doc.HasMember("vec2Uniforms")) {
				mat->Vec2Uniforms.clear();
				for(auto& e : doc["vec2Uniforms"].GetArray()) {
					auto a = e["value"].GetArray();
					mat->Vec2Uniforms[e["name"].GetString()] = Vec2(a[0].GetFloat(), a[1].GetFloat());
				}
			}
			if(doc.HasMember("vec3Uniforms")) {
				mat->Vec3Uniforms.clear();
				for(auto& e : doc["vec3Uniforms"].GetArray()) {
					auto a = e["value"].GetArray();
					mat->Vec3Uniforms[e["name"].GetString()] = Vec3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());
				}
			}
			if(doc.HasMember("vec4Uniforms")) {
				mat->Vec4Uniforms.clear();
				for(auto& e : doc["vec4Uniforms"].GetArray()) {
					auto a = e["value"].GetArray();
					mat->Vec4Uniforms[e["name"].GetString()] = Vec4(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat(), a[3].GetFloat());
				}
			}
			if(doc.HasMember("textureUniforms")) {
				mat->TextureUniforms.clear();
				for(auto& e : doc["textureUniforms"].GetArray())
					mat->TextureUniforms[e["name"].GetString()] = e["value"].GetUint64();
			}

			OK(res);
		});
	})

	.listen(9002, [](auto* listenSocket) {
		if(listenSocket)
			std::cout << "Listening on port 9002" << std::endl;
	})
	.run();
}

}
