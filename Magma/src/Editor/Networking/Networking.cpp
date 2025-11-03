#include "Networking.h"

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/Document.h>

using namespace VolcaniCore;

#define VOLCANIC_AUTH_URL "http://localhost:8848"

namespace Magma::Networking {

Ref<std::thread> s_HttpThread;

void TokenStore::SaveToken(const std::string& service, const std::string& token) {
	Application::PushDir();

	fs::path path = fs::path("Editor") / ".cache" / "tokens";
	fs::create_directories(path);
	fs::create_directories(path / service);
	auto file = File(path / service / ".token", true);
	file.Write(token);
	file.Close();

	Application::PopDir();
}

std::string TokenStore::LoadToken(const std::string& service) {
	Application::PushDir();

	auto path = fs::path("Editor") / ".cache" / "tokens" / service / ".token";
	File file(path, false);
	std::string token = file.Get();

	Application::PopDir();
	return token;
}

void NetworkingManager::Init() {
	s_HttpThread = CreateRef<std::thread>(
		[]()
		{
			// app().run();
		});

	s_HttpThread->detach();
}

void NetworkingManager::Shutdown() {
	// s_HttpThread.reset();
}

void NetworkingManager::GitHubOAuth() {
	std::string authURL = VOLCANIC_AUTH_URL"/auth/github/login";

#ifdef _WIN32
	system(("start " + authURL).c_str());
#elif __APPLE__
	system(("open " + authURL).c_str());
#elif __linux__
	system(("xdg-open " + authURL).c_str());
#endif

	std::this_thread::sleep_for(std::chrono::seconds(4));
	bool polling = true;

	while(polling) {
		auto client = HttpClient("127.0.0.1", 8848);
		client.Get("/auth/github/token",
			[&polling](ResponseResult res, const Response& resp)
			{
				if(res != ResponseResult::Success) {
					VOLCANICORE_LOG_INFO("Error: %i", res);
					return;
				}

				rapidjson::Document doc;
				doc.Parse((char*)resp.Data.Get(), resp.Data.GetSize());

				if(doc["status"].GetString() == "success") {
					std::string token = doc["token"].GetString();
					TokenStore::SaveToken("github", token);
					polling = false;
					return;
				}
				if(doc["status"].GetString() == "error") {
					polling = false;
					return;
				}
			});

		std::this_thread::sleep_for(std::chrono::seconds(4));
	}
}

HttpClient::HttpClient(const std::string& baseURL)
	: m_BaseURL(baseURL) { }

HttpClient::HttpClient(const std::string& ip, uint16_t port)
	: m_IP(ip), m_Port(port) { }

void HttpClient::Get(const std::string& path, const ResponseCB& cb) {
	asio::io_context io_context;
	asio::ssl::context ctx(asio::ssl::context::tlsv12_client);
	asio::ssl::stream<asio::ip::tcp::socket> socket(io_context, ctx);

	// Resolve host
	asio::ip::tcp::resolver resolver(io_context);
	asio::ip::basic_resolver_results<asio::ip::tcp> endpoints;
	if(!m_BaseURL.empty())
		endpoints = resolver.resolve(m_BaseURL, "http");
	else
		endpoints = resolver.resolve(m_IP, std::to_string(m_Port));

	// Connect and handshake
	asio::connect(socket.lowest_layer(), endpoints);
	socket.handshake(asio::ssl::stream_base::client);

	// Build HTTP request
	std::ostringstream request;
	// request << "POST " << target << " HTTP/1.1\r\n"
	// 		<< "Host: " << host << "\r\n"
	// 		<< "Authorization: Bearer " << api_key << "\r\n"
	// 		<< "Content-Type: application/json\r\n"
	// 		<< "Content-Length: " << data.size() << "\r\n"
	// 		<< "Connection: close\r\n\r\n"
	// 		<< data;

	// Send request
	asio::write(socket, asio::buffer(request.str()));

	// Read response
	asio::streambuf response;
	asio::read_until(socket, response, "\r\n");

	// Check response
	std::istream response_stream(&response);
	std::string http_version;
	unsigned int status_code;
	std::string status_message;

	response_stream >> http_version >> status_code;
	std::getline(response_stream, status_message);

	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		throw std::runtime_error("Invalid HTTP response");
	if (status_code != 200)
		throw std::runtime_error("Request failed with status " + std::to_string(status_code));

	// Read headers
	asio::read_until(socket, response, "\r\n\r\n");
	std::string header;
	while (std::getline(response_stream, header) && header != "\r")
		;

	// Read body
	std::ostringstream body;
	if (response.size() > 0)
		body << &response;
	asio::error_code ec;
	while (asio::read(socket, response, asio::transfer_at_least(1), ec))
		body << &response;
	if (ec != asio::error::eof) throw std::runtime_error("Read failed: " + ec.message());

	Response resp;
	resp.Data.Set((uint8_t*)body.str().c_str(), (uint32_t)body.str().size());
	cb(ResponseResult::Success, std::move(resp));
}

void HttpClient::Post(const std::string& path, Bytes bytes, PayloadType type,
					  const ResponseCB& cb)
{
	
}

void HttpClient::Put(const std::string& path, Bytes bytes, PayloadType type,
					 const ResponseCB& cb)
{
}

void HttpClient::Delete(const std::string& path, const ResponseCB& cb) {

}

}