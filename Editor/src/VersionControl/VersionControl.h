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
	void Open(const std::string& path);
	// void Clone(const std::string& url, const std::string& path);
	// void Pull();
	// void Push();
	// void Commit(const std::string& message);
	// void Add(const std::string& path);
	// void Remove(const std::string& path);
	// void Revert(const std::string& path);
	// void Checkout(const std::string& path);

private:
	git_repository *m_Repo = nullptr;
};

}