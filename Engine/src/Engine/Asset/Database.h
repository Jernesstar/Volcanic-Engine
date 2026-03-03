#pragma once

#include <lmdb.h>

#include <VolcaniCore/Core/Buffer.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

using Bytes = Buffer<u8>;

namespace VolcanicEngine {

struct DatabaseKey {
	Bytes Key;

	DatabaseKey(Bytes&& key)
		: Key(std::move(key)) { }
	DatabaseKey(const std::string& key)
		: Key((u8*)key.data(), key.size(), 0, false) { }
	DatabaseKey(const char* key)
		: Key((u8*)key, strlen(key), 0, false) { }
	DatabaseKey(u64 key)
		: Key((u8*)&key, sizeof(u64), 0, false) { }
};

struct DatabaseResult {
	bool Success = false;
	Bytes Data;

	operator bool() const { return Success; }
	template<typename T>
	T& Get() {
		return *(T*)Data.Get();
	}
};

class Registry;

class Database {
public:
	const std::string Name;

public:
	Database(const std::string& name, MDB_env* registry, MDB_dbi handle);
	~Database();

	void Insert(DatabaseKey&& key, Bytes&& value);
	DatabaseResult Query(DatabaseKey&& key);
	void Remove(DatabaseKey&& key);

private:
	MDB_env* m_Registry;
	MDB_dbi m_Handle;
};

class Registry {
public:
	Registry(const std::string& path, u32 maxDatabases);
	~Registry();

	Database* NewDatabase(const std::string& name);
	Database* GetDatabase(const std::string& name);

private:
	MDB_env* m_Handle;
	List<Database> m_Databases;
};

}