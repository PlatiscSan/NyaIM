
#include "pch.h"
#include "DBConnection.h"
#include "Common/NyaIMType.h"

#ifdef SQLConnect
	#undef SQLConnect
#endif // SQLConnect

#ifdef SQLExecDirect
	#undef SQLExecDirect
#endif // SQLExecDirect

#ifdef SQLGetDiagRec
	#undef SQLGetDiagRec
#endif // SQLGetDiagRec



using namespace NyaIMServer::core::db;

static std::queue<std::shared_ptr<DBConnection>> s_db_connections;
static std::mutex s_db_connections_mutex;
static std::condition_variable s_condition;
static std::atomic_bool s_is_running = false;

NyaIMServer::core::db::DBConnection::DBConnection(config::ServerConfig const& config)
	:m_config(config) {


	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env);
	SQLSetEnvAttr(m_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_dbc);

	auto ret = SQLConnect(
		m_dbc, 
		(SQLCHAR*)config.db_server_name.c_str(), SQL_NTS,
		(SQLCHAR*)config.db_user_name.c_str(), SQL_NTS,
		(SQLCHAR*)config.db_authentication.c_str(), SQL_NTS
	);

	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		std::array<SQLCHAR, 1024> sql_state;
		SQLINTEGER native_error;
		SQLSMALLINT text_length;
		std::array<SQLCHAR, 1024> message_text[1024];
		SQLGetDiagRec(SQL_HANDLE_DBC, m_dbc, 1, sql_state.data(), &native_error, message_text->data(), 1024, &text_length);
		throw std::runtime_error("Connection to SQL Server failed: " + std::string((char*)message_text->data(), text_length));
	}

	//SQLAllocHandle(SQL_HANDLE_STMT, m_dbc, &m_stmt);

	//std::ostringstream oss;
	//oss << "use " << m_config.db_server_name << ";" << "\0";
	//SQLExecDirect(m_stmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);



}

NyaIMServer::core::db::DBConnection::~DBConnection() noexcept
{
}

void NyaIMServer::core::db::DBConnection::ExecuteQuery(std::string const& query) {



}

void NyaIMServer::core::db::InitializeDBConnectionPool(std::size_t max_connection, config::ServerConfig const& config) {

	if (s_is_running) {
		return;
	}

	std::lock_guard<std::mutex> lock(s_db_connections_mutex);
	for (std::size_t i = 0; i < max_connection; i++) {
		s_db_connections.push(std::make_shared<DBConnection>(config));
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

std::shared_ptr<DBConnection> NyaIMServer::core::db::RequestConnection() {

	std::unique_lock<std::mutex> lock(s_db_connections_mutex);
	s_condition.wait(lock, []() {return !s_db_connections.empty(); });

	std::shared_ptr<DBConnection> connection = std::move(s_db_connections.front());
	s_db_connections.pop();

	return connection;

}

void NyaIMServer::core::db::ReturnConnection(std::shared_ptr<DBConnection>& connection) {

	std::lock_guard<std::mutex> lock(s_db_connections_mutex);
	s_db_connections.push(std::move(connection));

}
