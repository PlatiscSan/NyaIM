
#include "pch.h"
#include "MessageHandler.h"
#include "DBConnection.h"
#include "ServerConfig.h"

static std::hash<std::string> s_hash_string{};

NyaIM_AcceptLoginMessage NyaIMServer::core::HandleLoginMessage(NyaIM_LoginMessage* msg) {
    
    std::ostringstream sql_oss;
    sql_oss << "SELECT * FROM " << config::AuthenticationTable() << " WHERE uid = ? ";

    std::unique_ptr<db::DBConnection> db_connection = db::RequestConnection();

    NyaIM_AcceptLoginMessage ret_msg = {};
    ret_msg.base.msg_size = sizeof(NyaIM_AcceptLoginMessage);
    ret_msg.base.msg_type = NyaIM_MessageType::NYAIMMSG_ACCEPT_LOGIN;
    ret_msg.uid = NyaIMInvalidUID;

    try {
        db_connection->ExecuteQuery(
            sql_oss.str(),
            [&msg, &ret_msg](SQLHSTMT& stmt) {

                std::string uid = std::to_string(s_hash_string(msg->username));

                SQLLEN indicator_ptr = SQL_NTS;
                SQLBindParameter(
                    stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, 
                    SQL_VARCHAR, NyaIMMaxIntegerStringLength, 0,
                    uid.data(), uid.size(), &indicator_ptr
                );

                auto ret = SQLExecute(stmt);

                if (!SQL_SUCCEEDED(ret)) {
                    db::HandleDiagnosticRecord(stmt, SQL_HANDLE_STMT, ret);
                    throw std::runtime_error("SQL execute error");
                }

                std::string password(NyaIMMaxDataSize, '\0');

                SQLLEN uid_len = 0, password_len = 0;

                SQLBindCol(stmt, 1, SQL_C_CHAR, uid.data(), uid.capacity(), &uid_len);
                SQLBindCol(stmt, 2, SQL_C_CHAR, password.data(), password.capacity(), &password_len);


                while (SQLFetch(stmt) == SQL_SUCCESS) {
                    if (s_hash_string(msg->password) == std::stoull(password)) {
                        ret_msg.uid = std::stoull(uid);
                    }
                }

            }
        );
    }
    catch (std::exception const&) {
        db::ReturnConnection(db_connection);
        throw;
    }

    db::ReturnConnection(db_connection);


    return ret_msg;

}

NyaIM_AcceptRegisterMessage NyaIMServer::core::HandleRegisterMessage(NyaIM_RegisterMessage* msg) {

    NyaIM_AcceptRegisterMessage ret_msg = {};
    ret_msg.base.msg_size = sizeof(NyaIM_AcceptRegisterMessage);
    ret_msg.base.msg_type = NyaIM_MessageType::NYAIMMSG_ACCEPT_REGISTER;
    ret_msg.success = false;

    auto login_result = HandleLoginMessage(reinterpret_cast<NyaIM_LoginMessage*>(msg));

    if (login_result.uid != NyaIMInvalidUID) {
        return ret_msg;
    }

    std::ostringstream sql_oss;
    sql_oss << "INSERT INTO " << config::AuthenticationTable() << " (uid, password) VALUES (?, ?)";

    std::unique_ptr<db::DBConnection> db_connection = db::RequestConnection();

    try {
        db_connection->ExecuteQuery(
            sql_oss.str(),
            [&msg, &ret_msg](SQLHSTMT& stmt) {

                SQLLEN indicator_ptr = SQL_NTS;

                std::string uid = std::to_string(s_hash_string(msg->username));
                std::string password = std::to_string(s_hash_string(msg->password));
                
                SQLBindParameter(
                    stmt, 1, SQL_PARAM_INPUT, 
                    SQL_C_CHAR, SQL_VARCHAR,  NyaIMMaxIntegerStringLength,
                    0, const_cast<char*>(uid.c_str()), uid.size(),
                    &indicator_ptr
                );  
                SQLBindParameter(
                    stmt, 2,  SQL_PARAM_INPUT, 
                    SQL_C_CHAR, SQL_VARCHAR, NyaIMMaxIntegerStringLength,
                    0, const_cast<char*>(password.c_str()), password.size(),
                    &indicator_ptr
                ); 

                auto ret = SQLExecute(stmt);
                if (!SQL_SUCCEEDED(ret)) {
                    db::HandleDiagnosticRecord(stmt, SQL_HANDLE_STMT, ret);
                    throw std::runtime_error("SQL execute error");
                }

                ret_msg.success = true;

            }
        );
    }
    catch (std::exception const&) {
        db::ReturnConnection(db_connection);
        throw;
    }

    db::ReturnConnection(db_connection);

    return ret_msg;


}
