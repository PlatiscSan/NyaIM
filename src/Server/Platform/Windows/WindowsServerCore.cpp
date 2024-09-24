
#include "pch.h"
#include "WindowsServerCore.h"
#include "Server/Core/ServerConfig.h"
#include "Server/Core/DBConnection.h"
#include "Server/Core/MessageHandler.h"

using namespace common_dev;
using namespace common_dev::windows;
using namespace NyaIMServer::core;

NyaIMServer::core::WindowsServerCore::WindowsServerCore(int argc, char** argv) {

	InitializeEnvironment(argc, argv);
	InitializeServer();

}

NyaIMServer::core::WindowsServerCore::~WindowsServerCore() noexcept {

	log::Info("Stopping server...");
	CleanUp();
	

}

void NyaIMServer::core::WindowsServerCore::InitializeEnvironment(int argc, char** argv) {

	config::LoadConfig(argc, argv);
	log::Info("config file is loaded");

	std::size_t num_threads = std::thread::hardware_concurrency();

	db::InitializeDBConnectionPool(num_threads);
	log::Info("database connection is loaded");

}

void NyaIMServer::core::WindowsServerCore::InitializeServer() {

	iocp::IOCPServer::InitializeWinSock2();

	m_iocp_server = std::make_unique<iocp::IOCPServer>();

	m_iocp_server->SetLogInfoFunc(log::Info);
	m_iocp_server->SetLogErrorFunc(log::Error);
	m_iocp_server->SetLogFatalFunc(log::Fatal);

	m_iocp_server->SetReceiveCallback(std::bind(&WindowsServerCore::ReceiveData, this, std::placeholders::_1, std::placeholders::_2));
	m_iocp_server->SetSendCallback(std::bind(&WindowsServerCore::RespondClient, this, std::placeholders::_1, std::placeholders::_2));

	m_iocp_server->Start(config::ServerPort());

	log::Info("Server is now running, listening on " + std::to_string(m_iocp_server->GetListenPort()));

}

void NyaIMServer::core::WindowsServerCore::ReceiveData(iocp::SocketContext* socket_context, iocp::IOContext* io_context) {

	NyaIM_BaseMessage* msg = reinterpret_cast<NyaIM_BaseMessage*>(io_context->buffer.data());
	switch (msg->msg_type) {
	case NyaIM_MessageType::NYAIMMSG_LOGIN: {
		NyaIM_AcceptLoginMessage ret = HandleLoginMessage(reinterpret_cast<NyaIM_LoginMessage*>(msg));
		if (ret.uid != NyaIMInvalidUID) {
			std::lock_guard<std::mutex> lock(m_uid_socket_map_mutex);
			m_uid_socket_map.emplace(ret.uid, socket_context);
		}
		m_iocp_server->SendData(socket_context, &ret, sizeof(NyaIM_AcceptLoginMessage));
	}
		break;
	case NyaIM_MessageType::NYAIMMSG_REGISTER: {
		NyaIM_AcceptRegisterMessage ret = HandleRegisterMessage(reinterpret_cast<NyaIM_RegisterMessage*>(msg));
		m_iocp_server->SendData(socket_context, &ret, sizeof(NyaIM_AcceptRegisterMessage));
	}
		break;
	default:
		break;
	}

}

void NyaIMServer::core::WindowsServerCore::RespondClient(iocp::SocketContext* socket_context, iocp::IOContext* io_context) {


}

void NyaIMServer::core::WindowsServerCore::CleanUp() {

	iocp::IOCPServer::CleanUpWinSock();

}

void NyaIMServer::core::WindowsServerCore::ParseCommandLine(std::string const& cmd) {

	if (cmd == "stop") {
		m_is_quit = true;
	}

}

void NyaIMServer::core::WindowsServerCore::ServerLoop() {

	if (m_is_quit) {
		return;
	}

	std::string cmd;
	std::cout << "Press enter to activate console\n";


	int ret = std::getchar();
	while (!m_is_quit) {

		std::cout << ">>";
		std::getline(std::cin, cmd);

		ParseCommandLine(cmd);

	}

}
