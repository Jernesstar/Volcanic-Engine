#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <cpp-httplib/httplib.h>

#include <VolcaniCore/Core/Buffer.h>
#include <VolcaniCore/Core/Defines.h>

using namespace VolcaniCore;

namespace Magma::Networking {

class TokenStore {
public:
	static void SaveToken(const std::string& service, const std::string& token);
	static std::string LoadToken(const std::string& service);
};

class NetworkingManager {
public:
	static void Init();
	static void Shutdown();
	static void GitHubOAuth();
};

using Bytes = Buffer<uint8_t>;

enum class PayloadType {
	JSON,
	XML,
	Text,
	Binary
};

struct Request {
	std::string Path;
	PayloadType Type;
	std::string Body;
	httplib::Headers Headers;

	void AddHeader(const std::string& key, const std::string& value) {
		Headers.insert({ key, value });
	}

	bool HasHeader(const std::string& key) const {
		return Headers.find(key) != Headers.end();
	}

	std::string GetHeader(const std::string& key) const {
		auto it = Headers.find(key);
		if(it != Headers.end())
			return it->second;
		return "";
	}
};

enum class ResponseResult {
	Success,
	Error
};

struct Response {
	ResponseResult Result;
	u16 StatusCode;
	std::string Body;
};

using ResponseCB = Func<void, const Response&>;

class HttpClient {
public:
	HttpClient(const std::string& baseURL);
	HttpClient(const std::string& ip, u16 port);

	Response Get(const Request& req);
	void Get(const Request& req, const ResponseCB& cb) {
		cb(Get(req));
	}

	Response Post(const Request& req);
	void Post(const Request& req, const ResponseCB& cb) {
		cb(Post(req));
	}

	Response Put(const Request& req);
	void Put(const Request& req, const ResponseCB& cb) {
		cb(Put(req));
	}

	Response Delete(const Request& req);
	void Delete(const Request& req, const ResponseCB& cb) {
		cb(Delete(req));
	}

	std::string GetURL() const;

private:
	httplib::Client m_Client;
};

class HTTPServer {
public:
	HTTPServer(const std::string& ip, u16 port);
	~HTTPServer();

private:
	httplib::Server m_Server;
};

class User {
public:
	User() = default;
	~User() = default;

	void Login(const std::string& username, const std::string& password);

private:
	bool m_LoggedIn = false;
};

}