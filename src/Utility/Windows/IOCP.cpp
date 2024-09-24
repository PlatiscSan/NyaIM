
#include "IOCP.h"

using namespace common_dev::windows::iocp;

constexpr std::size_t MaxPostAccept = 10u;

std::function<void(std::string const&)> common_dev::windows::iocp::IOCPServer::Info;
std::function<void(std::string const&)> common_dev::windows::iocp::IOCPServer::Error;
std::function<void(std::string const&)> common_dev::windows::iocp::IOCPServer::Fatal;

void common_dev::windows::iocp::IOCPServer::InitializeWinSock2() {

    WSADATA wsa_data = { 0 };

    int error = 0;
    error = WSAStartup(MAKEWORD(2, 2), &wsa_data);
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

    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2) {
        WSACleanup();
        throw std::runtime_error("winsock 2.2 is unavailable");
    }


}

void common_dev::windows::iocp::IOCPServer::CleanUpWinSock() {

    WSACleanup();

}

void common_dev::windows::iocp::IOCPServer::Start(std::uint16_t port) {

    m_port = port;
    InitializeIOCP();
    InitializeListenSocket();

}

void common_dev::windows::iocp::IOCPServer::Stop() {

    m_quit = true;
    std::lock_guard<std::mutex> clients_lock(m_clients_mutex);
    m_clients.clear();

    if (m_io_completion_port) {
        for (std::size_t i = 0; i < m_worker_threads.size(); i++) {
            PostQueuedCompletionStatus(m_io_completion_port, 0, 0, nullptr);
        }   
        CloseHandle(m_io_completion_port);
    }

    for (auto& worker_thread : m_worker_threads) {
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }

    m_worker_threads.clear();

}

void common_dev::windows::iocp::IOCPServer::SendData(SocketContext* socket_context, void* data, std::size_t size) {

    std::unique_lock<std::mutex> lock(m_clients_mutex);

    auto iter = std::find_if(m_clients.begin(), m_clients.end(),
        [&socket_context](std::shared_ptr<SocketContext> const& context) {
            return context.get() == socket_context;
        }
    );

    if (iter == m_clients.end()) {
        throw std::runtime_error("Connection " + socket_context->GetAddressStr() + " : " + std::to_string(socket_context->GetPort()) + " is not registered");
    }

    lock.unlock();

    std::shared_ptr<IOContext> send_io_context;
    try {
        send_io_context = std::make_shared<IOContext>();
        socket_context->PushIOContext(send_io_context);
    }
    catch (std::exception const&) {
        throw std::runtime_error("Connection " + socket_context->GetAddressStr() + " : " + std::to_string(socket_context->GetPort()) + " send io context bad alloc");
    }

    send_io_context->accept_socket = socket_context->GetSocket();
    send_io_context->post_type = IOPostType::SEND;
    send_io_context->total_bytes = static_cast<DWORD>(size);
    send_io_context->wsabuf.len = static_cast<DWORD>(size);
    std::memcpy(send_io_context->buffer.data(), data, size);

    PostSendIO(*iter, send_io_context);

}

void common_dev::windows::iocp::IOCPServer::SendData(SocketContext* socket_context, IOContext* io_context) {

    io_context->post_type = IOPostType::SEND;
    io_context->total_bytes = 0;
    io_context->bytes_sent = 0;

    std::unique_lock<std::mutex> lock(m_clients_mutex);

    auto iter = std::find_if(m_clients.begin(), m_clients.end(),
        [&socket_context](std::shared_ptr<SocketContext> const& context) {
            return context.get() == socket_context;
        }
    );

    if (iter == m_clients.end()) {
        throw std::runtime_error("Connection " + socket_context->GetAddressStr() + " : " + std::to_string(socket_context->GetPort()) + " is not registered");
    }

    lock.unlock();

    PostSendIO(*iter, (*iter)->GetIOContext(io_context));

}

void common_dev::windows::iocp::IOCPServer::ReceiveClientData(SocketContext* socket_context, IOContext* io_context) {

    io_context->post_type = IOPostType::RECEIVE;
    io_context->total_bytes = 0;
    io_context->bytes_sent = 0;

    std::unique_lock<std::mutex> lock(m_clients_mutex);

    auto iter = std::find_if(m_clients.begin(), m_clients.end(),
        [&socket_context](std::shared_ptr<SocketContext> const& context) {
            return context.get() == socket_context;
        }
    );

    if (iter == m_clients.end()) {
        throw std::runtime_error("Connection " + socket_context->GetAddressStr() + " : " + std::to_string(socket_context->GetPort()) + " is not registered");
    }

    lock.unlock();

    PostSendIO(*iter, (*iter)->GetIOContext(io_context));

}

