#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "coroutine.h"
#include "noncopyable.h"
#include "mysql_connector.h"
#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <atomic>


class executor : public noncopyable {
public:
	using Worker = coroutine::routine_t;
	using Workers = std::vector<Worker>;
	using string = std::string;
	using pmysql_connector = std::shared_ptr<mysql_connector>;



	executor(const executor&&) = delete;
	executor& operator=(const executor&&) = delete;
	executor(int workSize_);

	void init(const string& server, const string& user, const string& password, const string& database, unsigned port = 0);

	void push(string sql);

	void terminal();

	~executor();

	void worker(pmysql_connector pmc, int index);

	void manager();

private:
	


private:
	Workers allWorkers;
	std::queue<int> freeWorkersIndex;
	std::queue<int> busyWorkersIndex;
	int workerSize{ 1 };
	std::queue< string > tasks;
	std::queue< string > readyTasks;
	std::mutex mx;
	std::atomic_bool stop{ false };

};

#include "executor.h"
#include <iostream>
#include <thread>

executor::executor(int workerSize_) :
	workerSize(workerSize_) {
}

void executor::init(const string& server, const string& user, const string& password, const string& database, unsigned port) {
	allWorkers.emplace_back(coroutine::create(std::bind(&executor::manager, this)));


	for (int i = 1; i <= workerSize; ++i) {
		pmysql_connector pmc{ new mysql_connector{server, user, password, database, port} };
		if (pmc->connect_to() != 0) {
			std::cerr << "connect mysql error : " << pmc->error() << std::endl;
			exit(1);
		}
		allWorkers.emplace_back(coroutine::create(std::bind(&executor::worker, this, pmc, i)));
	}

	for (int i = 1; i <= workerSize; ++i) {
		coroutine::resume(allWorkers[i]);
	}
}


void executor::push(string sql) {
	std::lock_guard<std::mutex> lk(mx);
	//std::cout << "push [" << sql << "] into taskQueue" << std::endl;
	readyTasks.push(std::move(sql));
}

void executor::terminal() {
	stop.store(true);
	for (int i = 0; i < workerSize; ++i) {
		coroutine::resume(allWorkers[i]);
	}
	for (int i = 0; i < workerSize; ++i) {
		coroutine::destroy(allWorkers[i]);
	}
}

executor::~executor() {
	terminal();
}

void executor::manager() {
	while (true) {
		{
			std::lock_guard<std::mutex> lk(mx);
			if (tasks.empty() && readyTasks.size())
				swap(tasks, readyTasks);
		}
		if (tasks.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
		
		std::cout << "tasks.size() = " << tasks.size() << std::endl;
//		std::cout << "readyTasks.size() = " << readyTasks.size() << std::endl;
//		std::cout << "free.size() = " << freeWorkersIndex.size() << std::endl;
		
		while (tasks.size()) {
			while (tasks.size() && freeWorkersIndex.size()) {
				int index = freeWorkersIndex.front();
				freeWorkersIndex.pop();
				coroutine::resume(allWorkers[index]);
//				std::cout << "There are " << freeWorkersIndex.size() << " free workers"<< std::endl;
			}
			while (busyWorkersIndex.size()) {
				int index = busyWorkersIndex.front();
				coroutine::resume(allWorkers[index]);
//				std::cout << "There are " << busyWorkersIndex.size() << " busy workers" << std::endl;
			}
//			std::cout << "a circle" << std::endl;
			std::cout << "There are " << tasks.size() << " tasks" << std::endl;
		}
	}
}


void executor::worker(pmysql_connector pmc, int index) {
	std::cout << "worker" << index << std::endl;
	while (true) {
		freeWorkersIndex.push(index);
		coroutine::yield();
		if (tasks.empty() && stop) {
			break;
		}
		string sql = move(tasks.front());
		tasks.pop();
//		std::cout << "routine " << index << " execute sql [ " << sql << " ]" << std::endl;
		busyWorkersIndex.push(index);
//		std::cout << "routine " << index << " enqueue busy queue, now busy queue size is " << busyWorkersIndex.size() << std::endl;
		auto ans = coroutine::await([&pmc, &sql]() {
			return pmc->query(sql);
			});
		busyWorkersIndex.pop();
//		std::cout << "routine " << index << " finish executing sql [ " << sql << " ]" << std::endl;
		if (ans.second) {
			std::cerr << "query error : " << pmc->error() << std::endl;
			return;
		}
	}
}

#endif
