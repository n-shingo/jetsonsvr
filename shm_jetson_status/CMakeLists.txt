cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(shm_jetson_status)

# set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# set the build type
set(CMAKE_BUILD_TYPE Release)

include_directories( ../src/ )

# confirmation messages
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")


add_definitions("-std=c++11")
add_executable(test-writer ./src/test-writer.cpp ./src/shmJetsonStatus.cpp)
add_executable(test-reader ./src/test-reader.cpp ./src/shmJetsonStatus.cpp)


