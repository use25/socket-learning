cmake_minimum_required(VERSION 3.20)
project(socket-learning CXX)
 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# Turn on a reasonable set of warnings - catching mistakes at compile
# time is cheaper than catching them with GDB later.
add_compile_options(-Wall -Wextra)

add_executable(tcp_server src/tcp_server.cpp src/TCPHelper.cpp)
add_executable(tcp_client src/tcp_client.cpp src/TCPHelper.cpp)
add_executable(udp_server src/udp_server.cpp)
add_executable(udp_client src/udp_client.cpp)

foreach(target tcp_server tcp_client udp_server udp_client)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
endforeach()