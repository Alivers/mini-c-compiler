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

# Include any dependencies generated for this target.
include CMakeFiles/compiler.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/compiler.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/compiler.dir/flags.make

CMakeFiles/compiler.dir/src/compiler.cc.o: CMakeFiles/compiler.dir/flags.make
CMakeFiles/compiler.dir/src/compiler.cc.o: ../src/compiler.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/vsc-repo/homework/compiling/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/compiler.dir/src/compiler.cc.o"
	/usr/bin/g++-8  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/compiler.dir/src/compiler.cc.o -c /root/vsc-repo/homework/compiling/src/compiler.cc

CMakeFiles/compiler.dir/src/compiler.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compiler.dir/src/compiler.cc.i"
	/usr/bin/g++-8 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/vsc-repo/homework/compiling/src/compiler.cc > CMakeFiles/compiler.dir/src/compiler.cc.i

CMakeFiles/compiler.dir/src/compiler.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compiler.dir/src/compiler.cc.s"
	/usr/bin/g++-8 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/vsc-repo/homework/compiling/src/compiler.cc -o CMakeFiles/compiler.dir/src/compiler.cc.s

# Object files for target compiler
compiler_OBJECTS = \
"CMakeFiles/compiler.dir/src/compiler.cc.o"

# External object files for target compiler
compiler_EXTERNAL_OBJECTS =

../bin/compiler: CMakeFiles/compiler.dir/src/compiler.cc.o
../bin/compiler: CMakeFiles/compiler.dir/build.make
../bin/compiler: CMakeFiles/compiler.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/vsc-repo/homework/compiling/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/compiler"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/compiler.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/compiler.dir/build: ../bin/compiler

.PHONY : CMakeFiles/compiler.dir/build

CMakeFiles/compiler.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/compiler.dir/cmake_clean.cmake
.PHONY : CMakeFiles/compiler.dir/clean

CMakeFiles/compiler.dir/depend:
	cd /root/vsc-repo/homework/compiling/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/vsc-repo/homework/compiling /root/vsc-repo/homework/compiling /root/vsc-repo/homework/compiling/build /root/vsc-repo/homework/compiling/build /root/vsc-repo/homework/compiling/build/CMakeFiles/compiler.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/compiler.dir/depend

