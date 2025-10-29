#pragma once

#include "Core/Integration.h"

#include <git2.h>

namespace Magma::VC {

class VCManager : public Integration {
public:
	VCManager() = default;
	~VCManager() = default;

	void Init() override;
	void Shutdown() override;
};

class Repo {
public:
	Repo();
	~Repo();

	void Init(const std::string& path);
	void SetRemote(const std::string& url);
	void Clone(const std::string& url, const std::string& path);
	void Open(const std::string& path);
	void Push();
	void Pull();
	void Stage();
	void Stage(const std::string& path);
	void Unstage();
	void Unstage(const std::string& path);
	void Commit(const std::string& message);
	// void Remove(const std::string& path);
	// void Revert(const std::string& path);
	// void Checkout(const std::string& path);

private:
	git_repository* m_Repo = nullptr;
	// git_remote* m_Remote = nullptr;

	static int CredentialCallback(git_credential** out, const char* url,
		const char* usernameFromURL, unsigned int allowedTypes, void* payload);
};

}