
#include "pch.h"
#include "DBConnection.h"
#include "ServerConfig.h"

using namespace NyaIMServer::core::db;
using namespace NyaIMServer::core::config;

static std::queue<std::unique_ptr<DBConnection>> s_db_connections;
static std::mutex s_db_connections_mutex;
static std::condition_variable s_condition;
static std::atomic_bool s_is_running = false;

NyaIMServer::core::db::DBConnection::DBConnection() {

	auto ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env);
	if (!SQL_SUCCEEDED(ret)) {
		throw std::runtime_error("Connection to SQL Server: failed to allocate environment handle");
	}

	ret = SQLSetEnvAttr(m_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);	
	if (!SQL_SUCCEEDED(ret)) {
		HandleDiagnosticRecord(m_env, SQL_HANDLE_ENV, ret);
		throw std::runtime_error("Connection to SQL Server: failed to set environment attribute");
	}

	ret = SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_dbc);
	if (!SQL_SUCCEEDED(ret)) {
		HandleDiagnosticRecord(m_env, SQL_HANDLE_ENV, ret);
		throw std::runtime_error("Connection to SQL Server: failed to allocate database connection handle ");
	}

	ret = SQLConnect(
		m_dbc,
		(SQLCHAR*)DBServer().c_str(), SQL_NTS,
		(SQLCHAR*)DBLogin().c_str(), SQL_NTS,
		(SQLCHAR*)DBAuthentication().c_str(), SQL_NTS
	);

	HandleDiagnosticRecord(m_dbc, SQL_HANDLE_DBC, ret);
	if (!SQL_SUCCEEDED(ret)) {
		throw std::runtime_error("Connection to SQL Server: failed to connect to SQL Server ");
	}

}

NyaIMServer::core::db::DBConnection::~DBConnection() noexcept {

	SQLDisconnect(m_dbc);
	SQLFreeHandle(SQL_HANDLE_DBC, m_dbc);
	SQLFreeHandle(SQL_HANDLE_ENV, m_env);

}

void NyaIMServer::core::db::DBConnection::ExecuteQuery(std::string const& sql, std::function<void(SQLHSTMT& stmt)> const& callback) const {

	SQLHSTMT stmt = nullptr;
	auto ret = SQLAllocHandle(SQL_HANDLE_STMT, m_dbc, &stmt);
	if (!SQL_SUCCEEDED(ret)) {
		HandleDiagnosticRecord(m_dbc, SQL_HANDLE_DBC, ret);
		throw std::runtime_error("Execute SQL error: failed to allocate statement handle");
	}

	ret = SQLPrepare(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		HandleDiagnosticRecord(stmt, SQL_HANDLE_STMT, SQLPrepare(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS));
		throw std::runtime_error("Execute SQL error: failed to prepare statement handle");
	}

	try {
		callback(stmt);
	}
	catch (std::exception const&) {
		SQLFreeHandle(SQL_HANDLE_STMT, stmt);
		throw;
	}

	if (stmt) {
		SQLFreeHandle(SQL_HANDLE_STMT, stmt);
	}

}

void NyaIMServer::core::db::HandleDiagnosticRecord(SQLHANDLE handle, SQLSMALLINT type, RETCODE ret) {

	std::array<SQLCHAR, SQL_SQLSTATE_SIZE + 1> sql_state = { 0 };
	std::array<SQLCHAR, 1024> message_text = { 0 };
	SQLINTEGER native_error = 0;
	SQLSMALLINT rec = 0;

	std::ostringstream oss;

	if (ret == SQL_INVALID_HANDLE) {
		throw std::runtime_error("Invalid SQL handle!");
	}


	while (SQLGetDiagRec(type, handle,
		++rec, sql_state.data(), &native_error,
		message_text.data(), static_cast<SQLUSMALLINT>(message_text.size()),
		nullptr) == SQL_SUCCESS) {
		oss.str("");
		if (std::strcmp((char const*)sql_state.data(), "01004")) {
			oss << "[" << sql_state.data() << "]" << message_text.data() << "(" << native_error << ")";
			if (SQL_SUCCEEDED(ret)) {
				common_dev::log::Info(oss.str());
			}
			else {
				common_dev::log::Error(oss.str());
			}
		}
	}

}

void NyaIMServer::core::db::InitializeDBConnectionPool(std::size_t max_connection) {

	if (s_is_running) {
		return;
	}

	std::lock_guard<std::mutex> lock(s_db_connections_mutex);

	while (!s_db_connections.empty()) {
		s_db_connections.pop();
	}

	for (std::size_t i = 0; i < max_connection; i++) {
		s_db_connections.push(std::make_unique<DBConnection>());
	}

	s_is_running = true;

}

void NyaIMServer::core::db::CleanUp() {

	std::lock_guard<std::mutex> lock(s_db_connections_mutex);
	for (std::size_t i = 0, size = s_db_connections.size(); i < size; i++) {
		s_db_connections.pop();
	}
	s_is_running = false;

}

std::unique_ptr<DBConnection> NyaIMServer::core::db::RequestConnection() {

	std::unique_lock<std::mutex> lock(s_db_connections_mutex);
	s_condition.wait(lock, []() {return !s_db_connections.empty(); });

	std::unique_ptr<DBConnection> connection = std::move(s_db_connections.front());
	s_db_connections.pop();

	return connection;

}

void NyaIMServer::core::db::ReturnConnection(std::unique_ptr<DBConnection>& connection) {

	std::lock_guard<std::mutex> lock(s_db_connections_mutex);
	s_db_connections.push(std::move(connection));
	s_condition.notify_one();

}
