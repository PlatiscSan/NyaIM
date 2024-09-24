#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <any>
#include <functional>

#include <sql.h>
#include <sqlext.h>

namespace NyaIMServer::core::db {

	class DBConnection final {

	public:

		DBConnection();
		~DBConnection() noexcept;

		void ExecuteQuery(std::string const& sql, std::function<void(SQLHSTMT& stmt)> const& callback) const;

	private:

		SQLHENV m_env = {};
		SQLHDBC m_dbc = {};

	};

	void HandleDiagnosticRecord(SQLHANDLE handle, SQLSMALLINT type, RETCODE ret);

	void InitializeDBConnectionPool(std::size_t max_connection);
	void CleanUp();
	std::unique_ptr<DBConnection> RequestConnection();
	void ReturnConnection(std::unique_ptr<DBConnection>& connection);

}

#endif // !DB_CONNECTION_H
