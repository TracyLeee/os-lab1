/**
 * @file malloc.c
 * @brief Implementations of custom allocators
 *
 * @author Atri Bhattacharyya, Ahmad Hazimeh
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "malloc.h"
#include "error.h"

/*********************** Standard GLIBC malloc ***********/
void *libc_malloc(size_t size) {
  return malloc(size);
}

l1_error libc_free(void *ptr) {
  free(ptr);

  return SUCCESS;
}
/**********************************************************/

