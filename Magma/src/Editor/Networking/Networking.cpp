#include "Networking.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

using namespace VolcaniCore;

#define VOLCANIC_AUTH_URL "http://localhost:8848"

namespace Magma::Networking {

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
		client.Get("/auth/github/token",
			[&polling](ResponseResult res, const Response& resp)
			{
				if(res != ResponseResult::Success) {
					VOLCANICORE_LOG_INFO("Error: %i", res);
					return;
				}

				rapidjson::Document doc;
				doc.Parse((char*)resp.Data.Get(), resp.Data.GetSize());

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

void HttpClient::Get(const std::string& path, const ResponseCB& cb) {
	m_Client.Get(path);
}

void HttpClient::Post(const std::string& path, Bytes bytes, PayloadType type,
					  const ResponseCB& cb)
{
	
}

void HttpClient::Put(const std::string& path, Bytes bytes, PayloadType type,
					 const ResponseCB& cb)
{
}

void HttpClient::Delete(const std::string& path, const ResponseCB& cb) {

}

}