# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Users/Anna/Desktop/Messenger

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/Anna/Desktop/Messenger/build

# Utility rule file for ExperimentalCoverage.

# Include the progress variables for this target.
include sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/progress.make

sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage:
	cd /mnt/c/Users/Anna/Desktop/Messenger/build/sqlpp11-connector-sqlite3 && /usr/bin/ctest -D ExperimentalCoverage

ExperimentalCoverage: sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage
ExperimentalCoverage: sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/build.make

.PHONY : ExperimentalCoverage

# Rule to build all files generated by this target.
sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/build: ExperimentalCoverage

.PHONY : sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/build

sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/clean:
	cd /mnt/c/Users/Anna/Desktop/Messenger/build/sqlpp11-connector-sqlite3 && $(CMAKE_COMMAND) -P CMakeFiles/ExperimentalCoverage.dir/cmake_clean.cmake
.PHONY : sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/clean

sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/depend:
	cd /mnt/c/Users/Anna/Desktop/Messenger/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/Anna/Desktop/Messenger /mnt/c/Users/Anna/Desktop/Messenger/sqlpp11-connector-sqlite3 /mnt/c/Users/Anna/Desktop/Messenger/build /mnt/c/Users/Anna/Desktop/Messenger/build/sqlpp11-connector-sqlite3 /mnt/c/Users/Anna/Desktop/Messenger/build/sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : sqlpp11-connector-sqlite3/CMakeFiles/ExperimentalCoverage.dir/depend

