# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_SOURCE_DIR = /root/vsc-repo/homework/compiling

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/vsc-repo/homework/compiling/build

# Utility rule file for Experimental.

# Include the progress variables for this target.
include CMakeFiles/Experimental.dir/progress.make

CMakeFiles/Experimental:
	/usr/bin/ctest -D Experimental

Experimental: CMakeFiles/Experimental
Experimental: CMakeFiles/Experimental.dir/build.make

.PHONY : Experimental

# Rule to build all files generated by this target.
CMakeFiles/Experimental.dir/build: Experimental

.PHONY : CMakeFiles/Experimental.dir/build

CMakeFiles/Experimental.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Experimental.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Experimental.dir/clean

CMakeFiles/Experimental.dir/depend:
	cd /root/vsc-repo/homework/compiling/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/vsc-repo/homework/compiling /root/vsc-repo/homework/compiling /root/vsc-repo/homework/compiling/build /root/vsc-repo/homework/compiling/build /root/vsc-repo/homework/compiling/build/CMakeFiles/Experimental.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Experimental.dir/depend

