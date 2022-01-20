include_directories(
        ${CMAKE_SOURCE_DIR}/include
)

set(
        sources ${sources}
        ../c_subprocess/c_subprocess.c
        ../file_fetcher/file_fetcher.c
        ../http_handler/http_handler.c
        ../socket_factory/socket_factory.c
)

add_executable(ashti web_server.c ${SOURCES})

target_link_libraries(
        ashti
        PUBLIC
        c_subprocess.a
        file_fetcher.a
        http_handler.a
        socket_factory.a
)

set_target_properties(ashti PROPERTIES LINKER_LANGUAGE C)
