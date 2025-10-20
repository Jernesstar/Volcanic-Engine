#include "UserService.h"

#include <VolcaniCore/Core/Defines.h>

using namespace drogon;

namespace Magma::Server {

static std::string sha256_hex(const std::string &input) {
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256((const unsigned char*)input.data(), input.size(), hash);
	std::ostringstream oss;
	oss << std::hex << std::setfill('0');
	for (auto b : hash) oss << std::setw(2) << (int)b;
	return oss.str();
}

static std::string create_jwt(const std::string &sub) {
	auto sec = std::getenv("VOLCANIC_JWT_SECRET");
	const std::string secret = sec ? sec : "demo-secret";
	using namespace std::chrono;
	auto now =
		duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	auto exp = now + 3600; // 1 hour

	auto token =
		jwt::create()
		.set_issuer("Volcanic-Dev")
		.set_type("JWS")
		.set_subject(sub)
		.set_issued_at(std::chrono::system_clock::now())
		.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{3600})
		.sign(jwt::algorithm::hs256{ secret });

	return token;
}

void UserService::Init() {
	app().registerHandler("/v1/auth/register",
		[](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
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
		[](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
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
		[](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
		{

		}, { HttpMethod::Get });

	app().registerHandler("/v1/oauth/callback",
		[](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
		{

		}, { HttpMethod::Get });
	}
}