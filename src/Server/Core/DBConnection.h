#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include "ServerConfig.h"

#include <sql.h>
#include <sqlext.h>

namespace NyaIMServer::core::db {

	class DBConnection {

	public:

		DBConnection(config::ServerConfig const& config);
		virtual ~DBConnection() noexcept;

		void ExecuteQuery(std::string const& query);

	protected:

		SQLHENV m_env = {};
		SQLHDBC m_dbc = {};
		SQLHSTMT m_stmt = {};
		config::ServerConfig m_config;

	};

	void InitializeDBConnectionPool(std::size_t max_connection, config::ServerConfig const& config);
	void CleanUp();
	std::shared_ptr<DBConnection> RequestConnection();
	void ReturnConnection(std::shared_ptr<DBConnection>& connection);

}

#endif // !DB_CONNECTION_H
