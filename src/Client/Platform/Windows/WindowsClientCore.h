
#ifndef WINDOWS_CLIENT_CORE_H
#define WINDOWS_CLIENT_CORE_H

#include "Client/Core/ClientCore.h"

#include <WinSock2.h>

#include <atomic>
#include <thread>

namespace NyaIM::core {

	class WindowsClientCore final : public ClientCore {

	public:

		WindowsClientCore();
		~WindowsClientCore() noexcept {
			Stop();
		}

		bool Start(std::string const& host, std::uint16_t port) override;
		void Login(std::string const& user, std::string const& password, std::function<void(NyaIM_AcceptLoginMessage* msg)> const& callback) override;
		void Register(std::string const& user, std::string const& password, std::function<void(NyaIM_AcceptRegisterMessage* msg)> const& callback) override;

	private:


		void InitializeClientSocket(std::string const& host, std::uint16_t port);
		void InitializeThreads();
		void RegisterEventHandler();

		void Stop();

		void ReceiveThreadProc();
		void SendManagerThreadProc();

		void HandleMessage(void* buffer, std::size_t bytes_received) override;

		SOCKET m_listen_socket = INVALID_SOCKET;

		std::mutex m_pending_send_msgs_mutex;
		std::queue<NyaIM_BaseMessage*> m_pending_send_msgs;
		std::condition_variable m_send_condition;

		std::atomic_bool m_quit = true;
		
		

	};

}

#endif // !WINDOWS_CLIENT_CORE_H
