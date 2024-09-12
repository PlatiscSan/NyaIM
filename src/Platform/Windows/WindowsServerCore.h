#ifndef WINDOWS_SERVER_CORE_H
#define WINDOWS_SERVER_CORE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>


#include "Server/Core/ServerCore.h"

namespace NyaIMServer::core {

	class WindowsServerCore final : public ServerCore {

	public:

		WindowsServerCore(int argc, char** argv);
		~WindowsServerCore() noexcept;

		void ServerLoop() override;

	private:

		std::uint32_t m_server_port;
		SOCKET m_listen_socket = INVALID_SOCKET;

		std::mutex m_clients_mutex;
		std::list<SOCKET> m_clients;

		std::thread m_listen_thread;
		std::atomic_bool m_is_stop = false;

		void InitializeServer(int argc, char** argv) override;
		void InitializeServerSocket() override;

		void HandleConnection() override;


	};

}

#endif // !WINDOWS_SERVER_CORE_H
