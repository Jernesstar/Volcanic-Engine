#include "Networking.h"

using namespace drogon;

namespace Magma::Networking {

void HttpClient::Get(const std::string& path,
					 const Func<void, const drogon::HttpResponsePtr&> cb)
{
	auto req = HttpRequest::newHttpRequest();
	req->setPath(path);
	req->setMethod(HttpMethod::Get);
	req->setContentTypeCode(CT_TEXT_PLAIN);
	m_Client->sendRequest(req,
		[&](ReqResult res, const HttpResponsePtr& response)
		{
			if(res == ReqResult::Ok)
				cb(response);
			else
				cb(nullptr);
		});
}

void HttpClient::Post(const std::string& path, Bytes bytes, PayloadType type,
					  const Func<void, const drogon::HttpResponsePtr&> cb)
{
	
}

void HttpClient::Put(const std::string& path, Bytes bytes, PayloadType type,
					 const Func<void, const drogon::HttpResponsePtr&> cb)
{
}

void HttpClient::Delete(const std::string& path,
					const Func<void, const drogon::HttpResponsePtr&> cb)
{
}

}