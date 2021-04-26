#pragma once
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/sqlite3/sqlite3.h"
#include <string>
#include <assert.h>

namespace sql = sqlpp::sqlite3; 

class Connection 
{
private:
    static sql::connection* db;
    void set_config(std::string path, bool debug);
public:
    Connection();
    Connection(std::string path, bool debug);
    sql::connection* get_db();
    // FIXME:
    // ~_Connection(){
    //     delete db;
    // }
};