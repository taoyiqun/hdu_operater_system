# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.15

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2019.2.1\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2019.2.1\bin\cmake\win\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\10059\CLionProjects\untitled13

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\10059\CLionProjects\untitled13\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/untitled13.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/untitled13.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/untitled13.dir/flags.make

CMakeFiles/untitled13.dir/main.c.obj: CMakeFiles/untitled13.dir/flags.make
CMakeFiles/untitled13.dir/main.c.obj: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\10059\CLionProjects\untitled13\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/untitled13.dir/main.c.obj"
	C:\PROGRA~2\CODEBL~1\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\untitled13.dir\main.c.obj   -c C:\Users\10059\CLionProjects\untitled13\main.c

CMakeFiles/untitled13.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/untitled13.dir/main.c.i"
	C:\PROGRA~2\CODEBL~1\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\10059\CLionProjects\untitled13\main.c > CMakeFiles\untitled13.dir\main.c.i

CMakeFiles/untitled13.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/untitled13.dir/main.c.s"
	C:\PROGRA~2\CODEBL~1\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\10059\CLionProjects\untitled13\main.c -o CMakeFiles\untitled13.dir\main.c.s

# Object files for target untitled13
untitled13_OBJECTS = \
"CMakeFiles/untitled13.dir/main.c.obj"

# External object files for target untitled13
untitled13_EXTERNAL_OBJECTS =

untitled13.exe: CMakeFiles/untitled13.dir/main.c.obj
untitled13.exe: CMakeFiles/untitled13.dir/build.make
untitled13.exe: CMakeFiles/untitled13.dir/linklibs.rsp
untitled13.exe: CMakeFiles/untitled13.dir/objects1.rsp
untitled13.exe: CMakeFiles/untitled13.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\10059\CLionProjects\untitled13\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable untitled13.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\untitled13.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/untitled13.dir/build: untitled13.exe

.PHONY : CMakeFiles/untitled13.dir/build

CMakeFiles/untitled13.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\untitled13.dir\cmake_clean.cmake
.PHONY : CMakeFiles/untitled13.dir/clean

CMakeFiles/untitled13.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\10059\CLionProjects\untitled13 C:\Users\10059\CLionProjects\untitled13 C:\Users\10059\CLionProjects\untitled13\cmake-build-debug C:\Users\10059\CLionProjects\untitled13\cmake-build-debug C:\Users\10059\CLionProjects\untitled13\cmake-build-debug\CMakeFiles\untitled13.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/untitled13.dir/depend

