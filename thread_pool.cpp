//
// Created by Lastprism on 2020/6/5.
//

#include "thread_pool.h"
#include <cassert>
#include <iostream>

void thread_pool::push(string sql){
    std::lock_guard<mutex> lk(mx);
{
//std::cout << "push [" << sql << "] into tasks" << std::endl;
//fprintf(stdout, "push [%s] into tasks\n", sql.c_str());
}
    tasks.push(move(sql));
    cv.notify_one();
}

void thread_pool::work(pmysql_connector pmc){
    //在这里connect_to会报找不到主机的错误
    string sql;
    while(1){
        {
            unique_lock lk(mx);
            while (tasks.empty() && !stop) {
                cv.wait(lk);
            }
            if (tasks.empty() && stop) {
                break;
            }
            sql = tasks.front();
            tasks.pop();
        }

//{
//std::lock_guard<mutex> lk(mx);
//std::cout << "thread"<< std::this_thread::get_id << " execute sql [" << sql << "]" << std::endl;
//}

//fprintf(stdout, "thread%d execute sql [%s]\n", std::this_thread::get_id(), sql.c_str());
        auto ans = pmc->query(sql);
        if(ans.second != 0){

{
std::lock_guard<mutex> lk(mx);
std::cerr << "thread" << std::this_thread::get_id() << "query error : " << ans.second << ", " << pmc->error() << std::endl;
//fprintf(stderr, "thread%d query error : %d, %s\n", std::this_thread::get_id(), ans.second, pmc->error().c_str());
}

            //exit(0);
            return;
        }
    }
}

void thread_pool::terminal(){
    {
        std::lock_guard<mutex> lk(mx);
        stop = 1;
        cv.notify_all();
    }
    for(auto &worker : workers){
        worker.join();
    }
}

int thread_pool::taskSize(){
    std::lock_guard<mutex> lk(mx);
    return tasks.size();
}