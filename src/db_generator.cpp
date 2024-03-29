#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/sqlite3/sqlite3.h"

namespace sql = sqlpp::sqlite3;
int main() {
    sql::connection_config config;
    config.path_to_database = "db.sqlite";
    config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    config.debug = true; //debug aneluts messagenery tesnelu hamar

    sql::connection db(config); 
    db.execute("CREATE TABLE User( \
    username varchar(255) PRIMARY KEY,\
    status bool NOT NULL,\
    private_guid varchar(64) NOT NULL);");  
    db.execute("CREATE TABLE message ( \
    messageGUID VARCHAR(64) NOT NULL PRIMARY KEY,\
    sender_username VARCHAR(255) NOT NULL,\
    reciever_username VARCHAR(255) NOT NULL,\
    content VARCHAR(150) NOT NULL,\
    timestamp INT NOT NULL,\
    isGroup bool NOT NULL);");
    db.execute("CREATE TABLE groups (\
    groupId varchar(64) NOT NULL PRIMARY KEY,\
    groupName VARCHAR(150) NOT NULL,\
    createDate DATE NOT NULL,\
    isActive bool);"); 
    db.execute("CREATE TABLE user_group (\
    username varchar(255) NOT NULL,\
    groupId varchar(64) NOT NULL,\
    FOREIGN KEY (username) REFERENCES user(username),\
    FOREIGN KEY (groupId) REFERENCES groups(groupId),\
    PRIMARY KEY (username, groupId));");
}