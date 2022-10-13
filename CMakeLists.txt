cmake_minimum_required(VERSION 3.10)
project(mytinywebserver)
add_compile_options(-std=c++17)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++0x -rdynamic -O2 -ggdb -Wall -Wno-deprecated -Werror -Wno-unused-function -DCMAKE_CXX_CPPLINT=cpplint")

set(LIB_SRC
    src/log/LogFile.cpp
    src/log/Logger.cpp
    src/log/LogEvent.cpp
    src/log/LogFormatter.cpp
    src/log/LogAppender.cpp
    src/log/LogManager.cpp
    src/eventloop.cpp
    src/channel.cpp
    src/epollpoller.cpp
    src/CurrentThread.cpp
    src/Timestamp.cpp
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
    src/Timer.cpp
)

add_library(mytinywebserver SHARED ${LIB_SRC})

add_executable(testHttpServer tests/testHttpserver.cpp)
add_dependencies(testHttpServer mytinywebserver)
target_link_libraries(testHttpServer mytinywebserver)
target_link_libraries(testHttpServer pthread)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)