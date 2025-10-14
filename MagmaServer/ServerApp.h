#pragma once

#include <drogon/drogon.h>
#include <jwt-cpp/jwt.h>
#include <openssl/sha.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/List.h>

using namespace drogon;

using namespace VolcaniCore;

namespace Magma::Server {

struct User {
	std::string Name;
	std::string Username;
	std::string Email;
	std::string Password;
	std::string PasswordHash;
};

class ServerApp : public Application {
public:
	ServerApp();
	~ServerApp();

	void StartServer();

	void OnUpdate(TimeStep ts) override { };

private:
	std::thread m_HttpThread;
	List<User> m_Users;
};

ServerApp::ServerApp() {
	VOLCANICORE_LOG_INFO("Starting HTTP thread");

	m_HttpThread = std::thread(
		[this]()
		{
			StartServer();
		});

	m_HttpThread.detach();
}

ServerApp::~ServerApp() {
	VOLCANICORE_LOG_INFO("Stopping HTTP thread");
}
std::string sha256_hex(const std::string &input) {
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256((const unsigned char*)input.data(), input.size(), hash);
	std::ostringstream oss;
	oss << std::hex << std::setfill('0');
	for (auto b : hash) oss << std::setw(2) << (int)b;
	return oss.str();
}

// --- JWT creation using HS256 (jwt-cpp) ---
std::string create_jwt(const std::string &sub) {
	const std::string secret = std::getenv("VOLCANIC_JWT_SECRET") ? std::getenv("VOLCANIC_JWT_SECRET") : "demo-secret";
	using namespace std::chrono;
	auto now = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	auto exp = now + 3600; // 1 hour

	auto token =
		jwt::create()
		.set_issuer("volcanic-demo")
		.set_type("JWS")
		.set_subject(sub)
		.set_issued_at(std::chrono::system_clock::now())
		.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{3600})
		.sign(jwt::algorithm::hs256{secret});

	return token;
}

void ServerApp::StartServer() {
	// app().registerHandler("/v1/rpc",
	// 	[](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
	// 	{

	// 	}, { HttpMethod::Post });

	app().registerHandler("/v1/auth/register",
		[this](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
		{
			Json::CharReaderBuilder builder;
			Json::CharReader* reader = builder.newCharReader();
			Json::Value request;
			reader->parse(req->bodyData(),
				req->bodyData() + req->bodyLength(), &request, nullptr);

			auto& user = m_Users.Emplace();
			user.Name = request["Name"].asString();
			user.Username = request["Username"].asString();
			user.Email = request["Email"].asString();
			user.Password = request["Password"].asString();
			user.PasswordHash = sha256_hex(user.Password + "volcanic_salt");

			Json::Value response;
			response["status"] = "Ok";
			callback(HttpResponse::newHttpJsonResponse(response));
		}, { HttpMethod::Post });

	app().registerHandler("/v1/auth/login",
		[this](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
		{
			Json::CharReaderBuilder builder;
			Json::CharReader* reader = builder.newCharReader();
			Json::Value request;
			reader->parse(req->bodyData(),
				req->bodyData() + req->bodyLength(), &request, nullptr);

			std::string email = request["Email"].asString();
			std::string password = request["Password"].asString();
			if (email.empty() || password.empty()) {
				callback(HttpResponse::newHttpResponse());
				return;
			}

			auto [found, i] =
				m_Users.Find([&](auto& u) { return u.Email == email; });
			if(!found) {
				Json::Value response;
				response["error"] = "not_found";
				auto r = HttpResponse::newHttpJsonResponse(response);
				r->setStatusCode(k404NotFound);
				callback(r);
				return;
			}

			auto& u = m_Users[i];
			if(u.PasswordHash != sha256_hex(password + "volcanic_salt")) {
				Json::Value response;
				response["error"] = "invalid";
				auto r = HttpResponse::newHttpJsonResponse(response);
				r->setStatusCode(k401Unauthorized);
				callback(r);
				return;
			}

			auto token = create_jwt(u.Name);
			Json::Value response;
			response["access_token"] = token;
			callback(HttpResponse::newHttpJsonResponse(response));
		}, { HttpMethod::Post });

	app().registerHandler("/v1/oauth/google",
		[this](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
		{

		}, { HttpMethod::Get });

	app().registerHandler("/v1/oauth/callback",
		[this](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
		{

		}, { HttpMethod::Get });

	app()
		.addListener("127.0.0.1", 8848)
		.run();
}

}