#ifndef MYSQL_CONNECTOR_H
#define MYSQL_CONNECTOR_H

#include <cstring>
#include <vector>
#include "mysql/mysql.h"
#include <stdio.h>
#include <string>
#include <memory>
#include "noncopyable.h"

class mysql_deleter{
public:
    void operator()(MYSQL *conn){
        mysql_close(conn);
    }
};

class mysql_res_deleter{
public:
    void operator()(MYSQL_RES *res){
        mysql_free_result(res);
    }
};


class mysql_connector : public noncopyable{
private:
    using pMYSQL = std::unique_ptr<MYSQL, mysql_deleter>;
    using pMYSQL_RES = std::unique_ptr<MYSQL_RES, mysql_res_deleter>;
    using string = std::string;

    pMYSQL conn{nullptr};
    string server;
    string user;
    string password;
    string database;
    unsigned port;
    bool is_conn{false};
public:
    mysql_connector(const string &server_, const string &user_, const string &password_, const string &database_, unsigned port_ = 0) :
        server(server_), user(user_), password(password_), database(database_), port(port_)
        {}
    mysql_connector() = default;
//    mysql_connector(const mysql_connector&) = delete;
//    mysql_connector &operator=(const mysql_connector&) = delete;
    mysql_connector(mysql_connector &&) = default;
    mysql_connector &operator=(mysql_connector &&) = default;
    ~mysql_connector() = default;


    int connect_to();
    int connect_to(const string &server_, const string &user_, const string &password_, const string &database_, unsigned port_ = 0);

    std::pair<std::vector< std::vector<std::string> >, int> query(const std::string &q);

    void close(){
        conn.release();
        is_conn = false;
    }

    const string error(){
        return mysql_error(conn.get());
    }

    bool is_connected(){return is_conn;}

};

#endif //MYSQL_CONNECTOR_H