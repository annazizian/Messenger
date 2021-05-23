# Messenger

## SQL Library
### sqlpp11

Works with multiple databases. 

## Build

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
src/db_generator #for creating empty database
```

### Database model generation for cpp

```
/usr/local/bin/sqlpp11-ddl2cpp messenger.ddl include/messenger/models messenger
```