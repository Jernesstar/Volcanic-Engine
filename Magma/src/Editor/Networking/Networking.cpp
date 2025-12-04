#include "Networking.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

using namespace VolcaniCore;

#define VOLCANIC_AUTH_URL "http://localhost:8848"

namespace Magma::Networking {

inline std::string GetType(PayloadType type) {
	switch(type) {
	case PayloadType::JSON:
		return "application/json";
	case PayloadType::XML:
		return "application/xml";
	case PayloadType::Text:
		return "text/plain";
	case PayloadType::Binary:
		return "application/octet-stream";
	default:
		return "";
	}
}

void TokenStore::SaveToken(const std::string& service, const std::string& token) {
	Application::PushDir();

	fs::path path = fs::path("Editor") / ".cache" / "tokens";
	fs::create_directories(path);
	fs::create_directories(path / service);
	auto file = File(path / service / ".token", true);
	file.Write(token);
	file.Close();

	Application::PopDir();
}

std::string TokenStore::LoadToken(const std::string& service) {
	Application::PushDir();

	auto path = fs::path("Editor") / ".cache" / "tokens" / service / ".token";
	File file(path, false);
	std::string token = file.Get();

	Application::PopDir();
	return token;
}

void NetworkingManager::Init() {

}

void NetworkingManager::Shutdown() {

}

void NetworkingManager::GitHubOAuth() {
	std::string authURL = VOLCANIC_AUTH_URL"/auth/github/login";

#ifdef _WIN32
	system(("start " + authURL).c_str());
#elif __APPLE__
	system(("open " + authURL).c_str());
#elif __linux__
	system(("xdg-open " + authURL).c_str());
#endif

	std::this_thread::sleep_for(std::chrono::seconds(4));
	bool polling = true;

	while(polling) {
		auto client = HttpClient("127.0.0.1", 8848);
		client.Get({ "/auth/github/token" },
			[&polling](const Response& resp)
			{
				if(resp.Result != ResponseResult::Success) {
					VOLCANICORE_LOG_INFO("Error: %i", resp.StatusCode);
					return;
				}

				rapidjson::Document doc;
				doc.Parse(resp.Body.c_str(), resp.Body.length());

				if(doc["status"].GetString() == "success") {
					std::string token = doc["token"].GetString();
					TokenStore::SaveToken("github", token);
					polling = false;
					return;
				}
				if(doc["status"].GetString() == "error") {
					polling = false;
					return;
				}
			});

		std::this_thread::sleep_for(std::chrono::seconds(4));
	}
}

HttpClient::HttpClient(const std::string& baseURL)
	: m_Client(baseURL) { }

HttpClient::HttpClient(const std::string& ip, uint16_t port)
	: m_Client(ip, port) { }

Response HttpClient::Get(const Request& req) {
	auto res = m_Client.Get(req.Path, req.Headers);
	if(res)
		return { ResponseResult::Success, (u16)res->status, res->body };
	return { ResponseResult::Error, 0, "" };
}

Response HttpClient::Post(const Request& req) {
	auto res = m_Client.Post(req.Path, req.Headers, req.Body, GetType(req.Type));
	if(res)
		return { ResponseResult::Success, (u16)res->status, res->body };
	return { ResponseResult::Error, 0, "" };
}

Response HttpClient::Put(const Request& req) {

	return { };
}

Response HttpClient::Delete(const Request& req) {

	return { };
}

}