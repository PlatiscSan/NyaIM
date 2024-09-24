#ifndef LOG_H
#define LOG_H

#include <string>

namespace common_dev::log {

	void InitializeLog(std::string const& log_name);
	bool IsLogRunning();
	void QuitLog();

	void Normal(std::string const& msg);
	void Info(std::string const& msg);
	void Debug(std::string const& msg);
	void Warning(std::string const& msg);
	void Error(std::string const& msg);
	void Fatal(std::string const& msg);

}

#endif // !LOG_H
