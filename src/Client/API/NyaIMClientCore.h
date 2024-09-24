#ifndef NYAIM_CLIENT_CORE_H
#define NYAIM_CLIENT_CORE_H

#include "APIControl.h"
#include "Common/NyaIMMessage.h"

#ifdef __cplusplus

#include <cstdarg>
#include <cstdint>

#else

#include <stdarg.h>
#include <stdint.h>

#endif // __cplusplus

typedef struct NyaIM_ClientCore NyaIM_ClientCore;
typedef void (NYAIM_CALL* NyaIM_LoginCallback)(NyaIM_AcceptLoginMessage*);
typedef void (NYAIM_CALL* NyaIM_RegisterCallback)(NyaIM_AcceptRegisterMessage*);

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	NYAIM_API NyaIM_ClientCore* NYAIM_CALL NyaIM_CreateClient();
	NYAIM_API void NYAIM_CALL NyaIM_DestroyClient(NyaIM_ClientCore * client_core);

	NYAIM_API bool NYAIM_CALL NyaIM_Start(NyaIM_ClientCore * client_core, char const* host, uint16_t port);
	NYAIM_API void NYAIM_CALL NyaIM_Login(NyaIM_ClientCore * client_core, char const* user, char const* password, NyaIM_LoginCallback callback);
	NYAIM_API void NYAIM_CALL NyaIM_Register(NyaIM_ClientCore * client_core, char const* user, char const* password, NyaIM_RegisterCallback callback);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !NYAIM_CLIENT_CORE_H
