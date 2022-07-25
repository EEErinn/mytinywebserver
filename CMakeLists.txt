cmake_minimum_required(VERSION 3.10)
project(mytinywebserver)
add_compile_options(-std=c++17)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++0x -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -DCMAKE_CXX_CPPLINT=cpplint")

set(LIB_SRC
    src/eventloop.cpp
    src/channel.cpp
    src/poller.cpp
    src/epollpoller.cpp
    src/CurrentThread.cpp
    src/Timestamp.cpp
    src/defaultPoller.cpp
)

add_library(mytinywebserver SHARED ${LIB_SRC})
add_executable(testEventloop tests/testEventloop.cpp)
add_dependencies(testEventloop mytinywebserver)
target_link_libraries(testEventloop mytinywebserver)

add_executable(testEvent tests/testEvent.cpp)
add_dependencies(testEvent mytinywebserver)
target_link_libraries(testEvent mytinywebserver)
target_link_libraries(testEvent pthread)

# add_executable(test_config tests/testConfig.cpp)
# add_dependencies(test_config mysylar)
# target_link_libraries(test_config mysylar)

# add_executable(test_yml tests/test_yml.cpp)
# target_link_libraries(test_yml yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)