std::uint16_t common_dev::windows::iocp::IOCPServer::GetListenPort() const noexcept {
    return m_server_context->GetPort();
}

std::string common_dev::windows::iocp::IOCPServer::GetLastErrorMsg() {

    DWORD raw_error = GetLastError();
    if (raw_error == 0) {
        return "No error occurred";
    }

    TCHAR* msg_buff = nullptr;
    size_t size = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        raw_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (TCHAR*)&msg_buff,
        0,
        nullptr);

    if (size == 0) {
        DWORD format_error = GetLastError();
        throw std::runtime_error("Failed to format message. Error code: " + std::to_string(format_error));
    }

    std::string error_str(msg_buff, size);

    LocalFree(msg_buff);
    error_str[size - 1] = '\0';

    return error_str;

}

void common_dev::windows::iocp::IOCPServer::AcceptConnection(SocketContext* socket_context, IOContext* io_context) {

    thread_local sockaddr_in6* client_addr = nullptr, * local_addr = nullptr;
    thread_local int remote_length = 0, local_length = 0;
    DWORD const addr_len = sizeof(sockaddr_in6) + 16;

    this->GetAcceptExSockAddrs(
        io_context->wsabuf.buf, 0, addr_len, 
        addr_len, reinterpret_cast<sockaddr**>(&local_addr), 
        &local_length, reinterpret_cast<sockaddr**>(&client_addr), 
        &remote_length
    );


    std::shared_ptr<SocketContext> client_context = nullptr;

    try {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        client_context = std::make_shared<SocketContext>(io_context->accept_socket, *client_addr);
        m_clients.push_back(client_context);
    }
    catch (std::exception const&) {

        std::string ip_str{ 64 };
        std::uint16_t port = ntohs(client_addr->sin6_port);
        inet_ntop(AF_INET6, &client_addr->sin6_addr, ip_str.data(), ip_str.size());

        throw std::runtime_error("New connection from " + ip_str + " : " + std::to_string(port) + " failed");

    }

    PostAcceptIO(m_server_context->GetIOContext(io_context));
    AssociateWithIOCP(client_context);

    tcp_keepalive alive_in = { 0 }, alive_out = { 0 };
    alive_in.keepalivetime = 1000 * 60;
    alive_in.keepaliveinterval = 1000 * 10;
    alive_in.onoff = TRUE;

    DWORD lpcbBytesReturned = 0;
    auto ret = WSAIoctl(client_context->GetSocket(),
        SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in), &alive_out,
        sizeof(alive_out), &lpcbBytesReturned, nullptr, nullptr);
    if (ret == SOCKET_ERROR && Error) {
        Error("Connection " + client_context->GetAddressStr() + " : " + std::to_string(client_context->GetPort()) + "setting TCP Keep Alive failed, error code : " + std::to_string(WSAGetLastError()));
    }

    if (Info) {
        Info("New connection " + client_context->GetAddressStr() + " : " + std::to_string(client_context->GetPort()));
    }

    std::shared_ptr<IOContext> receive_io_context = nullptr;
    try {
        receive_io_context = std::make_shared<IOContext>();
        client_context->PushIOContext(receive_io_context);
    }
    catch (const std::exception&) {

        std::string ip_str{ 64 };
        std::uint16_t port = ntohs(client_addr->sin6_port);
        inet_ntop(AF_INET6, &client_addr->sin6_addr, ip_str.data(), ip_str.size());

        CloseConnection(client_context);
        throw std::runtime_error("Connection " + client_context->GetAddressStr() + " : " + std::to_string(client_context->GetPort()) + " receive io context bad alloc");

    }

    receive_io_context->accept_socket = client_context->GetSocket();
    receive_io_context->post_type = IOPostType::RECEIVE;

    PostReceiveIO(client_context, receive_io_context);


}

void common_dev::windows::iocp::IOCPServer::ReceiveData(SocketContext* socket_context, IOContext* io_context) {

    if (ReceiveCallback) {
        ReceiveCallback(socket_context, io_context);
    }

}

void common_dev::windows::iocp::IOCPServer::RespondClient(SocketContext* socket_context, IOContext* io_context) {

    if (io_context->bytes_sent < io_context->total_bytes) {

        io_context->wsabuf.buf = io_context->buffer.data() + io_context->bytes_sent;
        io_context->wsabuf.len = io_context->total_bytes - io_context->bytes_sent;

        std::unique_lock<std::mutex> lock(m_clients_mutex);

        auto iter = std::find_if(m_clients.begin(), m_clients.end(),
            [&socket_context](std::shared_ptr<SocketContext> const& context) {
                return context.get() == socket_context;
            }
        );

        if (iter == m_clients.end()) {
            throw std::runtime_error("Connection " + socket_context->GetAddressStr() + " : " + std::to_string(socket_context->GetPort()) + " is not registered");
        }

        lock.unlock();

        PostSendIO(*iter, (*iter)->GetIOContext(io_context));
        return;

    }

    if (SendCallback) {
        SendCallback(socket_context, io_context);
    }

}

