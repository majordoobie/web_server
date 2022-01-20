/** @file test_driver.c
 *
 * @brief The testing file which brings all the individual
 *        suites containing tests up to run.
 *
 */

#include <check.h>
#include <stdlib.h>
#include <locale.h>

// Extern all the suite names in the directory
extern Suite * socket_factory_test_suite(void);
extern Suite * file_fetcher_test_suite(void);
extern Suite * http_handler_test_suite(void);


int main(int argc, char ** argv)
{
    // Suppress unused parameter warnings
    (void)argv[argc];

    // create test suite runner
    SRunner *sr = srunner_create(NULL);

	// Add extern line here to add to runner
	srunner_add_suite(sr, socket_factory_test_suite());
    srunner_add_suite(sr, file_fetcher_test_suite());
    srunner_add_suite(sr, http_handler_test_suite());

    // run the test suites
    srunner_run_all(sr, CK_VERBOSE);

    // report the test failed status
    int tests_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (tests_failed==0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*** end of file ***/

