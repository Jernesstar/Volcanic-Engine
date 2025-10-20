#pragma once
#include <drogon/drogon.h>

using namespace drogon;

namespace Magma::Server {

class OAuthController : public drogon::HttpController<OAuthController> {
public:
	METHOD_LIST_BEGIN
	ADD_METHOD_TO(OAuthController::Login, "/auth/github/login", drogon::Get);
	ADD_METHOD_TO(OAuthController::Callback, "/auth/github/callback", drogon::Get);
	METHOD_LIST_END

	void Login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
	void Callback(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
};

}