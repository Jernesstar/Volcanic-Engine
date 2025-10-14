#pragma once

#include "Core/Integration.h"

namespace Magma::VersionControl {

class VersionControlManager : public Integration {
public:
	VersionControlManager() = default;
	~VersionControlManager() = default;

	void Init() override;
	void Shutdown() override;
};

class Repo {
public:
	Repo();
	~Repo();

	void Open(const std::string& path);
	void Clone(const std::string& url, const std::string& path);
	void Pull();
	void Push();
	void Commit(const std::string& message);
	void Add(const std::string& path);
	void Remove(const std::string& path);
	void Revert(const std::string& path);
	void Checkout(const std::string& path);
};

}