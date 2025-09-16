#pragma once

#include <lmdb.h>

#include "Buffer.h"

using namespace VolcaniCore;

using Bytes = Buffer<uint8_t>;

namespace Magma {

struct DatabaseQuery {
	
};

struct DatabaseResult {
	bool Success = false;
	Bytes Data;

	template<typename T>
	T Get() {
		return *(T*)Data.Get();
	}
};

class Registry;

class Database {
public:
	void Insert(const Bytes& key, const Bytes& value);
	DatabaseResult Query(const DatabaseQuery& query);

private:
	
	Registry* m_Registry;
};

class Registry {
public:
	Registry(const std::string& path, uint32_t maxDatabases);
	~Registry();

	Database* NewDatabase(const std::string& name);
	Database* GetDatabase(const std::string& name);
}

}