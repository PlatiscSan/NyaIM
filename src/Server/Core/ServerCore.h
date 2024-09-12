#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include <functional>
#include <memory>
#include <thread>

#include "ConnectionHandler.h"
#include "ServerError.h"

namespace NyaIMServer::core {

	class ServerCore {

	public:

		using ErrorCallback = std::function<void()>;

		ServerCore() = default;
		virtual ~ServerCore() noexcept = default;

		virtual void ServerLoop() = 0;

	protected:

		virtual void InitializeServer(int argc, char** argv) = 0;
		virtual void InitializeServerSocket() = 0;

		virtual void HandleConnection() = 0;


	};

	std::shared_ptr<ServerCore> CreateServer(int argc, char** argv);

}

#endif // !SERVER_CORE_H
