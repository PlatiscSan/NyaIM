#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <string>

namespace NyaIMServer::core::config {

	struct ServerConfig {

		std::string db_server_name;
		std::string db_user_name;
		std::string db_authentication;
		std::string server_port;

	};

	ServerConfig LoadConfig(int argc, char** argv);


}

#endif // !SERVER_CONFIG_H
