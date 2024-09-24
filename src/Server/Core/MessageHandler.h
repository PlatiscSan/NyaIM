#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "Common/NyaIMMessage.h"

namespace NyaIMServer::core {

	NyaIM_AcceptLoginMessage HandleLoginMessage(NyaIM_LoginMessage* msg);
	NyaIM_AcceptRegisterMessage HandleRegisterMessage(NyaIM_RegisterMessage* msg);

}

#endif // !MESSAGE_HANDLER_H
