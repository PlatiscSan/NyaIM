
#include "pch.h"
#include "ServerConfig.h"
#include "yaml-cpp/yaml.h"

using namespace NyaIMServer::core::config;

static std::filesystem::path s_config_path = "config.yaml";
static std::string s_db_server = "NyaIM_DB";
static std::string s_db_login = "NyaIMAdmin";
static std::string s_db_authentication = "123456";
static std::uint16_t s_server_port = 23595u;
static std::string s_authentication_table = "Authentication";

static void ParseCommandLineArgs(int argc, char** argv) {

    for (std::size_t i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-c") == 0 || std::strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                s_config_path = argv[i + 1];
                break;
            }
        }
    }

}

static void CreateConfigFile() {

    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "DBServer";
    out << YAML::Value << s_db_server;

    out << YAML::Key << "DBLogin";
    out << YAML::Value << s_db_login;

    out << YAML::Key << "DBAuthentication";
    out << YAML::Value << s_db_authentication;

    out << YAML::Key << "port";
    out << YAML::Value << s_server_port;

    out << YAML::Key << "Authentication Table";
    out << YAML::Value << s_authentication_table;


    std::ofstream fout(s_config_path);
    if (!fout) {
        throw std::runtime_error("Failed to create config file");
    }
    fout << out.c_str();

    fout.close();

}

void NyaIMServer::core::config::LoadConfig(int argc, char** argv) {

    ParseCommandLineArgs(argc, argv);
    if (!std::filesystem::exists(s_config_path)) {
        CreateConfigFile();
        return;
    }

    YAML::Node config = YAML::LoadFile(s_config_path.string());

    if (config["DBServer"]) {
        s_db_server = config["DBServer"].as<std::string>();
    }

    if (config["DBLogin"]) {
        s_db_login = config["DBLogin"].as<std::string>();
    }

    if (config["DBAuthentication"]) {
        s_db_authentication = config["DBAuthentication"].as<std::string>();
    }

    if (config["port"]) {
        s_server_port = config["port"].as<std::uint16_t>();
    }

    if (config["Authentication Table"]) {
        s_authentication_table = config["Authentication Table"].as<std::string>();
    }

}

std::string const& NyaIMServer::core::config::DBServer() noexcept {
    return s_db_server;
}

std::string const& NyaIMServer::core::config::DBLogin() noexcept {
    return s_db_login;
}

std::string const& NyaIMServer::core::config::DBAuthentication() noexcept {
    return s_db_authentication;
}

std::uint16_t const& NyaIMServer::core::config::ServerPort() noexcept {
    return s_server_port;
}

std::string const& NyaIMServer::core::config::AuthenticationTable() noexcept {
    return s_authentication_table;
}
