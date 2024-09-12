#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

namespace NyaIMServer::core {

	class ConnectionHandler {
	public:

		virtual ~ConnectionHandler() {}
		virtual void HandleConnection(int client_socket) = 0;

	};

}

#endif // !CONNECTION_HANDLER_H
