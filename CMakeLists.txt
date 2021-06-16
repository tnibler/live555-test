cmake_minimum_required(VERSION 3.4)
project(live555-test)

add_definitions("-std=c++11")

set(CONAN_DISABLE_CHECK_COMPILER 1)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(server src/server.cpp
src/Encoder.cpp
src/LiveStreamSource.cpp
src/H264VideoLiveServerMediaSubsession.cpp)

set(LIVE555_INCLUDE_DIR "${CONAN_INCLUDE_DIRS_LIVE555}/live555")
include_directories(${LIVE555_INCLUDE_DIR}/UsageEnvironment)
include_directories(${LIVE555_INCLUDE_DIR}/BasicUsageEnvironment)
include_directories(${LIVE555_INCLUDE_DIR}/groupsock)
include_directories(${LIVE555_INCLUDE_DIR}/liveMedia)

include_directories(src/include)

# target_include_directories(server PUBLIC ${CONAN_INCLUDE_DIRS_LIVE555})
target_link_libraries(server ${CONAN_LIBS})
