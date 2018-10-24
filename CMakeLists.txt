cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(jetsonsvr_tc)

include_directories( ../systemMgr/src ../utility/src)

# set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# set the build type
set(CMAKE_BUILD_TYPE Release)
# confirmation messages
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

find_package(PkgConfig)
find_package(Threads REQUIRED)

add_executable(jetsonsvr_tc
	./src/jetsonsvr.cpp
	./src/UdpSender.cpp ./src/UdpReceiver2.cpp ./src/Tool.cpp)
#add_executable(jetson-recver ./src/jetson-recver.cpp ./src/UdpReceiver2.cpp ./src/Tool.cpp)
#add_executable(jetson-sender ./src/jetson-sender.cpp ./src/UdpSender.cpp ./src/Tool.cpp)
add_executable(test-viewer ./src/test-viewer.cpp ./src/Tool.cpp)
add_executable(test-command	./src/test-command.cpp	./src/Tool.cpp)
add_definitions("-std=c++11")

target_link_libraries(jetsonsvr_tc ssm Threads::Threads )
#target_link_libraries(jetson-recver ssm)
#target_link_libraries(jetson-sender ssm)
target_link_libraries(test-viewer ssm)
target_link_libraries(test-command ssm)


add_subdirectory(shm_jetson_status)
