#include "VersionControl.h"

#include <VolcaniCore/Core/Assert.h>

namespace Magma::VC {

void VCManager::Init() {
	git_libgit2_init();
}

void VCManager::Shutdown() {
	git_libgit2_shutdown();
}

Repo::Repo() {

}

Repo::~Repo() {
	git_repository_free(m_Repo);
}

void Repo::Init(const std::string& path) {
	VOLCANICORE_LOG_INFO("Initializing repo at '%s'", path.c_str());
	int error = git_repository_init(&m_Repo, path.c_str(), 0);
	if(error < 0) {
		const git_error *e = git_error_last();
		printf("Error %d/%d: %s\n", error, e->klass, e->message);
		exit(error);
	}
}

void Repo::Open(const std::string& path) {
	git_repository_open(&m_Repo, path.c_str());
}

}