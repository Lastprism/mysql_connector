cmake_minimum_required(VERSION 3.15)

project(mysql_connector)

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include)


link_directories(${PROJECT_SOURCE_DIR}/lib)


add_library(mysql_connector mysql_connector.cpp thread_pool.cpp executor.cpp)


target_link_libraries(mysql_connector libmysql64)


add_executable(example example.cpp)


target_link_libraries(example mysql_connector)


file(COPY ${PROJECT_SOURCE_DIR}/lib/libmysql.dll DESTINATION ${PROJECT_BINARY_DIR})

