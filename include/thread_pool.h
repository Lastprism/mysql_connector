//
// Created by Lastprism on 2020/6/5.
//

#ifndef MYSQL_CONNECTOR_THREAD_POOL_H
#define MYSQL_CONNECTOR_THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "mysql_connector.h"
#include "noncopyable.h"

class thread_pool : public noncopyable{
private:
    using thread = std::thread;
    using thread_vector = std::vector<thread>;
    using string = std::string;
    using mutex = std::mutex;
    using condition_variable = std::condition_variable;
    using unique_lock = std::unique_lock<mutex>;
    using pmysql_connector = std::unique_ptr<mysql_connector>;


    int pool_size;
    thread_vector workers;
    std::queue< string > tasks;
    std::mutex mx;
    condition_variable cv;
    bool stop {false};

    void work(pmysql_connector pmc);

public:
    thread_pool(int pool_size_, const string &server, const string &user, const string &password, const string &database, unsigned port = 0) :
        pool_size(pool_size_){
        for(int i = 0; i < pool_size; ++i){
            pmysql_connector pmc{new mysql_connector{server, user, password, database, port}};
            if(pmc->connect_to() != 0){
                fprintf(stderr, "connect error : %s\n", pmc->error().c_str());
                exit(0);
            }
            workers.emplace_back(&thread_pool::work, this, move(pmc));
        }
    }

    thread_pool(thread_pool &&) = delete;
    thread_pool &operator=(thread_pool &&) = delete;

    void push(string sql);

    void terminal();

    int is_stop(){return stop;}

    int taskSize();

    ~thread_pool(){
        if(!stop){
            terminal();
        }
    }
};


#endif //MYSQL_CONNECTOR_THREAD_POOL_H
