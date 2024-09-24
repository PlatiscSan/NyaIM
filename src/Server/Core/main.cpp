
#include "pch.h"
#include "Server/Core/ServerCore.h"

using namespace common_dev;
using namespace NyaIMServer::core;

int main(int argc, char** argv) {

	try {

		log::InitializeLog("Server");
		auto server = CreateServer(argc, argv);
		server->ServerLoop();

	}
	catch (std::exception const& ex) {

		if (log::IsLogRunning()) {
			log::Fatal(ex.what());
		}
		else {
			std::cerr << "Fatal Error: " << ex.what() << std::endl;
		}

		log::QuitLog();
		event_system::ClearSubscriptions();
		return EXIT_FAILURE;

	}

	log::QuitLog();
	event_system::ClearSubscriptions();
	return EXIT_SUCCESS;

}


