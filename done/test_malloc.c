/**
 * @file test_malloc.c
 * @brief Unit-test suite for week 5
 *
 * @author Atri Bhattacharyya, Ahmad Hazimeh
 */
#include <check.h>
#include <stdio.h>
#include <stdbool.h>
#include "malloc.h"

void *(*l1_malloc)(size_t) = libc_malloc;
l1_error (*l1_free)(void *) = libc_free;
void (*l1_init)(void) = NULL;
void (*l1_deinit)(void) = NULL;

START_TEST(chunk_malloc_test_1) {
  /* This will test the chunk allocator */
  l1_init = l1_chunk_init;
  l1_deinit = l1_chunk_deinit;
  l1_malloc = l1_chunk_malloc;
  l1_free = l1_chunk_free;

  l1_init();
  void *ptr = l1_malloc(0);
  ck_assert_msg(ptr == NULL, "An allocation of size 0 should fail.");
  l1_deinit();
}
END_TEST

START_TEST(chunk_malloc_test_seg_fault) {
  /* This will test the chunk allocator */
  l1_init = l1_chunk_init;
  l1_deinit = l1_chunk_deinit;
  l1_malloc = l1_chunk_malloc;
  l1_free = l1_chunk_free;

  size_t K = 1024;

  l1_init();
  void *p1 = l1_malloc(1);
  void *p2 = l1_malloc(8*K);
  void *p3 = l1_malloc(4*K+1);
  l1_free(p2);
  void *p4 = l1_malloc(2*K);
  l1_free(p3);
  l1_free(p1);
  l1_free(p4);
  l1_deinit();
}
END_TEST

/* Test malloc with 0 size returns NULL */
START_TEST(list_malloc_test1) {
  /* This will test the listoc8r allocator */
  l1_init = l1_listoc8r_init;
  l1_deinit = l1_listoc8r_deinit;
  l1_malloc = l1_listoc8r_malloc;
  l1_free = l1_listoc8r_free;

  l1_init();
  ck_assert_msg(l1_malloc(0) == NULL, "A malloc of size 0 should return NULL.");
  l1_deinit();
}
END_TEST

START_TEST(list_malloc_test_dummy) {
  /* This will test the listoc8r allocator */
  l1_init = l1_listoc8r_init;
  l1_deinit = l1_listoc8r_deinit;
  l1_malloc = l1_listoc8r_malloc;
  l1_free = l1_listoc8r_free;

  l1_init();
  l1_malloc(1);
  l1_malloc(128);
  l1_deinit();
}
END_TEST

int main(int argc, char **argv)
{
  Suite* s = suite_create("Threading lab");
  TCase *tc1 = tcase_create("basic tests"); 
  suite_add_tcase(s,tc1);

  tcase_add_test(tc1, chunk_malloc_test_1);
  tcase_add_test(tc1, list_malloc_test1);

  /* Add more tests of your own */
  tcase_add_test(tc1, list_malloc_test_dummy);
  tcase_add_test(tc1, chunk_malloc_test_seg_fault);

  SRunner *sr = srunner_create(s); 
  srunner_run_all(sr, CK_VERBOSE); 

  int number_failed = srunner_ntests_failed(sr); 
  srunner_free(sr); 

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
