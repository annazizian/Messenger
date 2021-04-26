#include <messenger/connection.h>

Connection::Connection()
{
    assert(db != nullptr); 
}
Connection::Connection(std::string path, bool debug)
{
    set_config(path, debug);
}
void Connection::set_config(std::string path, bool debug)
{
    assert(db == nullptr);
    sql::connection_config config;
    config.path_to_database = path;
    config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    config.debug = debug;
    db = new sql::connection(config);
}
sql::connection* Connection::get_db()
{
    return this->db;
} 
sql::connection* Connection::db = nullptr; 