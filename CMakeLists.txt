cmake_minimum_required(VERSION 3.4)
project(live555-test)

add_definitions("-std=c++11")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
# conan_set_find_paths()
# conan_set_find_library_paths()

add_executable(server src/main.cpp)
# target_include_directories(server PUBLIC ${CONAN_INCLUDE_DIRS_LIVE555})
target_link_libraries(server ${CONAN_LIBS})
