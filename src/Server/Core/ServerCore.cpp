
#include "pch.h"
#include "ServerCore.h"

#ifdef _WIN32
    #include "Platform/Windows/WindowsServerCore.h"
#elif defined(__linux__)
    #include "Platform/Linux/LinuxServerCore.h"
#endif // _WIN32

using namespace NyaIMServer::core;

#ifdef _WIN32

std::shared_ptr<ServerCore> NyaIMServer::core::CreateServer(int argc, char** argv) {
    return std::make_shared<WindowsServerCore>(argc, argv);
}

#elif defined(__linux__)

std::shared_ptr<ServerCore> NyaIMServer::core::CreateServer() {
    return std::shared_ptr<LinuxServerCore>();
}

#endif // _WIN32




