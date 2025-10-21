#include "Networking.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

using namespace drogon;
using namespace VolcaniCore;

#define VOLCANIC_AUTH_URL "http://localhost:8848"

namespace Magma::Networking {

Ref<std::thread> s_HttpThread;

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
	s_HttpThread = CreateRef<std::thread>(
		[]()
		{
			app().run();
		});

	s_HttpThread->detach();
}

void NetworkingManager::Shutdown() {
	// s_HttpThread.reset();
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
		auto client = drogon::HttpClient::newHttpClient("127.0.0.1", 8848);
		auto req = HttpRequest::newHttpRequest();
		req->setPath("/auth/github/token");
		req->setMethod(drogon::Get);

		client->sendRequest(req,
			[&polling](ReqResult result, const HttpResponsePtr& resp)
			{
				if(result != ReqResult::Ok) {
					VOLCANICORE_LOG_INFO("Error: %s", to_string(result).c_str());
					return;
				}

				auto responseStr = std::string(resp->getBody().data());

				Json::CharReaderBuilder builder;
				Json::CharReader* reader = builder.newCharReader();
				Json::Value response;
				reader->parse(responseStr.c_str(),
					responseStr.c_str() + responseStr.size(), &response, nullptr);
	
				if(response["status"].asString() == "success") {
					std::string token = response["token"].asString();
					TokenStore::SaveToken("github", token);
					polling = false;
					return;
				}
				if(response["status"].asString() == "error") {
					polling = false;
					return;
				}
			});

		std::this_thread::sleep_for(std::chrono::seconds(4));
	}
}

void HttpClient::Get(const std::string& path,
					 const Func<void, const drogon::HttpResponsePtr&> cb)
{
	auto req = HttpRequest::newHttpRequest();
	req->setPath(path);
	req->setMethod(HttpMethod::Get);
	req->setContentTypeCode(CT_TEXT_PLAIN);
	m_Client->sendRequest(req,
		[&](ReqResult res, const HttpResponsePtr& response)
		{
			if(res == ReqResult::Ok)
				cb(response);
			else
				cb(nullptr);
		});
}

void HttpClient::Post(const std::string& path, Bytes bytes, PayloadType type,
					  const Func<void, const drogon::HttpResponsePtr&> cb)
{
	
}

void HttpClient::Put(const std::string& path, Bytes bytes, PayloadType type,
					 const Func<void, const drogon::HttpResponsePtr&> cb)
{
}

void HttpClient::Delete(const std::string& path,
					const Func<void, const drogon::HttpResponsePtr&> cb)
{
}

}