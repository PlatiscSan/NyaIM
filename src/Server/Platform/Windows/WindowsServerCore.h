#ifndef WINDOWS_SERVER_CORE_H
#define WINDOWS_SERVER_CORE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include "Utility/Windows/IOCP.h"

#include "Server/Core/ServerCore.h"

namespace NyaIMServer::core {

	class WindowsServerCore final : public ServerCore {

	public:

		WindowsServerCore(int argc, char** argv);
		~WindowsServerCore() noexcept;

		void ServerLoop() override;

	private:

		std::unordered_map<std::size_t, common_dev::windows::iocp::SocketContext*> m_uid_socket_map;
		std::mutex m_uid_socket_map_mutex;

		std::unique_ptr<common_dev::windows::iocp::IOCPServer> m_iocp_server;
		std::atomic_bool m_is_quit = false;	

		void InitializeEnvironment(int argc, char** argv);
		void InitializeServer();

		void ReceiveData(common_dev::windows::iocp::SocketContext* socket_context, common_dev::windows::iocp::IOContext* io_context);
		void RespondClient(common_dev::windows::iocp::SocketContext* socket_context, common_dev::windows::iocp::IOContext* io_context);

		void CleanUp();
		void ParseCommandLine(std::string const& cmd);


	};

}

#endif // !WINDOWS_SERVER_CORE_H
