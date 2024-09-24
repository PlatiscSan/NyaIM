#ifndef AUTHENTICATION_EVENT_H
#define AUTHENTICATION_EVENT_H

#include "Utility/EventBus.h"
#include "Common/NyaIMMessage.h"

#ifdef GetMessage
	#undef GetMessage
#endif // GetMessage


namespace NyaIM::core {

	class AcceptLoginEvent final : public common_dev::event_system::BaseEvent {

	public:

		AcceptLoginEvent(NyaIM_AcceptLoginMessage const& msg) 
			:m_msg(msg) {}

		NyaIM_AcceptLoginMessage const& GetMessage() const noexcept { return m_msg; }

	private:

		NyaIM_AcceptLoginMessage m_msg;

	};

	class AcceptRegisterEvent final : public common_dev::event_system::BaseEvent {

	public:

		AcceptRegisterEvent(NyaIM_AcceptRegisterMessage const& msg)
			:m_msg(msg) {}

		NyaIM_AcceptRegisterMessage const& GetMessage() const noexcept { return m_msg; }

	private:

		NyaIM_AcceptRegisterMessage m_msg;

	};


}

#endif // !AUTHENTICATION_EVENT_H
