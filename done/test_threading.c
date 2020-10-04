/**
 * @file test_threading.c
 * @brief Unit-test suite for week 3
 *
 * @author Mark Sutherland
 */
#include <check.h>
#include <stdlib.h> 

int main(int argc, char **argv)
{
    Suite* s = suite_create("Stack Library Tests");
    TCase *tc1 = tcase_create("basic"); 
    suite_add_tcase(s,tc1);

    /* TODO: Write your own tests */

    SRunner *sr = srunner_create(s); 
    srunner_run_all(sr, CK_VERBOSE); 

    int number_failed = srunner_ntests_failed(sr); 
    srunner_free(sr); 

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE; 
}
