#pragma once

#include <drogon/drogon.h>

#include <VolcaniCore/Core/Defines.h>

using namespace drogon;

namespace Magma::Server {

class OAuthController : public drogon::HttpController<OAuthController> {
public:
	METHOD_LIST_BEGIN
	ADD_METHOD_TO(OAuthController::Login, "/auth/github/login", drogon::Get);
	ADD_METHOD_TO(OAuthController::Callback, "/auth/github/callback", drogon::Get);
	ADD_METHOD_TO(OAuthController::Token, "/auth/github/token", drogon::Get);
	METHOD_LIST_END

	void Login(const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback);
	void Callback(const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback);
	void Token(const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback);
};

}