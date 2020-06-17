//
// Created by Lastprism on 2020/6/5.
//

#include "mysql_connector.h"
#include "thread_pool.h"
#include <iostream>
#include <cstdio>

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
		//char buf[1024];
		//snprintf(buf, 1024,  "insert into test_blob values ('\1\1\1\1\0\1\1');", i);
		//tp.push(buf);
	}
}


int main() {
	//use_mysql_connector();
	use_thread_pool();
	return 0;
}


