# Messenger

## SQL Library
### sqlpp11

Works with multiple databases. Currently sqlite3 is used.
You'll need `sqlite3` and `libsqlite3-dev` for that.

## Build

### Build sqlpp11

Install sqlpp. First clone the sqlpp11 repository. Change your current directory to cloned one.
After, do the following:

```
mkdir build
cd build
cmake ..
sudo make install  # sudo for installing to /usr/local directory
```

Do the same for `sqlpp11-connector-sqlite3`.

### Build messenger

Don't forget to update submodules
```
git submodule update --init --recursive
```

Do the following to build project, create database and run main script.
```
mkdir build
cd build
cmake ..

make
src/db_generator # for creating empty database
```

### Database model (re)generation for cpp

```
/usr/local/bin/sqlpp11-ddl2cpp messenger.ddl include/messenger/models messenger
```

## Running

Do the following from `build` directory to start socket server:

```
src/socket_server
```

Similarly you can start socket clients and connect to the server

```
src/socket_client
```