#ifndef IOCP_H
#define IOCP_H

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <MSWSock.h>
#include <mstcpip.h>

#include <iostream>
#include <sstream>

#include <array>
#include <string>
#include <list>
#include <vector>

#include <functional>

#include <thread>
#include <mutex>

namespace common_dev::windows::iocp {

	constexpr std::size_t MaxBufferLength = 1024ui64 * 8;

	enum class IOPostType : std::uint64_t {
		UNKNOWN,
		ACCEPT,
		SEND,
		RECEIVE,
	};

	struct IOContext final {

		OVERLAPPED overlapped = { 0 };
		SOCKET accept_socket = INVALID_SOCKET;
		IOPostType post_type = IOPostType::UNKNOWN;
		WSABUF wsabuf;
		std::vector<char> buffer;
		DWORD total_bytes = 0;
		DWORD bytes_sent = 0;

		IOContext()
			:buffer(MaxBufferLength, 0){
			wsabuf.buf = buffer.data();
			wsabuf.len = static_cast<ULONG>(buffer.size());
		}


	};

	class SocketContext final {

	public:

		SOCKET const& GetSocket() const noexcept { return m_socket; }
		void SetSocket(SOCKET socket) noexcept { m_socket = socket; }

		sockaddr_in6 const& GetSockAddr() const noexcept { return m_addr; }

		SocketContext() = default;	
		SocketContext(SOCKET socket)
			:m_socket(socket) {}
		SocketContext(SOCKET socket, sockaddr_in6 const& addr)
			:m_socket(socket), m_addr(addr) {}


		~SocketContext() noexcept;

		std::shared_ptr<IOContext> GetIOContext(IOContext* context);

		void PushIOContext(std::shared_ptr<IOContext> const& context);

		void RemoveIOContext(std::shared_ptr<IOContext> const& context);

		void RemoveIOContext(IOContext* context);

		std::string GetAddressStr() const noexcept;

		std::uint16_t GetPort() const noexcept;

	private:

		SOCKET m_socket = INVALID_SOCKET;
		sockaddr_in6 m_addr = { 0 };

		std::list<std::shared_ptr<IOContext>> m_contexts;


	};


	class IOCPServer final {
	
	public:

		static void InitializeWinSock2();
		static void CleanUpWinSock();

		IOCPServer() = default;
		~IOCPServer() noexcept {
			Stop();
		}

		void Start(std::uint16_t port);
		void Stop();

		void SendData(SocketContext* socket_context, void* data, std::size_t size);
		void SendData(SocketContext* socket_context, IOContext* io_context);

		void ReceiveClientData(SocketContext* socket_context, IOContext* io_context);

		std::uint16_t GetListenPort() const noexcept;

		static void SetLogInfoFunc(std::function<void(std::string const&)> const& func) {
			Info = func;
		}
		static void SetLogErrorFunc(std::function<void(std::string const&)> const& func) {
			Error = func;
		}
		static void SetLogFatalFunc(std::function<void(std::string const&)> const& func) {
			Fatal = func;
		}

		void SetReceiveCallback(std::function<void(SocketContext* socket_context, IOContext* io_context)> const& func) {
			ReceiveCallback = func;
		}

		void SetSendCallback(std::function<void(SocketContext* socket_context, IOContext* io_context)> const& func) {
			SendCallback = func;
		}

	private:

		static std::function<void(std::string const&)> Info;
		static std::function<void(std::string const&)> Error;
		static std::function<void(std::string const&)> Fatal;

		std::function<void(SocketContext* socket_context, IOContext* io_context)> ReceiveCallback;
		std::function<void(SocketContext* socket_context, IOContext* io_context)> SendCallback;

		std::unique_ptr<SocketContext> m_server_context;

		std::mutex m_clients_mutex;
		std::list<std::shared_ptr<SocketContext>> m_clients;

		LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockAddrs;
		LPFN_ACCEPTEX AcceptEx;

		HANDLE m_io_completion_port = INVALID_HANDLE_VALUE;
		std::list<std::thread> m_worker_threads;

		std::atomic_size_t m_accept_post_count;
		std::atomic_size_t m_connection_count;
		std::atomic_size_t m_error_count;

		std::uint16_t m_port;
		std::atomic_bool m_quit;

		static std::string GetLastErrorMsg();

		void AcceptConnection(SocketContext* socket_context, IOContext* io_context);
		void ReceiveData(SocketContext* socket_context, IOContext* io_context);
		void RespondClient(SocketContext* socket_context, IOContext* io_context);
		void HandleError(SocketContext* context);
		void CloseConnection(SocketContext* context);
		bool IsClientDisconnected(SocketContext* context);

		void InitializeIOCP();
		void InitializeListenSocket();
		
		void PostAcceptIO(std::shared_ptr<IOContext> const& context);
		void PostReceiveIO(std::shared_ptr<SocketContext> const& socket_context, std::shared_ptr<IOContext> const& io_context);
		void PostSendIO(std::shared_ptr<SocketContext> const& socket_context, std::shared_ptr<IOContext> const& io_context);
		void CloseConnection(std::shared_ptr<SocketContext> const& context);
		void AssociateWithIOCP(std::shared_ptr<SocketContext> const& context);

	};


}

#endif // !IOCP_H
