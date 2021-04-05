#pragma once
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/sqlite3/sqlite3.h"
#include <string>
#include <assert.h>

namespace sql = sqlpp::sqlite3; 

struct Connection{
    static sql::connection* db;
    Connection(){
        assert(db != nullptr); 
    }
    Connection(std::string path, bool debug){
        set_config(path, debug);
    }
    void set_config(std::string path, bool debug){
        assert(db == nullptr);
        sql::connection_config config;
        config.path_to_database = path;
        config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        config.debug = debug;
        db = new sql::connection(config);
    }
    sql::connection* get_db(){return this->db;}
    // FIXME:
    // ~_Connection(){
    //     delete db;
    // }
};

sql::connection* Connection::db = nullptr;