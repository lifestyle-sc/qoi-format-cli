cmake_minimum_required(VERSION 3.14)
project(example CXX C)

set(CMAKE_CXX_STANDARD 23)
enable_testing()

add_subdirectory(src)

add_custom_target(
    check-format ALL
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/format.sh --verify-only ${CMAKE_CURRENT_SOURCE_DIR}/src
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Verifying source code formatting"
)
