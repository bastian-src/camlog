cmake_minimum_required(VERSION 3.10)
project(camlog)
include(ExternalProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(camlog src/main.cpp)


# Statically link libaries
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
include(Dependencies.cmake)

target_include_directories(camlog PRIVATE ${THIRD_PARTY_INCLUDE_DIR})
target_link_directories(camlog PRIVATE ${THIRD_PARTY_LIBRARY_DIR})
target_link_libraries(camlog PRIVATE jpeg exif)

target_compile_options(camlog PRIVATE -Wall -Wextra -Wpedantic -Werror -g)

# Run check.sh to make a memory leakage check
add_custom_target(check
    COMMAND valgrind -s --track-origins=yes --leak-check=full ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/camlog
    DEPENDS camlog
    COMMENT "Running Valgrind to check for memory leaks"
)
