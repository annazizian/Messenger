add_library(
    messenger
    user.cpp
    connection.cpp
    helpers.cpp
    message.cpp
    MessageQueue.cpp
    handlers.cpp
    groups.cpp
)

add_library(
    socket_parser
    message_parser.cpp
)

target_include_directories(messenger PUBLIC ../include)
target_include_directories(socket_parser PUBLIC ../include)

target_link_libraries(messenger PRIVATE sqlpp11 sqlpp11-connector-sqlite3 socket_parser)

add_executable(db_generator db_generator.cpp)

target_link_libraries(db_generator PUBLIC messenger)

#### Thread
if (UNIX)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
endif (UNIX)
#### Thread ####

add_executable(socket_server socket_server.cpp)
add_executable(socket_client socket_client.cpp)

#### Thread
if (UNIX)
    target_link_libraries(socket_server PRIVATE Threads::Threads)
    target_link_libraries(socket_client PRIVATE Threads::Threads)
endif (UNIX)
#### Thread ####

target_link_libraries(socket_server PUBLIC messenger socket_parser)
target_link_libraries(socket_client PUBLIC socket_parser)