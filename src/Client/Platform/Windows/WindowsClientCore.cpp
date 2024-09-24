
#include "pch.h"
#include "Client/Event/AuthenticationEvent.h"
#include "Client/Core/EventHandler.h"
#include "Common/NyaIMDef.h"
#include "WindowsClientCore.h"
#include "WindowsUtility.h"

using namespace common_dev;
using namespace NyaIM::utility::windows;
using namespace NyaIM::core;


NyaIM::core::WindowsClientCore::WindowsClientCore() {

	log::InitializeLog("Client");
	thread_pool::SetLogErrorFunc(log::Error);
	thread_pool::InitializeThreads();

}

bool NyaIM::core::WindowsClientCore::Start(std::string const& host, std::uint16_t port) {

	if (!m_quit) {
		return true;
	}

	m_quit = false;

	try {
		RegisterEventHandler();
		InitializeClientSocket(host, port);
		InitializeThreads();
	}
	catch (std::exception const& ex) {
		log::Fatal(ex.what());
		Stop();
		return false;
	}

	return true;
}

void NyaIM::core::WindowsClientCore::Login(std::string const& user, std::string const& password, std::function<void(NyaIM_AcceptLoginMessage* msg)> const& callback) {

	std::unique_lock<std::mutex> lock(m_pending_send_msgs_mutex);
	NyaIM_LoginMessage* login_msg = new NyaIM_LoginMessage;
	std::memset(login_msg, 0, sizeof(NyaIM_LoginMessage));
	login_msg->base.msg_size = sizeof(NyaIM_LoginMessage);
	login_msg->base.msg_type = NyaIM_MessageType::NYAIMMSG_LOGIN;

	strncpy_s(login_msg->username, user.size() + 1, user.c_str(), user.size());
	strncpy_s(login_msg->password, password.size() + 1, password.c_str(), password.size());

	m_pending_send_msgs.push(reinterpret_cast<NyaIM_BaseMessage*>(login_msg));
	event_system::Subscribe<AcceptLoginEvent>(
		[callback](AcceptLoginEvent& e) {
			NyaIM_AcceptLoginMessage msg = e.GetMessage();
			callback(&msg);
		},
		true
	);

	m_send_condition.notify_one();

}

void NyaIM::core::WindowsClientCore::Register(std::string const& user, std::string const& password, std::function<void(NyaIM_AcceptRegisterMessage* msg)> const& callback) {


	std::unique_lock<std::mutex> lock(m_pending_send_msgs_mutex);
	NyaIM_RegisterMessage* reg_msg = new NyaIM_RegisterMessage;
	std::memset(reg_msg, 0, sizeof(NyaIM_RegisterMessage));
	reg_msg->base.msg_size = sizeof(NyaIM_RegisterMessage);
	reg_msg->base.msg_type = NyaIM_MessageType::NYAIMMSG_REGISTER;

	strncpy_s(reg_msg->username, NyaIMMaxUserNameLength, user.c_str(), user.size());
	strncpy_s(reg_msg->password, NyaIMMaxUserPasswordLength, password.c_str(), password.size());

	std::memset(reg_msg->username + user.size(), 0, NyaIMMaxUserNameLength - user.size());
	std::memset(reg_msg->password + password.size(), 0, NyaIMMaxUserPasswordLength - password.size());

	m_pending_send_msgs.push(reinterpret_cast<NyaIM_BaseMessage*>(reg_msg));

	event_system::Subscribe<AcceptRegisterEvent>(
		[callback](AcceptRegisterEvent& e) {
			NyaIM_AcceptRegisterMessage msg = e.GetMessage();
			callback(&msg);
		},
		true
	);

	m_send_condition.notify_one();

}

void NyaIM::core::WindowsClientCore::InitializeClientSocket(std::string const& host, std::uint16_t port) {

	InitializeWSA();

	addrinfo* result = nullptr, hints = { 0 };
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	auto ret = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
	if (ret != 0) {
		throw std::runtime_error("getaddrinfo failed, error code: " + std::string(gai_strerror(ret)));
	}

	m_listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_listen_socket == INVALID_SOCKET) {
		freeaddrinfo(result);
		throw std::runtime_error("socket failed, error code: " + std::to_string(WSAGetLastError()));
	}

	u_long optval = 1;

	ret = ioctlsocket(m_listen_socket, FIONBIO, &optval);
	if (ret == SOCKET_ERROR) {
		throw std::runtime_error("ioctlsocket failed, error code: " + std::to_string(WSAGetLastError()));
	}

	ret = connect(m_listen_socket, result->ai_addr, static_cast<int>(result->ai_addrlen));
	if (ret == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK) {
			closesocket(m_listen_socket);
			m_listen_socket = INVALID_SOCKET;
			freeaddrinfo(result);
			throw std::runtime_error("connect failed, error code: " + std::to_string(error));
		}
	}

	freeaddrinfo(result);

}

