#include "GitHubOAuth.h"

#include <VolcaniCore/Core/Log.h>

namespace Magma::Server {

const char* GITHUB_CLIENT_ID = "Ov23liQd1sHpMWBUhEuA";
const char* GITHUB_CLIENT_SECRET = "bc976994b87be40408b0610e4df6054b969dab35";
const char* GITHUB_REDIRECT_URI = "http://localhost:8848/auth/github/callback";

static std::string TOKEN = "NULL";

static bool ParseJson(const std::string &s, Json::Value &out, std::string &errMsg) {
	Json::CharReaderBuilder b;
	std::unique_ptr<Json::CharReader> reader(b.newCharReader());
	const char* start = s.c_str();
	const char* end = start + s.size();
	return reader->parse(start, end, &out, &errMsg);
}

void OAuthController::Login(const HttpRequestPtr& req,
							Func<void, const HttpResponsePtr&>&& callback)
{
	TOKEN = "PENDING";
	const std::string clientId = GITHUB_CLIENT_ID;
	const std::string redirectURI = GITHUB_REDIRECT_URI;
	const std::string scope = "repo%20user"; // URL-encoded: repo user

	std::string authorizeUrl =
		"https://github.com/login/oauth/authorize?client_id=" + clientId +
		"&redirect_uri=" + drogon::utils::urlEncode(redirectURI) +
		"&scope=" + scope +
		"&allow_signup=true";

	auto resp = HttpResponse::newRedirectionResponse(authorizeUrl);
	callback(resp);
}

void OAuthController::Callback(const HttpRequestPtr& req,
							   Func<void, const HttpResponsePtr&>&& callback)
{
	auto codeOpt = req->getOptionalParameter<std::string>("code");
	// auto stateOpt = req->getOptionalParameter<std::string>("state");
	if(!codeOpt) {
		auto r = HttpResponse::newHttpResponse();
		r->setStatusCode(k400BadRequest);
		r->setContentTypeCode(CT_TEXT_HTML);
		r->setBody(
		"<html> \
			<body> \
				<h3>Authentication Failed!</h3> \
				<p>You may close this window</p> \
			</body> \
		</html>");
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
	const std::string clientId = GITHUB_CLIENT_ID;
	const std::string clientSecret = GITHUB_CLIENT_SECRET;
	const std::string redirectURI = GITHUB_REDIRECT_URI;

	// Prepare JSON body for token exchange
	Json::Value body;
	body["client_id"] = clientId;
	body["client_secret"] = clientSecret;
	body["code"] = code;
	body["redirect_uri"] = redirectURI;

	Json::StreamWriterBuilder wbuilder;
	std::string bodyStr = Json::writeString(wbuilder, body);

	// Send POST to GitHub token endpoint
	auto client = HttpClient::newHttpClient("https://github.com");
	auto req2 = HttpRequest::newHttpRequest();
	req2->setPath("/login/oauth/access_token");
	req2->setMethod(drogon::Post);
	req2->setBody(bodyStr);
	req2->setContentTypeCode(CT_APPLICATION_JSON);
	req2->addHeader("Accept", "application/json");

	client->sendRequest(req2,
		[callback](ReqResult result, const HttpResponsePtr &resp)
		{
			if(result != ReqResult::Ok) {
				auto r = HttpResponse::newHttpResponse();
				r->setStatusCode(k500InternalServerError);
				r->setBody(to_string(result));
				callback(r);
				return;
			}
			std::string respBody = resp->getBody().data();

			Json::Value jsonResp;
			std::string err;
			if(!ParseJson(respBody, jsonResp, err)) {
				auto r = HttpResponse::newHttpResponse();
				r->setStatusCode(k500InternalServerError);
				r->setBody(std::string("Failed to parse token response: ") + err);
				callback(r);
				return;
			}

			if(!jsonResp.isMember("access_token")) {
				auto r = HttpResponse::newHttpResponse();
				r->setStatusCode(k500InternalServerError);
				r->setBody(respBody);
				callback(r);
				return;
			}

			TOKEN = jsonResp["access_token"].asString();

			auto r = HttpResponse::newHttpResponse();
			r->setStatusCode(k200OK);
			r->setContentTypeCode(CT_TEXT_HTML);
			r->setBody(
			"<html> \
				<body> \
					<h3>Authentication Successful!</h3> \
					<p>You may close this window</p> \
				</body> \
			</html>");
			callback(r);
			return;
		});
}

void OAuthController::Token(const HttpRequestPtr& req,
							Func<void, const HttpResponsePtr&>&& callback)
{
	VOLCANICORE_LOG_INFO("Token: %s", TOKEN.c_str());

	if(TOKEN == "NULL") {
		auto r = HttpResponse::newHttpResponse();
		r->setStatusCode(k500InternalServerError);
		r->setBody("{ \"status\": \"error\" }");
		callback(r);
		return;
	}
	if(TOKEN == "PENDING") {
		auto r = HttpResponse::newHttpResponse();
		r->setStatusCode(k200OK);
		r->setBody("{ \"status\": \"pending\" }");
		callback(r);
		return;
	}

	auto r = HttpResponse::newHttpResponse();
	r->setStatusCode(k200OK);
	r->setBody("{ \"status\": \"success\", \"token\": \"" + TOKEN + "\" }");
	TOKEN = "NULL";
	callback(r);
}

}