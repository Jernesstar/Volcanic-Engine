#pragma once

#include <drogon/drogon.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Event/Events.h>

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
				.setThreadNum(4)
				.addListener("127.0.0.1", 8848)
				.run();
		});

	m_HttpThread.detach();

	Events::RegisterListener<KeyPressedEvent>(
		[&](const KeyPressedEvent& event)
		{
			if(event.Key == Key::Escape)
				Application::Close();
		});
}

ServerApp::~ServerApp() {
	VOLCANICORE_LOG_INFO("Stopping HTTP thread");
	app().quit();
}

}