void NyaIM::core::WindowsClientCore::InitializeThreads() {

	thread_pool::CommitTask(std::bind(&WindowsClientCore::ReceiveThreadProc, this));
	thread_pool::CommitTask(std::bind(&WindowsClientCore::SendManagerThreadProc, this));

}

void NyaIM::core::WindowsClientCore::RegisterEventHandler() {



}

void NyaIM::core::WindowsClientCore::Stop() {

	CleanUpWSA();
	m_quit = true;
	m_send_condition.notify_one();
	thread_pool::StopThreads();
	log::QuitLog();

}

void NyaIM::core::WindowsClientCore::ReceiveThreadProc() {

	thread_local fd_set read_fds = { 0 };
	thread_local timeval tv{
		.tv_sec = 5,
		.tv_usec = 0,
	};
	thread_local std::vector<char> buffer(NyaIMMaxDataSize, 0);

	while (!m_quit) {

		FD_ZERO(&read_fds);
		FD_SET(m_listen_socket, &read_fds);

		int select_ret = select(static_cast<int>(m_listen_socket + 1), &read_fds, nullptr, nullptr, &tv);
		if (select_ret == SOCKET_ERROR) {
			int error = WSAGetLastError();
			log::Fatal("select failed, error code: " + std::to_string(error));
			return;
		}

		if (select_ret > 0 && FD_ISSET(m_listen_socket, &read_fds)) {
			int recv_ret = recv(m_listen_socket, buffer.data(), static_cast<int>(buffer.size()), 0);
			if (recv_ret == SOCKET_ERROR) {
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK) {
					log::Fatal("recv failed, error code: " + std::to_string(error));
				}
			}
			else if (recv_ret == 0) {
				log::Info("Connection closed");
			}
			else {
				HandleMessage(buffer.data(), static_cast<std::size_t>(recv_ret));
			}
		}
	}

}

void NyaIM::core::WindowsClientCore::SendManagerThreadProc() {

	while (!m_quit) {

		std::unique_lock<std::mutex> lock(m_pending_send_msgs_mutex);
		m_send_condition.wait(lock, [this]() {return !m_pending_send_msgs.empty() || m_quit.load(); });

		if (m_quit) {
			return;
		}

		NyaIM_BaseMessage* pending_send_msg = std::move(m_pending_send_msgs.front());
		m_pending_send_msgs.pop();

		thread_pool::CommitTask(
			[this, &pending_send_msg]() mutable {

				int ret_send = send(m_listen_socket, reinterpret_cast<char*>(pending_send_msg), static_cast<int>(pending_send_msg->msg_size), 0);
				if (ret_send == SOCKET_ERROR) {
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK) {
						log::Fatal("select failed, error code: " + std::to_string(error));
					}
				}
				else if (ret_send == 0) {
					log::Info("Connection closed");
				}
				else {
					log::Info("bytes snet: " + std::to_string(ret_send));
				}

				delete pending_send_msg;
				pending_send_msg = nullptr;

			}
		);
	}

}

void NyaIM::core::WindowsClientCore::HandleMessage(void* buffer, std::size_t bytes_received) {

	NyaIM_BaseMessage* msg = reinterpret_cast<NyaIM_BaseMessage*>(buffer);
	switch (msg->msg_type) {
	case NyaIM_MessageType::NYAIMMSG_ACCEPT_LOGIN:
		thread_pool::CommitTask(
			[msg]() {
				event_system::Publish(AcceptLoginEvent(*reinterpret_cast<NyaIM_AcceptLoginMessage*>(msg))); 
			}
		);
		break;
	case NyaIM_MessageType::NYAIMMSG_ACCEPT_REGISTER:
		thread_pool::CommitTask(
			[msg]() {
				event_system::Publish(AcceptRegisterEvent(*reinterpret_cast<NyaIM_AcceptRegisterMessage*>(msg))); 
			}
		);
		break;
	}
	
}
