
#include "pch.h"
#include "ServerCore.h"

#ifdef _WIN32
    #include "Server/Platform/Windows/WindowsServerCore.h"
#elif defined(__linux__)
    #include "Server/Platform/Linux/LinuxServerCore.h"
#endif // _WIN32

using namespace NyaIMServer::core;

#ifdef _WIN32

std::shared_ptr<ServerCore> NyaIMServer::core::CreateServer(int argc, char** argv) {
    return std::make_shared<WindowsServerCore>(argc, argv);
}

#elif defined(__linux__)

std::shared_ptr<ServerCore> NyaIMServer::core::CreateServer(int argc, char** argv) {
    return std::shared_ptr<LinuxServerCore>(argc, argv);
}

#endif // _WIN32




