#include "Networking.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

using namespace drogon;

namespace Magma::Networking {

Ref<std::thread> s_HttpThread;

void TokenStore::SaveToken(const std::string& service, const std::string& token) {
	Application::PushDir();

	fs::path path = fs::path("Editor") / "tokens" / service;
	auto file = File(path.string() + ".token", true);
	file.Write(token);

	Application::PopDir();
}

std::string TokenStore::LoadToken(const std::string& service) {
	Application::PushDir();

	File file((fs::path("Editor") / "tokens" / service).string() + ".token");
	std::string token = file.Get();

	Application::PopDir();
	return token;
}

void NetworkingManager::Init() {
	// s_HttpThread = CreateRef<std::thread>(
	// 	[]()
	// 	{
	// 		app().addListener("0.0.0.0", 5050);
	// 		app().run();
	// 	});

	// s_HttpThread->detach();
}

void NetworkingManager::Shutdown() {
	// s_HttpThread.reset();
}

void NetworkingManager::GitHubOAuth() {
	// Start a local server to capture the token
	std::thread listener(
		[this]()
		{
			app().registerHandler("/token",
				[this](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback)
				{
					auto token = req->getParameter("access_token");
					if (!token.empty()) {
						TokenStore::SaveToken("github", token);
						std::cout << "✅ GitHub token received: " << token << std::endl;

						auto resp = drogon::HttpResponse::newHttpResponse();
						resp->setBody("<html><body><h3>Authentication Successful!</h3><p>You can close this window.</p></body></html>");
						resp->setStatusCode(drogon::k200OK);
						callback(resp);

						// Stop the listener once the token is received
						app().quit();
					} else {
						auto resp = drogon::HttpResponse::newHttpResponse();
						resp->setBody("Missing access token");
						resp->setStatusCode(drogon::k400BadRequest);
						callback(resp);
					}
				},
				{ HttpMethod::Get });

			drogon::app().addListener("0.0.0.0", 5050).run();
		}
	);

	listener.detach();

	// Direct user to GitHub login
	std::string authUrl = "http://localhost:8848/auth/github/login";

	// On real editor, open browser automatically:
	system(("start " + authUrl).c_str());  // Windows
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