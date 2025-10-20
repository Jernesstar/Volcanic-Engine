#pragma once

#include <drogon/drogon.h>

#include <VolcaniCore/Core/Application.h>

#include "UserService.h"
#include "GitHubOAuth.h"

using namespace drogon;
using namespace VolcaniCore;

namespace Magma::Server {

class ServerApp : public Application {
public:
	ServerApp();
	~ServerApp();

	void OnUpdate(TimeStep ts) override { }

private:
	std::thread m_HttpThread;
};

ServerApp::ServerApp() {
	VOLCANICORE_LOG_INFO("Starting HTTP thread");

	m_HttpThread = std::thread(
		[this]()
		{
			// UserService::Init();

			app()
				.addListener("127.0.0.1", 8848)
				.run();
		});

	m_HttpThread.detach();
}

ServerApp::~ServerApp() {
	VOLCANICORE_LOG_INFO("Stopping HTTP thread");
}

}