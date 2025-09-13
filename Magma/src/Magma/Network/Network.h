#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include <VolcaniCore/Core/Buffer.h>

using namespace asio::ip;
using namespace VolcaniCore;

namespace Magma::Network {

using Bytes = Buffer<uint8_t>;

struct ConnectionInfo {
	std::string PeerAddress;
	uint16_t PeerPort = 0;
};

enum ReturnCode {
	Success = 200,
	Redirect = 301,
	NotFound = 404
};

struct Response {
	ReturnCode code;
	std::string message;

	std::string ToString() const;
	rapidjson::Document ToJSON() const;
};

class Connection : public std::enable_shared_from_this<Connection> {
public:
	virtual void Start() = 0;
	virtual void Send(const Bytes& data) = 0;
	virtual void Close() = 0;
	virtual ConnectionInfo GetInfo() const = 0;
	virtual ~Connection() = default;
};

// struct HTTPRequest {
// 	std::string Path;
// 	std::string Host;
// };

}