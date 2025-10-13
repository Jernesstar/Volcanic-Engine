#pragma once

#include <drogon/drogon.h>

#include <VolcaniCore/Core/Defines.h>

using namespace VolcaniCore;

namespace Magma::Networking {

class HttpClient {
public:
	HttpClient(const std::string& baseURL) {
		m_Client = drogon::HttpClient::newHttpClient(baseURL);
	}

	HttpClient(const std::string& ip, uint16_t port) {
		m_Client = drogon::HttpClient::newHttpClient(ip, port);
	}

	void Get(const std::string& path,
			 const Func<void, const drogon::HttpResponsePtr&> callback);
	void Post(const std::string& path, const std::string& body,
			 const Func<void, const drogon::HttpResponsePtr&> callback);
	void Put(const std::string& path,
			 const Func<void, const drogon::HttpResponsePtr&> callback);
	void Delete(const std::string& path,
			 const Func<void, const drogon::HttpResponsePtr&> callback);

private:
	Ref<drogon::HttpClient> m_Client;
}

}