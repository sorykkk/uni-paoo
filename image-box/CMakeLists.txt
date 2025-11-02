# root
cmake_minimum_required(VERSION 3.20)    # minimum cmake version because i do not support legacy type shit
project(IMAGEBOX LANGUAGES CXX)         # name & supported languages

set(CMAKE_CXX_STANDARD 17)              # specified language standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # require a standard

# Global settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)   # where executables go
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)   # where shared libraries go
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)   # where static libraries go

add_subdirectory(src)
add_subdirectory(example)