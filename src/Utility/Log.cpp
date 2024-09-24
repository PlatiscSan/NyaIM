
#include "Log.h"

#include <chrono>
#include <filesystem>

#include <fstream>
#include <iostream>

#include <queue>
#include <unordered_map>

#include <mutex>
#include <thread>

enum LogRank {

	NORMAL,
	INFO,
	DEBUG,
	WARNING,
	ERROR,
	FATAL,

};

struct Log {

	std::chrono::system_clock::time_point const timestamp = std::chrono::system_clock::now();
	std::string msg;
	LogRank rank;

};

static std::ofstream s_stream;
static std::queue<Log> s_logs;
static std::mutex s_logs_mutex;
static std::thread s_log_thread;
static std::condition_variable s_logs_condition;

static std::atomic_bool s_is_quit = true;


static std::unordered_map<LogRank, std::string> s_rank_string = {

	{LogRank::NORMAL, "[Normal] "},

	{LogRank::INFO, "[Info] "},

	{LogRank::DEBUG, "[Debug] "},

	{LogRank::WARNING, "[Warning] "},

	{LogRank::ERROR, "[Error] "},

	{LogRank::FATAL, "[Fatal] "}

};

static void LogThreadProc() {

	std::array<char, 64> time_string{};

	while (!s_is_quit) {

		std::unique_lock<std::mutex> logs_lock(s_logs_mutex);
		s_logs_condition.wait(logs_lock, [&]() {return !s_logs.empty() || s_is_quit; });
		if (s_logs.empty() && s_is_quit) {
			return;
		}

		if (!s_logs.empty()) {

			Log log = std::move(s_logs.front());
			s_logs.pop();

			std::time_t raw_time = std::chrono::system_clock::to_time_t(log.timestamp);
			std::tm time_info{};
			localtime_s(&time_info, &raw_time);
			std::strftime(time_string.data(), time_string.size(), "[%Y-%m-%d %H:%M:%S] ", &time_info);

			s_stream << time_string.data() << s_rank_string[log.rank] << log.msg << std::endl;

		#if LOG_WRITE_CONSOLE
			std::cout << time_string.data() << s_rank_string[log.rank] << log.msg << std::endl;
		#endif // WRITE_CONSOLE

		}

	}

}

void common_dev::log::InitializeLog(std::string const& log_name) {

	if (!s_is_quit) {
		return;
	}

	s_is_quit = false;

	auto now = std::chrono::system_clock::now();
	std::time_t raw_time = std::chrono::system_clock::to_time_t(now);
	std::tm time_info{};

	localtime_s(&time_info, &raw_time);
	std::array<char, 64> time_string{};
	std::strftime(time_string.data(), time_string.size(), "%Y-%m-%d", &time_info);

	std::filesystem::path log_folder = "log";
	std::filesystem::path file = log_name + "_" + time_string.data() + ".log";

	if (!std::filesystem::exists(log_folder)) {
		if (!std::filesystem::create_directories(log_folder)) {
			throw std::runtime_error("Failed to create log directory");
		}
	}

	std::filesystem::path full_path = log_folder/file;

	s_stream.open(full_path.string(), std::ios::app);
	if (!s_stream) {
		throw std::runtime_error("Failed to create log file");
	}

	s_log_thread = std::thread(LogThreadProc);

}

bool common_dev::log::IsLogRunning() {
	return !s_is_quit;
}

void common_dev::log::QuitLog() {

	if (s_is_quit) {
		return;
	}

	s_is_quit = true;

	s_logs_condition.notify_one();

	if (s_log_thread.joinable()) {
		s_log_thread.join();
	}

	s_stream.close();

}



void common_dev::log::Normal(std::string const& msg) {

	std::lock_guard<std::mutex> lock(s_logs_mutex);
	Log log{};
	log.msg = msg;
	log.rank = LogRank::NORMAL;
	s_logs.push(log);
	s_logs_condition.notify_one();

}

void common_dev::log::Info(std::string const& msg) {

	std::lock_guard<std::mutex> lock(s_logs_mutex);
	Log log{};
	log.msg = msg;
	log.rank = LogRank::INFO;
	s_logs.push(log);
	s_logs_condition.notify_one();

}

void common_dev::log::Debug(std::string const& msg) {

	std::lock_guard<std::mutex> lock(s_logs_mutex);
	Log log{};
	log.msg = msg;
	log.rank = LogRank::DEBUG;
	s_logs.push(log);
	s_logs_condition.notify_one();

}

void common_dev::log::Warning(std::string const& msg) {

	std::lock_guard<std::mutex> lock(s_logs_mutex);
	Log log{};
	log.msg = msg;
	log.rank = LogRank::WARNING;
	s_logs.push(log);
	s_logs_condition.notify_one();

}

void common_dev::log::Error(std::string const& msg) {

	std::lock_guard<std::mutex> lock(s_logs_mutex);
	Log log{};
	log.msg = msg;
	log.rank = LogRank::ERROR;
	s_logs.push(log);
	s_logs_condition.notify_one();

}

void common_dev::log::Fatal(std::string const& msg) {

	std::lock_guard<std::mutex> lock(s_logs_mutex);
	Log log{};
	log.msg = msg;
	log.rank = LogRank::FATAL;
	s_logs.push(log);
	s_logs_condition.notify_one();

}