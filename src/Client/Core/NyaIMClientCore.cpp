
#include "pch.h"
#include "Client/API/NyaIMClientCore.h"

#ifdef _WIN32

	#include "Client/Platform/Windows/WindowsClientCore.h"

NYAIM_API NyaIM_ClientCore* NYAIM_CALL NyaIM_CreateClient() {
	return reinterpret_cast<NyaIM_ClientCore*>(new NyaIM::core::WindowsClientCore());
}

NYAIM_API void NYAIM_CALL NyaIM_DestroyClient(NyaIM_ClientCore* client_core) {
	delete reinterpret_cast<NyaIM::core::WindowsClientCore*>(client_core);
}

#elif defined(__linux__)

	#include "Client/Platform/Windows/LinuxClientCore.h"

NYAIM_API NyaIM_ClientCore* NYAIM_CALL NyaIM_CreateClient() {
	return reinterpret_cast<NyaIM_ClientCore*>(new NyaIM::core::LinuxClientCore());
}

NYAIM_API void NYAIM_CALL NyaIM_DestroyClient(NyaIM_ClientCore* client_core) {
	delete reinterpret_cast<NyaIM::core::LinuxClientCore*>(client_core);
}

#endif // _WIN32


NYAIM_API bool NYAIM_CALL NyaIM_Start(NyaIM_ClientCore* client_core, char const* host, uint16_t port) {
	return reinterpret_cast<NyaIM::core::ClientCore*>(client_core)->Start(host, port);
}

NYAIM_API void NYAIM_CALL NyaIM_Login(NyaIM_ClientCore* client_core, char const* user, char const* password, NyaIM_LoginCallback callback) {
	reinterpret_cast<NyaIM::core::ClientCore*>(client_core)->Login(user, password, callback);
}

NYAIM_API void NYAIM_CALL NyaIM_Register(NyaIM_ClientCore* client_core, char const* user, char const* password, NyaIM_RegisterCallback callback) {
	reinterpret_cast<NyaIM::core::ClientCore*>(client_core)->Register(user, password, callback);
}
