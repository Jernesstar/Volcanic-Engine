#include "VersionControl.h"

#include <git2.h>

namespace Magma::VersionControl {

void VersionControlManager::Init() {
	git_libgit2_init();
}

void VersionControlManager::Shutdown() {
	git_libgit2_shutdown();
}

}