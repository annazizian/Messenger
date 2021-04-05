cmake_minimum_required(VERSION 3.10)

#set project name
project(Messenger) 
include_directories(include)
add_subdirectory(sqlpp11-connector-sqlite3)
add_subdirectory(src)