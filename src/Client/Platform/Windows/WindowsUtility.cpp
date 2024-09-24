
#include "pch.h"
#include "WindowsUtility.h"

static WSADATA s_wsa_data;
static std::atomic_bool s_is_initialized = false;

void NyaIM::utility::windows::InitializeWSA() {

    int error = 0;
    error = WSAStartup(MAKEWORD(2, 2), &s_wsa_data);
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

    if (LOBYTE(s_wsa_data.wVersion) != 2 || HIBYTE(s_wsa_data.wVersion) != 2) {
        WSACleanup();
        throw std::runtime_error("winsock 2.2 is unavailable");
    }

    s_is_initialized = true;

}

void NyaIM::utility::windows::CleanUpWSA() {

    WSACleanup();
    s_is_initialized = false;

}

std::string NyaIM::utility::windows::GetLastErrorMessageFromWinAPI() {

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
    error_str[size - 1] = '\0';

    LocalFree(msg_buff);

    return error_str;
}
