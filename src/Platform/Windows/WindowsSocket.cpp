
#include "pch.h"
#include "WindowsSocket.h"
#include "WindowsUtility.h"

using namespace NyaIM::core;
using namespace NyaIM::utility::windows;

static WSADATA s_wsa_data;
static std::atomic_bool s_is_initialized = false;

NyaIM::core::WindowTCPClientSocket::WindowTCPClientSocket(std::string const& host, std::uint32_t port) {

	if (!s_is_initialized) {
		InitializeWSA();
	}

	addrinfo *result = nullptr, hints = { 0 };
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int error = 0;
	error = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
	if (error) {
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}
	
	m_connection = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_connection == INVALID_SOCKET) {
		freeaddrinfo(result);
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

	error = connect(m_connection, result->ai_addr, static_cast<int>(result->ai_addrlen));
	if (error) {
		closesocket(m_connection);
		m_connection = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (m_connection == INVALID_SOCKET) {
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

}

NyaIM::core::WindowTCPClientSocket::~WindowTCPClientSocket() noexcept {

	closesocket(m_connection);

}

int NyaIM::core::WindowTCPClientSocket::Send(void* buffer, std::size_t length) {

	int result = send(m_connection, reinterpret_cast<char*>(buffer), static_cast<int>(length), 0);
	if (result == SOCKET_ERROR) {
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

	return static_cast<std::size_t>(result);

}

int NyaIM::core::WindowTCPClientSocket::Receive(void* buffer, std::size_t length) {

	int result = recv(m_connection, reinterpret_cast<char*>(buffer), static_cast<int>(length), 0);

	if (result) {
		return static_cast<std::size_t>(result);
	}
	else if (result == 0) {
		throw std::runtime_error("Connection Lost");
	}
	else {
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

}

NyaIM::core::WindowsTCPServerSocket::WindowsTCPServerSocket(std::uint32_t port) {

	if (!s_is_initialized) {
		InitializeWSA();
	}

	addrinfo* result = nullptr, * ptr = nullptr, hints = { 0 };
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int error = 0;
	error = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &result);
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

NyaIM::core::WindowsTCPServerSocket::~WindowsTCPServerSocket() noexcept {

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

}

int NyaIM::core::WindowsTCPServerSocket::Send(void* buffer, std::size_t length) {

	int result = send(m_connection, reinterpret_cast<char*>(buffer), static_cast<int>(length), 0);
	if (result == SOCKET_ERROR) {
		throw std::runtime_error(GetLastErrorMessageFromWSA());
	}

	return static_cast<std::size_t>(result);

}

int NyaIM::core::WindowsTCPServerSocket::Receive(void* buffer, std::size_t length) {
	return std::size_t();
}

void NyaIM::core::InitializeWSA() {

	int error = 0;
	error = WSAStartup(MAKEWORD(2, 2), &s_wsa_data);
	if (error) {
		switch (error) {
		case WSASYSNOTREADY:
			throw std::runtime_error("The underlying network subsystem is not ready for network communication.");
		case WSAVERNOTSUPPORTED:
			throw std::runtime_error("The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.");
		case WSAEINPROGRESS:
			throw std::runtime_error("A blocking Windows Sockets 1.1 operation is in progress.");
		case WSAEPROCLIM:
			throw std::runtime_error("A limit on the number of tasks supported by the Windows Sockets implementation has been reached.");
		case WSAEFAULT:
			throw std::runtime_error("The lpWSAData parameter is not a valid pointer.");

		}
	}

	if (LOBYTE(s_wsa_data.wVersion) != 2 || HIBYTE(s_wsa_data.wVersion) != 2) {
		WSACleanup();
		throw std::runtime_error("winsock 2.2 is unavailable");
	}

	s_is_initialized = true;

}

void NyaIM::core::CleanUpWSA() {

	WSACleanup();
	s_is_initialized = false;

}
