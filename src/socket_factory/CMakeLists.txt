include_directories(
        ${CMAKE_SOURCE_DIR}/include
)

add_library(socket_factory.a socket_factory_mgr.c socket_factory.c log4j.c)
