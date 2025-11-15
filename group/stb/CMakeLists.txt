# === Exclude main and test files using regex
file(GLOB SRC_FILES CONFIGURE_DEPENDS "*.cpp")
list(FILTER SRC_FILES EXCLUDE REGEX ".*\\.m\\.cpp$")
list(FILTER SRC_FILES EXCLUDE REGEX ".*\\.t\\.cpp$")

# === Build Source files as libraries
add_library(stblib STATIC ${SRC_FILES})
target_include_directories(stblib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})