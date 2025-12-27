#include "VersionControl.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

#include "Networking/Networking.h"

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
		const git_error* e = git_error_last();
		printf("Repo::Init Error %d/%d: %s\n", error, e->klass, e->message);
		exit(error);
	}
}

void Repo::Open(const std::string& path) {
	int error = git_repository_open(&m_Repo, path.c_str());
	if(error < 0) {
		const git_error* e = git_error_last();
		printf("Repo::Open Error %d/%d: %s\n", error, e->klass, e->message);
		exit(error);
	}
}

void Repo::Clone(const std::string& url, const std::string& path) {
	int error = git_clone(&m_Repo, url.c_str(), path.c_str(), nullptr);
	if(error < 0) {
		const git_error* e = git_error_last();
		printf("Repo::Clone Error %d/%d: %s\n", error, e->klass, e->message);
		exit(error);
	}
}

void Repo::SetRemote(const std::string& url) {
	git_remote* remote = nullptr;
	int error =
		git_remote_create_with_fetchspec(&remote, m_Repo,
			"origin", url.c_str(), "+refs/heads/*:refs/custom/namespace/*");
	if(error < 0) {
		const git_error* e = git_error_last();
		printf("Repo::SetRemote Error %d/%d: %s\n", error, e->klass, e->message);
	}

	git_remote_free(remote);
}

void Repo::Stage() {
	git_index* index = nullptr;
	if(git_repository_index(&index, m_Repo) != 0)
		return;
	if(git_index_add_all(index, nullptr, 0, nullptr, nullptr) != 0) {
		git_index_free(index);
		return;
	}
	if(git_index_write(index) != 0) {
		git_index_free(index);
		return;
	}

	git_index_free(index);
}

void Repo::Commit(const std::string& message) {
	if(!m_Repo)
		return;

	int err = 0;
	git_index* index = nullptr;
	if(git_repository_index(&index, m_Repo) != 0)
		return;

	git_oid tree_oid;
	if(git_index_write_tree(&tree_oid, index) != 0) {
		git_index_free(index);
		return;
	}
	git_index_free(index);

	git_tree* tree = nullptr;
	if(git_tree_lookup(&tree, m_Repo, &tree_oid) != 0)
		return;

	std::string authorName = "Test User";
	std::string authorEmail = "example@example.com";
	git_signature *sig = nullptr;
	err = git_signature_now(&sig, authorName.c_str(), authorEmail.c_str());
	if(err < 0) {
		const git_error* e = git_error_last();
		printf("Repo::Commit::sig Error %d/%d: %s\n", err, e->klass, e->message);
		git_tree_free(tree);
		return;
	}

	git_reference *head_ref = nullptr;
	git_commit *parent = nullptr;
	int parent_count = 0;

	if(git_repository_head(&head_ref, m_Repo) == 0) {
		git_oid parent_oid;
		if(git_reference_name_to_id(&parent_oid, m_Repo, "HEAD") == 0)
			if(git_commit_lookup(&parent, m_Repo, &parent_oid) == 0)
				parent_count = 1;
	}

	git_oid commit_oid;
	err = git_commit_create_v(
		&commit_oid,
		m_Repo,
		"HEAD",
		sig, sig,
		nullptr,
		message.c_str(),
		tree,
		parent_count,
		parent_count ? (const git_commit**)&parent : nullptr
	);

	if(parent)
		git_commit_free(parent);
	if(head_ref)
		git_reference_free(head_ref);

	git_signature_free(sig);
	git_tree_free(tree);
}

void Repo::Pull() {
	git_remote* remote;
	if(git_remote_lookup(&remote, m_Repo, "origin") < 0)
		return;

	git_fetch_options opts;
	git_fetch_init_options(&opts, GIT_FETCH_OPTIONS_VERSION);
	opts.callbacks.credentials = CredentialCallback;

	int err = git_remote_fetch(remote, NULL, &opts, NULL);
	if(err < 0) {
		const git_error* e = git_error_last();
		printf("Repo::Pull Error %d/%d: %s\n", err, e->klass, e->message);
	}

	git_remote_free(remote);
}

void Repo::Push() {
	git_remote* remote;
	if(git_remote_lookup(&remote, m_Repo, "origin") < 0)
		return;

	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	callbacks.credentials = CredentialCallback;
	callbacks.certificate_check =
		[](git_cert *cert, int valid, const char *host, void *payload)
		{
			// Always accept
			return 0;
		};

	git_push_options opts;
	git_push_init_options(&opts, GIT_PUSH_OPTIONS_VERSION);
	// opts.callbacks.credentials = CredentialCallback;
	opts.callbacks = callbacks;

	int err = git_remote_push(remote, NULL, &opts);
	if(err < 0) {
		const git_error* e = git_error_last();
		printf("Repo::Push Error %d/%d: %s\n", err, e->klass, e->message);
	}

	git_remote_free(remote);
}

int Repo::CredentialCallback(git_credential** out, const char* url,
	const char* usernameFromURL, unsigned int allowedTypes, void* payload)
{
	std::string token = Networking::TokenStore::LoadToken("github");
	if(token.empty()) {
		VOLCANICORE_LOG_WARNING("Failed to load github token");
		return GIT_PASSTHROUGH;
	}
	if(allowedTypes & GIT_CREDENTIAL_USERPASS_PLAINTEXT)
		return
			git_cred_userpass_plaintext_new(
				out, "x-access-token", token.c_str());

	VOLCANICORE_LOG_WARNING("Failed to authenticate");
	return GIT_PASSTHROUGH;
}

}