void common_dev::windows::iocp::IOCPServer::HandleError(SocketContext* context) {

    int error = WSAGetLastError();

    switch (error) {
    case WAIT_TIMEOUT:
        if (IsClientDisconnected(context)) {
            CloseConnection(context);
        }
        else {
            if (Info) {
                Info("Connection " + context->GetAddressStr() + " : " + std::to_string(context->GetPort()) + "timeout, retrying...");
            }
        }
        break;
    case ERROR_NETNAME_DELETED:
        CloseConnection(context);
        break;
    default:
        throw std::runtime_error("Completion IO operation failed: " + GetLastErrorMsg());
    }

}

void common_dev::windows::iocp::IOCPServer::CloseConnection(SocketContext* context) {

    std::lock_guard<std::mutex> lock(m_clients_mutex);

    if (Info) {
        Info("Connection " + context->GetAddressStr() + " : " + std::to_string(context->GetPort()) + " closed");
    }

    m_clients.erase(std::remove_if(m_clients.begin(), m_clients.end(),
        [&context](std::shared_ptr<SocketContext> const& _context) {
            return _context.get() == context;
        }
    ));

}

bool common_dev::windows::iocp::IOCPServer::IsClientDisconnected(SocketContext* context) {

    thread_local std::array<char, 16> test_msg = { "Test Message\0" };
    thread_local int bytes_sent = send(context->GetSocket(), test_msg.data(), static_cast<int>(test_msg.size()), 0);
    return SOCKET_ERROR != bytes_sent;

}

void common_dev::windows::iocp::IOCPServer::InitializeIOCP() {

    m_io_completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (!m_io_completion_port) {
        Stop();
        throw std::runtime_error("CreateIoCompletionPort failed: " + GetLastErrorMsg());
    }

    for (std::size_t i = 0; i < std::thread::hardware_concurrency(); i++) {
        m_worker_threads.emplace_back(
            [this]() mutable {
                
                thread_local DWORD bytes_transferred = 0;
                thread_local OVERLAPPED* overlapped_ptr = nullptr;
                thread_local SocketContext* socket_context_ptr = nullptr;
                thread_local IOContext* io_context_ptr = nullptr;
                thread_local BOOL queue_ret = FALSE; 

                while (!m_quit) {

                    bytes_transferred = 0;
                    overlapped_ptr = nullptr;
                    socket_context_ptr = nullptr;

                    queue_ret = GetQueuedCompletionStatus(
                        m_io_completion_port,
                        &bytes_transferred,
                        reinterpret_cast<PULONG_PTR>(&socket_context_ptr),
                        &overlapped_ptr,
                        INFINITE
                    );

                    if (m_quit) {
                        return;
                    }

                    io_context_ptr = CONTAINING_RECORD(overlapped_ptr, IOContext, overlapped);

                    if (!queue_ret) {

                        try {
                            HandleError(socket_context_ptr);
                        }
                        catch (std::exception const& ex) {
                            if (Fatal) {
                                Fatal(ex.what());
                            }
                            return;
                        }

                        continue;

                    }

                    if (!bytes_transferred && 
                        (io_context_ptr->post_type == IOPostType::RECEIVE ||
                            io_context_ptr->post_type == IOPostType::SEND)
                        ) {
                        CloseConnection(socket_context_ptr);
                        continue;
                    }

                    switch (io_context_ptr->post_type) {
                    case IOPostType::ACCEPT:
                        io_context_ptr->total_bytes = bytes_transferred;
                        try {
                            AcceptConnection(socket_context_ptr, io_context_ptr);
                        }
                        catch (std::exception const& ex) {
                            if (Error) {
                                Error(ex.what());
                            }
                        }
                        break;
                    case IOPostType::SEND:
                        io_context_ptr->total_bytes = bytes_transferred;
                        try {
                            RespondClient(socket_context_ptr, io_context_ptr);
                        }
                        catch (std::exception const& ex) {
                            if (Error) {
                                Error(ex.what());
                            }
                        }
                        break;
                    case IOPostType::RECEIVE:
                        io_context_ptr->total_bytes = bytes_transferred;
                        try {
                            ReceiveData(socket_context_ptr, io_context_ptr);
                        }
                        catch (std::exception const& ex) {
                            if (Error) {
                                Error(ex.what());
                            }
                        }
                        break;
                    default:
                        if (Error) {
                            thread_local std::ostringstream error_info;
                            error_info << "Completion IO operation thread" << std::this_thread::get_id() << " was passed an invalid argument";
                            Error(error_info.str());
                        }
                        break;
                    }

                }

            }
        );
    }

}

