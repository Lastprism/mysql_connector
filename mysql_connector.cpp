#include "mysql_connector.h"
#include <iostream>

using std::string;
using std::vector;
using std::unique_ptr;

int mysql_connector::connect_to(){
    conn.reset(mysql_init(nullptr));
    is_conn = (mysql_real_connect(conn.get(), server.c_str(), user.c_str(), password.c_str(),
                                 database.c_str(), port, NULL, CLIENT_MULTI_RESULTS) != nullptr);
    return !is_conn;
}

int mysql_connector::connect_to(const string &server_, const string &user_, const string &password_, const string &database_, unsigned port_){
    server = server_;
    user = user_;
    password = password_;
    database = database_;
    port = port_;
    return connect_to();
}

std::pair<std::vector< std::vector<std::string> >, int> mysql_connector::query(const std::string &q){
    vector< vector<string> > result;
    if (conn == nullptr){
        return {result, 1};
    }

    if (mysql_real_query(conn.get(), q.c_str(), q.size())){
        return {result, 2};
    }

    pMYSQL_RES res{mysql_use_result(conn.get())};
    if (res == nullptr) {
        return {result, mysql_errno(conn.get())};
    };

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(res.get())) != NULL){
        vector<string> tmprow;
        int fields_num = mysql_num_fields(res.get());

        for (int i = 0;i<fields_num;i++){
            tmprow.push_back(row[i]);
        }

        result.push_back(tmprow);
    }
    return {result, 0};
}