#include <stdio.h>
#include <stdlib.h>
#include <c_subprocess.h>

static void report_not_null(char * result, const char * test_num)
{
    if (NULL == result)
    {
        printf("[X] %s failure\n", test_num);
    }
    else
    {
        printf("[O] %s success\n", test_num);
        free(result);
    }
    printf("\n------\n");
}

static void report_is_null(char * result, const char * test_num)
{
    if (NULL != result)
    {
        printf("[X] %s failure\n", test_num);
        free(result);
    }
    else
    {
        printf("[O] %s success\n", test_num);
    }
    printf("\n------\n");
}

static void test_one(const char * test_num)
{
    char * arr[] = {
        "ping",
        "-c",
        "3",
        "127.0.0.888",
        NULL
    };

    char * val = c_subprocess(
        arr
    );
    report_is_null(val, test_num);
}

static void test_two(const char * test_num)
{
    char * arr[] = {
        "pin",
        "-c",
        "3",
        "127.0.0.888",
        NULL
    };
    char * val = c_subprocess(
        arr
    );
    report_is_null(val, test_num);
}

static void test_three(const char * test_num)
{
    char * arr[] = {
        "ping",
        "-c",
        "3",
        "127.0.0.8",
        NULL
    };
    char * val = c_subprocess(
        arr
    );
    report_not_null(val, test_num);
}

static void test_four(const char * test_num)
{
    char * arr[] = {
        "../../www_root/cgi-bin/hello_world_no_exec",
        NULL
    };
    char * val = c_subprocess(
        arr
    );
    report_is_null(val, test_num);
}

static void test_five(const char * test_num)
{
    char *arr[] = {
        "../../www_root/cgi-bin/hello_world",
        NULL
    };
    char *val = c_subprocess(
        arr
    );
    report_not_null(val, test_num);
}



int main(void)
{

    test_one("TEST ONE");
    test_two("TEST TWO");
    test_three("TEST THREE");
    test_four("TEST FOUR");
    test_five("TEST FIVE");
}
