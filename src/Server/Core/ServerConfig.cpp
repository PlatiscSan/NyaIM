
#include "pch.h"
#include "ServerConfig.h"
#include "yaml-cpp/yaml.h"

using namespace NyaIMServer::core::config;

constexpr auto DefaultConfigFileName = "config.yaml";
constexpr auto DefaultDBServerName = "NyaIM_DB";
constexpr auto DefaultDBUserName = "sa";
constexpr auto DefaultDBAuthentication = "123456";
constexpr auto DefaultListenPort = "23595";

static std::filesystem::path ParseCommandLineArgs(int argc, char** argv) {

    std::filesystem::path config_file = DefaultConfigFileName;

    for (std::size_t i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-c") == 0 && std::strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                config_file = argv[i + 1];
                break;
            }
        }
    }

    return config_file;

}

static ServerConfig LoadDefault() {

    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "DBServerName";
    out << YAML::Value << DefaultDBServerName;
    out << YAML::Key << "DBUserName";
    out << YAML::Value << DefaultDBUserName;
    out << YAML::Key << "DBAuthentication";
    out << YAML::Value << DefaultDBAuthentication;
    out << YAML::Key << "port";
    out << YAML::Value << DefaultListenPort;

    std::ofstream fout(DefaultConfigFileName);
    if (!fout) {
        throw std::runtime_error("Failed to create config file");
    }
    fout << out.c_str();

    fout.close();
    
    ServerConfig server_config = {
        .db_server_name = DefaultDBServerName,
        .db_user_name = DefaultDBUserName,
        .db_authentication = DefaultDBAuthentication,
        .server_port = DefaultListenPort,
    };


    return server_config;

}

ServerConfig NyaIMServer::core::config::LoadConfig(int argc, char** argv) {

    std::filesystem::path config_file = ParseCommandLineArgs(argc, argv);
    if (!std::filesystem::exists(config_file)) {
        return LoadDefault();
    }

    ServerConfig server_config;
    YAML::Node config = YAML::LoadFile(config_file.string());
    if (config["DBServerName"] && config["DBUserName"] && config["DBAuthentication"] &&config["port"]) {
        server_config.db_server_name = config["DBServerName"].as<std::string>();
        server_config.db_user_name = config["DBUserName"].as<std::string>();
        server_config.db_authentication = config["DBAuthentication"].as<std::string>();
        server_config.server_port = config["port"].as<std::string>();
    }
    else {
        throw std::runtime_error("Invalid config file");
    }

    return server_config;

}
