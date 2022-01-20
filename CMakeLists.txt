cmake_minimum_required(VERSION 3.20)
project(ashti C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(src/c_subprocess)
add_subdirectory(src/file_fetcher)
add_subdirectory(src/http_handler)
add_subdirectory(src/socket_factory)
add_subdirectory(src/web_server)
add_subdirectory(test/)
