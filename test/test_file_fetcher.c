#include <check.h>
#include <socket_factory_mgr.h>
#include <file_fetcher.h>

START_TEST(test_slash)
{
    char dirname[] = "www_root";
    char src_str[] = "GET / /1.1\nHost: localhost\n\n";
    file_return_t * f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    printf("FULL PATH: %s\n", f_info->root_dir);
    http_parse(f_info, src_str);
    file_finder(f_info);

    //Check source type is www
    ck_assert_int_eq(f_info->source_type, 0);
    //Check status 200 OK
    ck_assert_int_eq(f_info->status, 200);

    f_ret_destroy(f_info);
}END_TEST

START_TEST(test_index)
{
    char dirname[] = "www_root";
    char src_str[] = "GET /index.html /1.1\nHost: localhost\n\n";
    file_return_t * f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    http_parse(f_info, src_str);
    file_finder(f_info);

    //Check source type is www
    ck_assert_int_eq(f_info->source_type, 0);
    //Check status 200 OK
    ck_assert_int_eq(f_info->status, 200);

    f_ret_destroy(f_info);
}END_TEST

START_TEST(test_invalid_index)
{
    char dirname[] = "www_root";
    char src_str[] = "GET /../index.html /1.1\nHost: localhost\n\n";
    file_return_t * f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    http_parse(f_info, src_str);
    file_finder(f_info);

    //Check file from GET request read correctly
    ck_assert_str_eq(f_info->target_file, "/../index.html");
    //Check status is 404 Not Found
    ck_assert_int_eq(f_info->status, 404);

    f_ret_destroy(f_info);
}END_TEST

START_TEST(test_back_forward)
{
    char dirname[] = "www_root";
    char src_str[] = "GET /../www/../www/index.html /1.1\nHost: localhost\n\n";
    file_return_t * f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    http_parse(f_info, src_str);
    file_finder(f_info);

    //Check source type is www
    ck_assert_int_eq(f_info->source_type, 0);
    //Check status 200 OK
    ck_assert_int_eq(f_info->status, 200);

    f_ret_destroy(f_info);
}END_TEST

START_TEST(test_cgi_exec)
{
    char dirname[] = "www_root";
    char src_str[] = "GET /cgi-bin/hello_world /1.1\nHost: localhost\n\n";
    file_return_t * f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    http_parse(f_info, src_str);
    file_finder(f_info);
    
    //Check source type is cgi
    ck_assert_int_eq(f_info->source_type, 1);
    //Check status 200 OK
    ck_assert_int_eq(f_info->status, 200);
    //Check read set
    ck_assert_int_eq(f_info->read, 1);
    //Check execute set
    ck_assert_int_eq(f_info->execute, 1);

   f_ret_destroy(f_info);
}END_TEST

START_TEST(test_cgi_no_exec)
{
    char dirname[] = "www_root";
    char src_str[] = "GET /cgi-bin/hello_world_no_exec /1.1\nHost: localhost\n\n";
    file_return_t * f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    http_parse(f_info, src_str);
    file_finder(f_info);

    //Check source type is cgi
    ck_assert_int_eq(f_info->source_type, 1);
    //Check status 403 FORBIDDEN
    ck_assert_int_eq(f_info->status, 403);
    //Check read set
    ck_assert_int_eq(f_info->read, 1);
    //Check execute not set
    ck_assert_int_eq(f_info->execute, 0);

    f_ret_destroy(f_info);
}END_TEST

START_TEST(test_cgi_bad_dir)
{
    char dirname[] = "www_root";
    char src_str[] = "GET /../cgi-bin/hello_world /1.1\nHost: localhost\n\n";
    file_return_t * f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    http_parse(f_info, src_str);
    file_finder(f_info);

    //Check source type is cgi
    ck_assert_int_eq(f_info->source_type, 1);
    //Check status 404 Not Found
    ck_assert_int_eq(f_info->status, 404);
    //Check read not set
    ck_assert_int_eq(f_info->read, 0);
    //Check execute not set
    ck_assert_int_eq(f_info->execute, 0);

    f_ret_destroy(f_info);
}END_TEST

START_TEST(test_multi_req)
{
    char dirname[] = "www_root";
    char src_str1[] = "GET / /1.1\nHost: localhost\n\n";
    char src_str2[] = "GET /cgi-bin/hello_world /1.1\nHost: localhost\n\n";

    file_return_t * f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    http_parse(f_info, src_str1);
    file_finder(f_info);

    //Check source type is www
    ck_assert_int_eq(f_info->source_type, 0);
    //Check status 200 OK
    ck_assert_int_eq(f_info->status, 200);

    f_ret_destroy(f_info);

    f_info = calloc(1, sizeof(file_return_t));
    f_info->root_dir = get_full_path(dirname);
    http_parse(f_info, src_str2);
    file_finder(f_info);

    //Check source type is cgi
    ck_assert_int_eq(f_info->source_type, 1);
    //Check status 200 OK
    ck_assert_int_eq(f_info->status, 200);
    //Check read set
    ck_assert_int_eq(f_info->read, 1);
    //Check execute set
    ck_assert_int_eq(f_info->execute, 1);

    f_ret_destroy(f_info);
}END_TEST


static TFun creation_default_test[] = {
    test_slash,
    test_index,
    test_invalid_index,
    test_back_forward,
    test_cgi_exec,
    test_cgi_no_exec,
    test_cgi_bad_dir,
    test_multi_req,
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

Suite * file_fetcher_test_suite(void)
{
    // suite creation
    Suite * suite = suite_create("\n  File Fetcher Tester");
    TFun * test_list;
    TCase * test_cases;

	// add tests to suite
    test_list = creation_default_test;
    test_cases = tcase_create(" File Fetching Testing");
    add_tests(test_cases, test_list);

    // use this to control timesouts when using sleeps
    //    tcase_set_timeout(test_cases, 0);

    suite_add_tcase(suite, test_cases);


    return suite;
}

/*** end of file ***/

