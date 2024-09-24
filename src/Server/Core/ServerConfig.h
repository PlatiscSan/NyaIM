#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <string>

namespace NyaIMServer::core::config {


	void LoadConfig(int argc, char** argv);

	std::string const& DBServer() noexcept;
	std::string const& DBLogin() noexcept;
	std::string const& DBAuthentication() noexcept;
	std::uint16_t const& ServerPort() noexcept;
	std::string const& AuthenticationTable() noexcept;


}

#endif // !SERVER_CONFIG_H
