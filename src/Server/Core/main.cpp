
#include "pch.h"

#include "ServerCore.h"

using namespace NyaIMServer::core;

int main(int argc, char** argv) {

	try {

		auto server = CreateServer(argc, argv);
		server->ServerLoop();

	}
	catch (std::exception const&) {

	}

	return 0;

}
