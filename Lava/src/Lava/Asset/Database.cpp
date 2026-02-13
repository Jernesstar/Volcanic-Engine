#include "Database.h"

namespace Lava {

Database::Database(const std::string& name, MDB_env* registry, MDB_dbi handle)
	: Name(name), m_Handle(handle) { }

void Database::Insert(const Bytes& key, const Bytes& value) {
	MDB_txn* txn;
	MDB_val mdbKey, mdbValue;

	mdb_txn_begin(m_Registry, nullptr, 0, &txn);

	mdbKey.mv_size = key.GetSize();
	mdbKey.mv_data = (void*)key.Get();

	mdbValue.mv_size = value.GetSize();
	mdbValue.mv_data = (void*)value.Get();

	int rc = mdb_put(txn, m_Handle, &mdbKey, &mdbValue, 0);
	if(rc != 0) {
		mdb_txn_abort(txn);
		throw std::runtime_error("Failed to insert asset!");
	}

	mdb_txn_commit(txn);
}

DatabaseResult Database::Query(const DatabaseQuery& query) {
	MDB_txn* txn;
	MDB_val key, value;

	mdb_txn_begin(m_Registry, nullptr, MDB_RDONLY, &txn);

	key.mv_size = query.Key.GetSize();
	key.mv_data = (void*)query.Key.Get();

	int rc = mdb_get(txn, m_Handle, &key, &value);
	if(rc == MDB_NOTFOUND) {
		mdb_txn_abort(txn);
		return { };
	} else if(rc != 0) {
		mdb_txn_abort(txn);
		throw std::runtime_error("Failed to read bytes!");
	}

	// TODO(Watch out!)
	DatabaseResult val = { true, { (uint8_t*)value.mv_data, value.mv_size } };

	mdb_txn_commit(txn);
	return val;
}

Registry::Registry(const std::string& path, uint32_t maxDatabases) {
	mdb_env_create(&m_Handle);
	mdb_env_set_maxdbs(m_Handle, 8);
	mdb_env_open(m_Handle, path.c_str(), 0, 0664);
}

Registry::~Registry() {

}

Database* Registry::NewDatabase(const std::string& name) {
	MDB_txn* txn;
	MDB_dbi dbi;

	mdb_txn_begin(m_Handle, nullptr, 0, &txn);
	mdb_dbi_open(txn, name.c_str(), MDB_CREATE, &dbi);
	mdb_txn_commit(txn);

	return &m_Databases.Emplace(name, m_Handle, dbi);
}

Database* Registry::GetDatabase(const std::string& name) {
	auto [found, i] =
		m_Databases.Find([&](auto& db) { return db.Name == name; });
	if(!found)
		return nullptr;
	return m_Databases.At(i);
}

}