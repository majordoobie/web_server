#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <http_handler.h>
#include "socket_factory_mgr.h"

START_TEST(test_valid_request)
{
    char src_str[] = "GET /index.html HTTP/1.1\nHost: localhost\n\n"; 
    file_return_t * f_info = calloc(1, sizeof(file_return_t));

    http_parse(f_info, src_str);

    ck_assert_str_eq(f_info->target_file, "/index.html");

    f_ret_destroy(f_info);
}END_TEST

START_TEST(test_slash_request)
{
    char src_str[] = "GET / HTTP/1.1\nHost: localhost\n\n"; 
    file_return_t * f_info = calloc(1, sizeof(file_return_t));

    http_parse(f_info, src_str);

    ck_assert_str_eq(f_info->target_file, "/index.html");

    f_ret_destroy(f_info);
}END_TEST


START_TEST(test_no_path_request)
{
    char src_str[] = "GET HTTP/1.1\nHost: localhost\n\n"; 
    file_return_t * f_info = calloc(1, sizeof(file_return_t));

    http_parse(f_info, src_str);

    ck_assert_ptr_eq(f_info->target_file, NULL);

    f_ret_destroy(f_info);
}END_TEST

START_TEST(test_bad_path_request)
{
    char src_str[] = "GET blue HTTP/1.1\nHost: localhost\n\n"; 
    file_return_t * f_info = calloc(1, sizeof(file_return_t));

    http_parse(f_info, src_str);

    ck_assert_ptr_eq(f_info->target_file, NULL);
    ck_assert_str_eq(f_info->err_msg, "400 Bad Request");
    ck_assert_int_eq(f_info->status, 400);

    f_ret_destroy(f_info);
}END_TEST


START_TEST(test_BadMethod_request)
{
    char src_str[] = "POST /index.html HTTP/1.1\nHost: localhost\n\n";
    file_return_t * f_info = calloc(1, sizeof(file_return_t));

    http_parse(f_info, src_str);

    ck_assert_int_eq(f_info->status, 405);
    ck_assert_str_eq(f_info->err_msg, "405 Method Not Allowed - POST");

    f_ret_destroy(f_info);
}END_TEST



static TFun creation_default_test[] = {
    test_valid_request,
    test_slash_request,
    test_no_path_request,
    test_bad_path_request,
    test_BadMethod_request,
    NULL
};


static void add_tests(TCase * test_cases, TFun * test_functions)
{
    while (* test_functions) {
        // add the test from the core_tests array to the t-case
        tcase_add_test(test_cases, * test_functions);
        test_functions++;
    }
}

Suite * http_handler_test_suite(void)
{
    // suite creation
    Suite * suite = suite_create("\n  HTTP Handler Tester");
    TFun * test_list;
    TCase * test_cases;

	// add tests to suite
    test_list = creation_default_test;
    test_cases = tcase_create(" HTTP Handler Testing");
    add_tests(test_cases, test_list);

    // use this to control timesouts when using sleeps
    //    tcase_set_timeout(test_cases, 0);

    suite_add_tcase(suite, test_cases);


    return suite;
}

/*** end of file ***/

