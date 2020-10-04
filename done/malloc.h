/**
 * @file malloc.h
 * @brief Header files for custom allocator
 *
 * @author Atri Bhattacharyya, Ahmad Hazimeh
 */
#pragma once
#include <stddef.h>
#include <stdlib.h>
#include "error.h"

#define ALLOC8R_HEAP_SIZE (1024 * 1024)

/* Week 5: Interface for heap memory allocator */
extern void *(*l1_malloc)(size_t);
extern l1_error (*l1_free)(void *);
extern void (*l1_init)(void);
extern void (*l1_deinit)(void);

/****** Standard libc based allocator *******************/
void *libc_malloc(size_t size);
l1_error libc_free(void *ptr);

