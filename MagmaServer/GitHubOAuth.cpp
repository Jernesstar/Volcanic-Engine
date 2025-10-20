#include "GitHubOAuth.h"

#include <VolcaniCore/Core/Log.h>

namespace Magma::Server {

static bool parseJson(const std::string &s, Json::Value &out, std::string &errMsg) {
	Json::CharReaderBuilder b;
	std::unique_ptr<Json::CharReader> reader(b.newCharReader());
	const char* start = s.c_str();
	const char* end = start + s.size();
	return reader->parse(start, end, &out, &errMsg);
}

void OAuthController::Login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
	// Build redirect URL to GitHub's authorization endpoint
	const std::string clientId = std::getenv("GITHUB_CLIENT_ID");
	const std::string redirectUri = std::getenv("GITHUB_REDIRECT_URI");
	const std::string scope = "repo%20user"; // URL-encoded: repo user

	std::string authorizeUrl =
		"https://github.com/login/oauth/authorize?client_id=" + clientId +
		"&redirect_uri=" + drogon::utils::urlEncode(redirectUri) +
		"&scope=" + scope +
		"&allow_signup=true";

	auto resp = HttpResponse::newRedirectionResponse(authorizeUrl);
	callback(resp);
}

void OAuthController::Callback(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
	auto codeOpt = req->getOptionalParameter<std::string>("code");
	// auto stateOpt = req->getOptionalParameter<std::string>("state");
	if(!codeOpt) {
		auto r = HttpResponse::newHttpResponse();
		r->setStatusCode(k400BadRequest);
		r->setBody("Missing code");
		callback(r);
		return;
	}
	// if(!stateOpt) {
	// 	auto r = HttpResponse::newHttpResponse();
	// 	r->setStatusCode(k400BadRequest);
	// 	r->setBody("Missing state");
	// 	callback(r);
	// 	return;
	// }
	// Optional: validate state against cookie/session saved at /login

	const std::string code = *codeOpt;
	const std::string clientId = std::getenv("GITHUB_CLIENT_ID");
	const std::string clientSecret = std::getenv("GITHUB_CLIENT_SECRET");
	const std::string redirectUri = std::getenv("GITHUB_REDIRECT_URI");

	// Prepare JSON body for token exchange
	Json::Value body;
	body["client_id"] = clientId;
	body["client_secret"] = clientSecret;
	body["code"] = code;
	body["redirect_uri"] = redirectUri;

	Json::StreamWriterBuilder wbuilder;
	std::string bodyStr = Json::writeString(wbuilder, body);

	// Send POST to GitHub token endpoint
	auto client = HttpClient::newHttpClient("https://github.com");
	auto req2 = HttpRequest::newHttpRequest();
	req2->setPath("/login/oauth/access_token");
	req2->setMethod(drogon::Post);
	req2->setBody(bodyStr);
	req2->addHeader("Accept", "application/json");
	req2->addHeader("Content-Type", "application/json");

	client->sendRequest(req2,
		[callback](ReqResult result, const HttpResponsePtr &resp)
		{
			if(result != ReqResult::Ok || !resp) {
				auto r = HttpResponse::newHttpResponse();
				r->setStatusCode(k500InternalServerError);
				r->setBody("Failed to contact GitHub token endpoint");
				callback(r);
				return;
			}

			std::string respBody = resp->getBody().data();

			Json::Value jsonResp;
			std::string err;
			if(!parseJson(respBody, jsonResp, err)) {
				auto r = HttpResponse::newHttpResponse();
				r->setStatusCode(k500InternalServerError);
				r->setBody(std::string("Failed to parse token response: ") + err);
				callback(r);
				return;
			}

			if(!jsonResp.isMember("access_token")) {
				auto r = HttpResponse::newHttpResponse();
				r->setStatusCode(k500InternalServerError);
				r->setBody("No access_token in response");
				callback(r);
				return;
			}

			std::string token = jsonResp["access_token"].asString();

			auto r = HttpResponse::newHttpResponse();
			std::string redirectUrl =
				"http://0.0.0.0:5050/token?access_token=" + token;
			auto redirect = HttpResponse::newRedirectionResponse(redirectUrl);
			callback(redirect);
			return;
		});
}

}