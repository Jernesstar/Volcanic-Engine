#include "Database.h"

#include <VolcaniCore/Core/FileUtils.h>

namespace VolcanicEngine {

Database::Database(const std::string& name, MDB_env* registry, MDB_dbi handle,
				   bool multiValue)
	: Name(name), m_Handle(handle), m_Registry(registry),
	MultiValue(multiValue) { }

Database::~Database() {
	mdb_dbi_close(m_Registry, m_Handle);
}

void Database::Insert(DatabaseKey&& key, Bytes&& value) {
	MDB_txn* txn;
	MDB_val mdbKey, mdbValue;

	mdb_txn_begin(m_Registry, nullptr, 0, &txn);

	mdbKey.mv_size = key.Key.GetSize();
	mdbKey.mv_data = (void*)key.Key.Get();

	mdbValue.mv_size = value.GetSize();
	mdbValue.mv_data = (void*)value.Get();

	int rc = mdb_put(txn, m_Handle, &mdbKey, &mdbValue, MDB_NODUPDATA & MultiValue);
	if(rc != 0) {
		mdb_txn_abort(txn);
		throw std::runtime_error("Failed to insert data!");
	}

	mdb_txn_commit(txn);
}

DatabaseValueCount Database::Count(DatabaseKey&& key) {
	MDB_txn* txn;
	MDB_val mdbKey, mdbValue;

	mdb_txn_begin(m_Registry, nullptr, 0, &txn);

	mdbKey.mv_size = key.Key.GetSize();
	mdbKey.mv_data = (void*)key.Key.Get();

	mdbValue.mv_size = 0;
	mdbValue.mv_data = nullptr;

	MDB_cursor* cursor;
	mdb_cursor_open(txn, m_Handle, &cursor);
	
	size_t count = 0;
	int rc = mdb_cursor_get(cursor, &mdbKey, &mdbValue, MDB_SET);
	if(rc == MDB_SUCCESS)
		mdb_cursor_count(cursor, &count);
	else if (rc == MDB_NOTFOUND) {
		mdb_txn_abort(txn);
		return { false };
	}

	return { true, count };
	mdb_cursor_close(cursor);
	mdb_txn_commit(txn);
}

DatabaseResult Database::Query(DatabaseKey&& query) {
	MDB_txn* txn;
	MDB_val key, value;

	mdb_txn_begin(m_Registry, nullptr, MDB_RDONLY, &txn);

	key.mv_size = query.Key.GetSize();
	key.mv_data = (void*)query.Key.Get();

	int rc = mdb_get(txn, m_Handle, &key, &value);
	if(rc == MDB_NOTFOUND) {
		mdb_txn_abort(txn);
		return { false };
	}
	else if(rc != 0) {
		mdb_txn_abort(txn);
		throw std::runtime_error("Failed to read bytes!");
	}

	DatabaseResult val = {
		true, Bytes((uint8_t*)value.mv_data, value.mv_size, 0, false)
	};

	mdb_txn_commit(txn);
	return val;
}

void Database::Remove(DatabaseKey&& key) {
	MDB_txn* txn;
	MDB_val mdbKey;

	mdb_txn_begin(m_Registry, nullptr, 0, &txn);

	mdbKey.mv_size = key.Key.GetSize();
	mdbKey.mv_data = (void*)key.Key.Get();

	int rc = mdb_del(txn, m_Handle, &mdbKey, nullptr);
	if(rc != 0) {
		mdb_txn_abort(txn);
		throw std::runtime_error("Failed to remove bytes!");
	}

	mdb_txn_commit(txn);
}

Registry::Registry(const std::string& path, u32 maxDatabases) {
	fs::create_directories(path);
	mdb_env_create(&m_Handle);
	mdb_env_set_maxdbs(m_Handle, maxDatabases);
	mdb_env_open(m_Handle, path.c_str(), 0, 0664);
}

Registry::~Registry() {
	m_Databases.Clear();
	mdb_env_close(m_Handle);
}

Database* Registry::NewDatabase(const std::string& name, bool multiValue) {
	MDB_txn* txn;
	MDB_dbi dbi;

	mdb_txn_begin(m_Handle, nullptr, 0, &txn);
	mdb_dbi_open(txn, name.c_str(), MDB_CREATE | (multiValue ? 0 : MDB_DUPSORT), &dbi);
	mdb_txn_commit(txn);

	return &m_Databases.Emplace(name, m_Handle, dbi, multiValue);
}

Database* Registry::GetDatabase(const std::string& name) {
	auto [found, i] =
		m_Databases.Find([&](auto& db) { return db.Name == name; });
	if(!found)
		return nullptr;
	return m_Databases.At(i);
}

}