#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include <functional>
#include <memory>
#include <thread>

namespace NyaIMServer::core {

	class ServerCore {

	public:

		ServerCore() = default;

		virtual ~ServerCore() noexcept = default;

		virtual void ServerLoop() = 0;

	protected:

	};

	std::shared_ptr<ServerCore> CreateServer(int argc, char** argv);

}

#endif // !SERVER_CORE_H
