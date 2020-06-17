//
// Created by Lastprism on 2020/6/5.
//

#include "mysql_connector.h"
#include "thread_pool.h"
#include "executor.h"
#include <iostream>
#include <cstdio>
#include <fstream>

#define SERVER "127.0.0.1"
#define USER "root"
#define PASSWORD "root"
#define DATABASE "test"

using namespace std;

void use_mysql_connector() {
	mysql_connector mc;
	if (mc.connect_to(SERVER, USER, PASSWORD, DATABASE)) {

		cerr << "connect error : " << mc.error() << endl;
		exit(1);

	}
	auto res = mc.query("select * from test_blob");

	if (res.second) {
		cerr << "query error : " << mc.error() << endl;
		exit(1);
	}

	for (auto& row : res.first) {
		for (auto& col : row) {
			cout << col << " ";
		}
		cout << endl;
	}
}

void use_thread_pool() {
	thread_pool tp{ 5, SERVER, USER, PASSWORD, DATABASE };
	for (int i = 0; i < 30; ++i) {
		const char* ptr = "insert into test_blob values('aaaaa\0aaaaa')";
		string sql{ ptr, ptr + sizeof("insert into test_blob values('aaaaa\0aaaaa')") };
		tp.push(sql);
	}
}

void use_executor() {
	std::shared_ptr<executor> spExtor{ new executor{20} };
	std::thread storeThread{ [spExtor]() {
		spExtor->init(SERVER, USER, PASSWORD, DATABASE);
		spExtor->manager();
	} };
	while (true) {
		auto startTime = std::chrono::steady_clock::now();
		for (int i = 0; i < 200; ++i) {
			const char* ptr = "insert into test_blob values('aaaaa\0aaaaa')";
			string sql{ ptr, ptr + sizeof("insert into test_blob values('aaaaa\0aaaaa')") };
			spExtor->push(sql);
		}
		auto endTime = std::chrono::steady_clock::now();
		auto du = endTime - startTime;

		std::cout << "sleep " << std::chrono::duration_cast<std::chrono::milliseconds>(du).count() << "ms" << endl;
		endTime = std::chrono::steady_clock::now();
		du = endTime - startTime;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 - std::chrono::duration_cast<std::chrono::milliseconds>(du).count()));
	}

	storeThread.join();
}

void  test_spd() {
	//std::shared_ptr<executor> spExtor{ new executor{20} };
	//std::thread storeThread{ [spExtor]() {
	//	spExtor->init(SERVER, USER, PASSWORD, DATABASE);
	//	spExtor->manager();
	//} };

	thread_pool tp{ 20, SERVER, USER, PASSWORD, DATABASE };
	while (true) {
		auto startTime = std::chrono::steady_clock::now();
		for (int i = 0; i < 1000; ++i) {
			const char* ptr = "insert into test_blob values('aaaaa\0aaaaa')";
			string sql{ ptr, ptr + sizeof("insert into test_blob values('aaaaa\0aaaaa')") };
			tp.push(sql);
			//spExtor->push(sql);
		}
		auto endTime = std::chrono::steady_clock::now();
		auto du = endTime - startTime;

		std::cout << "sleep " << 1000 - std::chrono::duration_cast<std::chrono::milliseconds>(du).count() << "ms" << endl;
		std::cout << "task size is " << tp.taskSize() << std::endl;
		endTime = std::chrono::steady_clock::now();
		du = endTime - startTime;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 - std::chrono::duration_cast<std::chrono::milliseconds>(du).count()));
	}
	//storeThread.join();
}

int main() {
	//use_mysql_connector();
	//use_thread_pool();
	//use_executor();
	//use();
	test_spd();
	return 0;
}



