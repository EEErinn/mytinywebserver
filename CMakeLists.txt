cmake_minimum_required(VERSION 3.10)
project(mytinywebserver)
add_compile_options(-std=c++17)

# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)
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
    src/Acceptor.cpp
    src/socket.cpp
    src/socketops.cpp
    src/InetAddress.cpp
    src/Tcpserver.cpp
    src/TcpConnection.cpp
    src/Buffer.cpp
    src/eventloopthreadpool.cpp
    src/eventloopthread.cpp
    src/http/httpcontext.cpp
    src/http/httpresponse.cpp
    src/http/httpserver.cpp
    src/log/Logger.cpp
    src/log/LogEvent.cpp
    src/log/LogFormatter.cpp
    src/log/LogAppender.cpp
    src/log/singleton.cpp
    src/log/LogUtils.cpp
    src/Timer.cpp
)

add_library(mytinywebserver SHARED ${LIB_SRC})
# add_executable(testEventloop tests/testEventloop.cpp)
# add_dependencies(testEventloop mytinywebserver)
# target_link_libraries(testEventloop mytinywebserver)
# target_link_libraries(testEventloop pthread)

# add_executable(testEvent tests/testEvent.cpp)
# add_dependencies(testEvent mytinywebserver)
# target_link_libraries(testEvent mytinywebserver)
# target_link_libraries(testEvent pthread)

# add_executable(testAcceptor tests/testAcceptor.cpp)
# add_dependencies(testAcceptor mytinywebserver)
# target_link_libraries(testAcceptor mytinywebserver)
# target_link_libraries(testAcceptor pthread)

add_executable(testHttpServer tests/testHttpserver.cpp)
add_dependencies(testHttpServer mytinywebserver)
target_link_libraries(testHttpServer mytinywebserver)
target_link_libraries(testHttpServer pthread)

# add_executable(testSharedPtr tests/testSharedPtr.cpp)

# add_executable(testLog tests/testLog.cpp)
# add_dependencies(testLog mytinywebserver)
# target_link_libraries(testLog mytinywebserver)
# target_link_libraries(testLog pthread)

# set(LIB_SRC
#     src/log/a.cpp
#     src/log/singleton.cpp
# )

add_executable(testTimer tests/testTimer.cpp)
add_dependencies(testTimer mytinywebserver)
target_link_libraries(testTimer mytinywebserver)
target_link_libraries(testTimer pthread)

# # add_library(singleton SHARED ${LIB_SRC})
# add_executable(testSingleton tests/testSingleton.cpp)
# # add_dependencies(testSingleton singleton)
# # target_link_libraries(testSingleton singleton)
# target_link_libraries(testSingleton pthread)


# add_executable(test_yml tests/test_yml.cpp)
# target_link_libraries(test_yml yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)