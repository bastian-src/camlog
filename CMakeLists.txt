cmake_minimum_required(VERSION 3.10)
project(camlog)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(camlog src/main.cpp)

find_package(JPEG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(EXIF REQUIRED libexif)

target_include_directories(camlog PRIVATE ${JPEG_INCLUDE_DIR} ${EXIF_INCLUDE_DIRS})
target_link_libraries(camlog PRIVATE ${JPEG_LIBRARY} ${EXIF_LIBRARIES})

target_compile_options(camlog PRIVATE -Wall -Wextra -Wpedantic -Werror -g)

# Run check.sh to make a memory leakage check
add_custom_target(check
    COMMAND valgrind -s --track-origins=yes --leak-check=full ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/camlog
    DEPENDS camlog
    COMMENT "Running Valgrind to check for memory leaks"
)
