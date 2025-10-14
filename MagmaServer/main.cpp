#include <VolcaniCore/Core/CommandLineArgs.h>

#include "ServerApp.h"

Application* CreateApplication(const CommandLineArgs& args) {
	return new Magma::Server::ServerApp();
}