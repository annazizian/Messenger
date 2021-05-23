cmake_minimum_required(VERSION 3.10)
add_definitions("-Wno-deprecated")

#set project name
project(Messenger)
add_subdirectory(sqlpp11-connector-sqlite3)
add_subdirectory(src)