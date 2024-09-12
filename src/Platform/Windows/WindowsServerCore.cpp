
#include "pch.h"
#include "WindowsServerCore.h"
#include "WindowsUtility.h"
#include "Server/Core/ServerConfig.h"
#include "Server/Core/ServerLog.h"

using namespace NyaIMServer::core;
using namespace NyaIM::utility::windows;

NyaIMServer::core::WindowsServerCore::WindowsServerCore(int argc, char** argv) {

	InitializeServer(argc, argv);
	InitializeServerSocket();

}

NyaIMServer::core::WindowsServerCore::~WindowsServerCore() noexcept {

	m_is_stop = true;
	if (m_listen_thread.joinable()) {
		m_listen_thread.join();
	}

	std::lock_guard<std::mutex> lock(m_clients_mutex);
	for (auto& client : m_clients) {
		shutdown(client, SD_BOTH);
		closesocket(client);
	}

	m_clients.clear();
	closesocket(m_listen_socket);

	CleanUpWSA();
	log::QuitLog();

}

void NyaIMServer::core::WindowsServerCore::InitializeServer(int argc, char** argv) {

	log::InitializeLog();
	log::Info("log initalized");

	auto server_config = config::LoadConfig(argc, argv);
	log::Info("config file loaded");


}

void NyaIMServer::core::WindowsServerCore::InitializeServerSocket() {

	InitializeWSA();

	addrinfo* result = nullptr, * ptr = nullptr, hints = { 0 };
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int error = 0;
	error = getaddrinfo(nullptr, std::to_string(m_server_port).c_str(), &hints, &result);
	if (error) {
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

	m_listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_listen_socket == INVALID_SOCKET) {
		freeaddrinfo(result);
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

	error = bind(m_listen_socket, result->ai_addr, static_cast<int>(result->ai_addrlen));
	freeaddrinfo(result);

	if (error == SOCKET_ERROR) {
		closesocket(m_listen_socket);
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

	if (listen(m_listen_socket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(m_listen_socket);
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

	m_listen_thread = std::thread(
		[this]() mutable {
			while (!m_is_stop) {
				SOCKET client = accept(m_listen_socket, nullptr, nullptr);
				if (client == INVALID_SOCKET) {
					continue;
				}
				std::unique_lock<std::mutex> lock(m_clients_mutex);
				m_clients.push_back(client);
				lock.unlock();
			}
		}
	);

}

void NyaIMServer::core::WindowsServerCore::ServerLoop()
{
}

void NyaIMServer::core::WindowsServerCore::HandleConnection()
{
}
