#pragma once

#include <drogon/drogon.h>

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

class HttpClient {
public:
	HttpClient(const std::string& baseURL) {
		m_Client = drogon::HttpClient::newHttpClient(baseURL);
	}

	HttpClient(const std::string& ip, uint16_t port) {
		m_Client = drogon::HttpClient::newHttpClient(ip, port);
	}

	void Get(const std::string& path,
			 const Func<void, const drogon::HttpResponsePtr&> cb);
	void Post(const std::string& path, Bytes bytes, PayloadType type,
			 const Func<void, const drogon::HttpResponsePtr&> cb);
	void Put(const std::string& path, Bytes bytes, PayloadType type,
			 const Func<void, const drogon::HttpResponsePtr&> cb);
	void Delete(const std::string& path,
			 const Func<void, const drogon::HttpResponsePtr&> cb);

private:
	Ref<drogon::HttpClient> m_Client;
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