void common_dev::windows::iocp::IOCPServer::InitializeListenSocket() {

    sockaddr_in6 ipv_addr = { 0 };

    ipv_addr.sin6_family = AF_INET6;
    ipv_addr.sin6_addr = in6addr_any;
    ipv_addr.sin6_port = htons(m_port);

    SOCKET listen_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        closesocket(listen_socket);
        Stop();
        throw std::runtime_error("socket failed, error code: " + std::to_string(WSAGetLastError()));
    }

    int optval = 0;
    auto error = setsockopt(listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char const*>(&optval), sizeof(optval));
    if (error == SOCKET_ERROR) {
        closesocket(listen_socket);
        Stop();
        throw std::runtime_error("setsockopt failed, error code: " + std::to_string(WSAGetLastError()));
    }

    try {
        m_server_context = std::make_unique<SocketContext>(listen_socket, ipv_addr);
    }
    catch (std::exception const&) {
        Stop();
        throw;
    }

    error = bind(listen_socket, (struct sockaddr*)&ipv_addr, sizeof(ipv_addr));
    if (error == SOCKET_ERROR) {
        closesocket(listen_socket);
        Stop();
        throw std::runtime_error("bind failed, error code: " + std::to_string(WSAGetLastError()));
    }


    error = listen(listen_socket, SOMAXCONN);
    if (error == SOCKET_ERROR) {
        closesocket(listen_socket);
        Stop();
        throw std::runtime_error("listen failed, error code: " + std::to_string(WSAGetLastError()));
    }


    DWORD dwBytes = 0;
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
    if (SOCKET_ERROR == WSAIoctl(listen_socket,
        SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx,
        sizeof(GuidAcceptEx), &this->AcceptEx,
        sizeof(this->AcceptEx), &dwBytes, nullptr, nullptr)) {
        closesocket(listen_socket);
        Stop();
        throw std::runtime_error("Failed to get the address of AcceptEx, error code: " + std::to_string(WSAGetLastError()));
    }

    if (SOCKET_ERROR == WSAIoctl(listen_socket,
        SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs,
        sizeof(GuidGetAcceptExSockAddrs), &this->GetAcceptExSockAddrs,
        sizeof(this->GetAcceptExSockAddrs), &dwBytes, NULL, NULL)) {
        closesocket(listen_socket);
        Stop();
        throw std::runtime_error("Failed to get the address of GetAcceptExSockAddrs, error code: " + std::to_string(WSAGetLastError()));
    }

    HANDLE ret_handle = CreateIoCompletionPort(
        reinterpret_cast<HANDLE>(listen_socket),
        m_io_completion_port,
        reinterpret_cast<ULONG_PTR>(m_server_context.get()),
        0
    );

    if (!ret_handle) {
        closesocket(listen_socket);
        Stop();
        throw std::runtime_error("Bind socket to completion IO failed, error msg: " + GetLastErrorMsg());
    }

    for (std::size_t i = 0; i < MaxPostAccept; i++) {
        try {
            std::shared_ptr<IOContext> io_context = std::make_shared<IOContext>();
            io_context->post_type = IOPostType::ACCEPT;
            PostAcceptIO(io_context);
            m_server_context->PushIOContext(io_context);
        }
        catch (std::exception const&) {
            closesocket(listen_socket);
            Stop();
            throw;
        }
    }

}

void common_dev::windows::iocp::IOCPServer::PostAcceptIO(std::shared_ptr<IOContext> const& context) {

    context->accept_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (context->accept_socket == INVALID_SOCKET) {
        throw std::runtime_error("PostAcceptIO socket failed, error code: " + std::to_string(WSAGetLastError()));
    }

    int optval = 0;
    auto error = setsockopt(context->accept_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char const*>(&optval), sizeof(optval));
    if (error == SOCKET_ERROR) {
        closesocket(context->accept_socket);
        Stop();
        throw std::runtime_error("PostAcceptIO setsockopt failed, error code: " + std::to_string(WSAGetLastError()));
    }

    DWORD dwBytes = 0;
    DWORD const dwAddrLen = (sizeof(sockaddr_in6) + 16);
    auto ret = this->AcceptEx(
        m_server_context->GetSocket(),
        context->accept_socket,
        context->wsabuf.buf,
        0, dwAddrLen,
        dwAddrLen, &dwBytes,
        &context->overlapped
    );

    if (!ret) {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING) {
            throw std::runtime_error("PostAcceptIO failed, error code: " + std::to_string(error));
        }
    }

    m_accept_post_count++;

}


