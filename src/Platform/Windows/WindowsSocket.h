#ifndef WINDOWS_SOCKET_H
#define WINDOWS_SOCKET_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include "Core/Socket.h"

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib")

namespace NyaIM::core {

	class WindowTCPClientSocket final : public Socket {

	public:

		WindowTCPClientSocket(std::string const& host, std::uint32_t port);
		~WindowTCPClientSocket() noexcept;

		int Send(void* buffer, std::size_t length) override;
		int Receive(void* buffer, std::size_t length) override;

	private:

		SOCKET m_connection = INVALID_SOCKET;

	};

	class WindowsTCPServerSocket : public Socket {

	public:

		WindowsTCPServerSocket(std::uint32_t port);
		~WindowsTCPServerSocket() noexcept;

		int Send(void* buffer, std::size_t length) override;
		int Receive(void* buffer, std::size_t length) override;

	protected:

		SOCKET m_listen_socket = INVALID_SOCKET;

		std::mutex m_clients_mutex;
		std::list<SOCKET> m_clients;

		std::thread m_listen_thread;
		std::atomic_bool m_is_stop = false;


	};

	void InitializeWSA();
	void CleanUpWSA();

}

#endif // !WINDOWS_SOCKET_H
