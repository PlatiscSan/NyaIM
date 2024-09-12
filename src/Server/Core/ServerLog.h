#ifndef SERVER_LOG_H
#define SERVER_LOG_H

#include <string>

namespace NyaIMServer::core::log {

	void InitializeLog();
	void QuitLog();

	void Normal(std::string const& msg);
	void Info(std::string const& msg);
	void Debug(std::string const& msg);
	void Warning(std::string const& msg);
	void Error(std::string const& msg);
	void Fatal(std::string const& msg);

}

#endif // !SERVER_LOG_H