void common_dev::windows::iocp::IOCPServer::PostReceiveIO(std::shared_ptr<SocketContext> const& socket_context, std::shared_ptr<IOContext> const& io_context) {

    DWORD flags = 0, bytes_received = 0;
    int ret = WSARecv(
        io_context->accept_socket,
        &io_context->wsabuf, 1,
        &bytes_received, &flags,
        &io_context->overlapped, 
        nullptr
    );

    if (ret == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING) {
            CloseConnection(socket_context);
            throw std::runtime_error("Connection " + socket_context->GetAddressStr() + " : " + std::to_string(socket_context->GetPort()) + "PostReceiveIO failed, error code:" + std::to_string(error));
        }
    }

}

void common_dev::windows::iocp::IOCPServer::PostSendIO(std::shared_ptr<SocketContext> const& socket_context, std::shared_ptr<IOContext> const& io_context) {

    DWORD const flags = 0;
    DWORD bytes_sent = 0;
    int ret = WSASend(
        io_context->accept_socket,
        &io_context->wsabuf, 1, &bytes_sent, flags,
        &io_context->overlapped, nullptr
    );

    if (ret == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING) {
            CloseConnection(socket_context);
            throw std::runtime_error("Connection " + socket_context->GetAddressStr() + " : " + std::to_string(socket_context->GetPort()) + "PostSendIO failed, error code:" + std::to_string(error));
        }
    }

}

void common_dev::windows::iocp::IOCPServer::CloseConnection(std::shared_ptr<SocketContext> const& context) {

    std::lock_guard<std::mutex> lock(m_clients_mutex);

    if (Info) {
        Info("Connection " + context->GetAddressStr() + " : " + std::to_string(context->GetPort()) + " closed");
    }

    m_clients.erase(std::remove_if(m_clients.begin(), m_clients.end(),
        [&context](std::shared_ptr<SocketContext> const& _context) {
            return _context == context;
        }
    ));

}

void common_dev::windows::iocp::IOCPServer::AssociateWithIOCP(std::shared_ptr<SocketContext> const& context) {

    HANDLE ret_handle = CreateIoCompletionPort(
        reinterpret_cast<HANDLE>(context->GetSocket()),
        m_io_completion_port, 
        reinterpret_cast<ULONG_PTR>(context.get()), 
        0
    );

    if (!ret_handle) {
        throw std::runtime_error("Connection " + context->GetAddressStr() + " : " + std::to_string(context->GetPort()) + "failed to bind IOCP: " + GetLastErrorMsg());
    }

}

common_dev::windows::iocp::SocketContext::~SocketContext() noexcept {

    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

}

std::shared_ptr<IOContext> common_dev::windows::iocp::SocketContext::GetIOContext(IOContext* context) {

    auto it = std::find_if(m_contexts.begin(), m_contexts.end(),
        [&context](std::shared_ptr<IOContext> const& _context) {
            return _context.get() == context;
        }
    );

    if (it == m_contexts.end()) {
        return nullptr;
    }

    return *it;

}

void common_dev::windows::iocp::SocketContext::PushIOContext(std::shared_ptr<IOContext> const& context) {

    m_contexts.push_back(context);

}

void common_dev::windows::iocp::SocketContext::RemoveIOContext(std::shared_ptr<IOContext> const& context) {

    m_contexts.erase(std::remove_if(m_contexts.begin(), m_contexts.end(),
        [&context](std::shared_ptr<IOContext> const& _context) {
            return _context == context;
        }
    ));

}

void common_dev::windows::iocp::SocketContext::RemoveIOContext(IOContext* context) {

    m_contexts.erase(std::remove_if(m_contexts.begin(), m_contexts.end(),
        [&context](std::shared_ptr<IOContext> const& _context) {
            return _context.get() == context;
        }
    ));

}

std::string common_dev::windows::iocp::SocketContext::GetAddressStr() const noexcept 
{
    thread_local std::array<char, 64> ip_str;
    inet_ntop(AF_INET6, &m_addr.sin6_addr, ip_str.data(), ip_str.size());

    return ip_str.data();

}

std::uint16_t common_dev::windows::iocp::SocketContext::GetPort() const noexcept {
    return ntohs(m_addr.sin6_port);
}
