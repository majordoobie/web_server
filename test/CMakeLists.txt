include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/test
)

add_executable(
        test_project_target
        test_driver.c
        test_socket_factory.c
        test_file_fetcher.c
        test_http_handler.c
)

set_target_properties(test_project_target PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(
        test_project_target PUBLIC

        # add libs here
        socket_factory.a
        c_subprocess.a
        file_fetcher.a
        http_handler.a

        # these needed for test suite
        check
        m
        rt
        pthread
        subunit
)
