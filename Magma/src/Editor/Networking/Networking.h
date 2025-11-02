#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <VolcaniCore/Core/Buffer.h>
#include <VolcaniCore/Core/Defines.h>

using namespace VolcaniCore;

namespace Magma::Networking {

using Bytes = Buffer<uint8_t>;

class TokenStore {
public:
	static void SaveToken(const std::string& service, const std::string& token);
	static std::string LoadToken(const std::string& service);
};

class NetworkingManager {
public:
	NetworkingManager() = default;
	~NetworkingManager() = default;

	static void Init();
	static void Shutdown();

	void GitHubOAuth();
};

enum class PayloadType {
	Json,
	Bytes,
	Text
};

enum class ResponseResult;

struct Response {
	Bytes Data;
};

using ResponseCB = Func<void, ResponseResult, Response>;

class HttpClient {
public:
	HttpClient(const std::string& baseURL);
	HttpClient(const std::string& ip, uint16_t port);

	void Get(const std::string& path, const ResponseCB& cb);
	void Post(const std::string& path, Bytes bytes, PayloadType type, const ResponseCB& cb);
	void Put(const std::string& path, Bytes bytes, PayloadType type, const ResponseCB& cb);
	void Delete(const std::string& path, const ResponseCB& cb);

private:

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