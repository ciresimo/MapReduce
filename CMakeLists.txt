cmake_minimum_required(VERSION 3.20)
project(MapReduceExample LANGUAGES CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(main main.cpp)

# Include the directory where mr.h is located (assuming it's in the same directory)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})
