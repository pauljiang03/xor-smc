# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.30.5/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/paul/documents/xor-smc

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/paul/documents/xor-smc/build

# Include any dependencies generated for this target.
include examples/CMakeFiles/simple_example.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include examples/CMakeFiles/simple_example.dir/compiler_depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/simple_example.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/simple_example.dir/flags.make

examples/CMakeFiles/simple_example.dir/simple_example.cpp.o: examples/CMakeFiles/simple_example.dir/flags.make
examples/CMakeFiles/simple_example.dir/simple_example.cpp.o: /Users/paul/documents/xor-smc/examples/simple_example.cpp
examples/CMakeFiles/simple_example.dir/simple_example.cpp.o: examples/CMakeFiles/simple_example.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/paul/documents/xor-smc/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/simple_example.dir/simple_example.cpp.o"
	cd /Users/paul/documents/xor-smc/build/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT examples/CMakeFiles/simple_example.dir/simple_example.cpp.o -MF CMakeFiles/simple_example.dir/simple_example.cpp.o.d -o CMakeFiles/simple_example.dir/simple_example.cpp.o -c /Users/paul/documents/xor-smc/examples/simple_example.cpp

examples/CMakeFiles/simple_example.dir/simple_example.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/simple_example.dir/simple_example.cpp.i"
	cd /Users/paul/documents/xor-smc/build/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/paul/documents/xor-smc/examples/simple_example.cpp > CMakeFiles/simple_example.dir/simple_example.cpp.i

examples/CMakeFiles/simple_example.dir/simple_example.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/simple_example.dir/simple_example.cpp.s"
	cd /Users/paul/documents/xor-smc/build/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/paul/documents/xor-smc/examples/simple_example.cpp -o CMakeFiles/simple_example.dir/simple_example.cpp.s

# Object files for target simple_example
simple_example_OBJECTS = \
"CMakeFiles/simple_example.dir/simple_example.cpp.o"

# External object files for target simple_example
simple_example_EXTERNAL_OBJECTS =

examples/simple_example: examples/CMakeFiles/simple_example.dir/simple_example.cpp.o
examples/simple_example: examples/CMakeFiles/simple_example.dir/build.make
examples/simple_example: libxor_smc.a
examples/simple_example: examples/CMakeFiles/simple_example.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/paul/documents/xor-smc/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable simple_example"
	cd /Users/paul/documents/xor-smc/build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simple_example.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/simple_example.dir/build: examples/simple_example
.PHONY : examples/CMakeFiles/simple_example.dir/build

examples/CMakeFiles/simple_example.dir/clean:
	cd /Users/paul/documents/xor-smc/build/examples && $(CMAKE_COMMAND) -P CMakeFiles/simple_example.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/simple_example.dir/clean

examples/CMakeFiles/simple_example.dir/depend:
	cd /Users/paul/documents/xor-smc/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/paul/documents/xor-smc /Users/paul/documents/xor-smc/examples /Users/paul/documents/xor-smc/build /Users/paul/documents/xor-smc/build/examples /Users/paul/documents/xor-smc/build/examples/CMakeFiles/simple_example.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : examples/CMakeFiles/simple_example.dir/depend
