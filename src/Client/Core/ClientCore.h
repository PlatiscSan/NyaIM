#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H

#include "Common/NyaIMMessage.h"

#include <functional>
#include <string>

namespace NyaIM::core {

	class ClientCore {

	public:

		ClientCore() = default;
		virtual ~ClientCore() noexcept = default;

		virtual bool Start(std::string const& host, std::uint16_t port) = 0;
		virtual void Login(std::string const& user, std::string const& password, std::function<void(NyaIM_AcceptLoginMessage* msg)> const& callback) = 0;
		virtual void Register(std::string const& user, std::string const& password, std::function<void(NyaIM_AcceptRegisterMessage* msg)> const& callback) = 0;

	protected:

		virtual void HandleMessage(void* buffer, std::size_t bytes_received) = 0;

	};


}

#endif // !CLIENT_CORE_H
