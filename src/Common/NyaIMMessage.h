#ifndef NYAIM_MESSAGE_H
#define NYAIM_MESSAGE_H

#ifdef __cplusplus

#include <cstddef>
#include <cstdint>

#define NyaIMInvalidUID std::size_t(~0)

#else

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define NyaIMInvalidUID size_t(~0)

#endif // __cplusplus

#include "NyaIMDef.h"

#define NyaIMMaxUserNameLength 16u
#define	NyaIMMaxUserPasswordLength 32u

typedef enum NyaIM_MessageType : uint64_t {

	NYAIMMSG_UNKNOWN,
	NYAIMMSG_LOGIN,
	NYAIMMSG_ACCEPT_LOGIN,
	NYAIMMSG_REGISTER,
	NYAIMMSG_ACCEPT_REGISTER,
	NYAIMMSG_LOGOUT,

}NyaIM_MessageType;

typedef struct NyaIM_BaseMessage {

	NyaIM_MessageType msg_type;
	std::size_t msg_size;

}NyaIM_BaseMessage;

typedef struct NyaIM_LoginMessage {

	NyaIM_BaseMessage base;
	char username[NyaIMMaxUserNameLength];
	char password[NyaIMMaxUserPasswordLength];

}NyaIM_LoginMessage;

typedef struct NyaIM_AcceptLoginMessage {

	NyaIM_BaseMessage base;
	std::size_t uid;

}NyaIM_AcceptLoginMessage;

typedef struct NyaIM_RegisterMessage {

	NyaIM_BaseMessage base;
	char username[NyaIMMaxUserNameLength];
	char password[NyaIMMaxUserPasswordLength];

}NyaIM_RegisterMessage;

typedef struct NyaIM_AcceptRegisterMessage {

	NyaIM_BaseMessage base;
	bool success;

}NyaIM_AcceptRegisterMessage;

#endif // !NYAIM_MESSAGE_H
