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
#include <math.h>
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

/*********************** Chunk malloc *********************/

char (*l1_chunk_arena)[CHUNK_SIZE];
l1_chunk_desc_t *l1_chunk_meta;
max_align_t l1_region_magic;

void l1_chunk_init(void)
{
  /* Allocate chunk arena and metadata */
  l1_chunk_arena = malloc(CHUNK_ARENA_LENGTH * CHUNK_SIZE);

  /* TODO: Allocate space for metadata */ 
  l1_chunk_meta = (l1_chunk_desc_t *)malloc(CHUNK_ARENA_LENGTH * sizeof(char));

  if ((l1_chunk_arena == NULL) || (l1_chunk_meta == NULL)) {
    printf("Unable to allocate %d bytes for the chunk allocator\n", ALLOC8R_HEAP_SIZE);
    exit(1);
  }

  /* Generate random chunk magic */
  srand(time(NULL));
  for(unsigned i = 0; i < sizeof(max_align_t); ++i)
    *(((char *)&l1_region_magic) + i) = rand();
}

void l1_chunk_deinit(void)
{
  /* TODO: Cleanup */
  free(l1_chunk_meta);
  free(l1_chunk_arena);
}

/* Return a feasible index, otherwise return -1 */
int l1_chunk_find_contiguous_chunks(size_t chunk_num) {
  for (int i = 0; i <= CHUNK_ARENA_LENGTH - chunk_num; ++i) {
    int chunk_feasible = 1;
    
    for (int j = i; j < i + chunk_num; ++j) {
      if (IS_CHUNK_TAKEN(j)) {
        chunk_feasible = 0;
        i = j;
        break;
      }
    }

    if (chunk_feasible == 1) return i;
  }

  return -1;
}

void *l1_chunk_malloc(size_t size)
{
  if (size == 0)
    return NULL;

  /* TODO: Implement your function here */
  size_t chunk_num = 1 + ceil((double)size/CHUNK_SIZE);

  if (chunk_num > CHUNK_ARENA_LENGTH) {
    l1_errno = ERRNOMEM;
    fprintf(stderr, "l1_chunk_malloc(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return NULL;
  }

  /* Find and reserve contiguous chunks */
  int start_idx = l1_chunk_find_contiguous_chunks(chunk_num);

  if (start_idx == -1) {
    l1_errno = ERRNOMEM;
    fprintf(stderr, "l1_chunk_malloc(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return NULL;
  }

  for (int i = start_idx; i < start_idx + chunk_num; ++i) {
    SET_CHUNK_TAKEN(i);
  }

  /* Initialize the header */
  l1_region_hdr_t *hdr_ptr = (l1_region_hdr_t *)(l1_chunk_arena + (start_idx * CHUNK_SIZE));

  hdr_ptr->magic0 = l1_region_magic;
  hdr_ptr->size = size;
  hdr_ptr->magic1 = l1_region_magic;

  return (void *)((char *)hdr_ptr + CHUNK_SIZE);
}

l1_error l1_chunk_free(void *ptr)
{
  if (ptr == NULL)
    return SUCCESS;

  /* TODO: Implement your function here */
  /* Verify ptr is on the valid boundary */
  if (((size_t)ptr - (size_t)l1_chunk_arena) % CHUNK_SIZE != 0 || 
      ptr < (void *)(l1_chunk_arena + CHUNK_SIZE) || 
      ptr > (void *)(l1_chunk_arena + CHUNK_ARENA_LENGTH*CHUNK_SIZE)) {
    l1_errno = ERRINVAL;
    fprintf(stderr, "l1_chunk_free(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return ERRINVAL;
  }

  /* Verify the magic numbers in the header */
  l1_region_hdr_t *hdr_ptr = (l1_region_hdr_t *)((char *)ptr - CHUNK_SIZE);
  
  if (memcmp(&hdr_ptr->magic0, &l1_region_magic, sizeof(max_align_t)) != 0 || 
      memcmp(&hdr_ptr->magic1, &l1_region_magic, sizeof(max_align_t)) != 0) {
    l1_errno = ERRINVAL;
    fprintf(stderr, "l1_chunk_free(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return ERRINVAL;
  }

  /* Free contiguous chunks */
  size_t chunk_num = 1 + ceil((double)hdr_ptr->size/CHUNK_SIZE);
  size_t start_idx = ((size_t)hdr_ptr - (size_t)l1_chunk_arena) / CHUNK_SIZE;

  for (size_t i = start_idx; i < start_idx + chunk_num; ++i) {
    SET_CHUNK_FREE(i);
  }

  return SUCCESS;
}
/**********************************************************/

/****************** Free list based malloc ****************/
l1_listoc8r_meta *l1_listoc8r_free_head = NULL;
void *l1_listoc8r_heap = NULL;
max_align_t l1_listoc8r_magic;
size_t meta_size = offsetof(l1_listoc8r_meta, next);

void l1_listoc8r_init() {
  l1_listoc8r_heap = malloc(ALLOC8R_HEAP_SIZE);

  if(l1_listoc8r_heap == NULL) {
    printf("Unable to allocate %d bytes for the listoc8r\n", ALLOC8R_HEAP_SIZE);
    exit(1);
  }

  /* Generate random listoc8r magic */
  srand(time(NULL));
  for(unsigned i = 0; i < sizeof(max_align_t); i++)
    *(((char *)&l1_listoc8r_magic) + i) = rand();

  /* TODO: Complete metadata setup */
  l1_listoc8r_free_head = (l1_listoc8r_meta *)l1_listoc8r_heap;

  l1_listoc8r_free_head->magic0 = l1_listoc8r_magic;
  l1_listoc8r_free_head->capacity = ALLOC8R_HEAP_SIZE - meta_size;
  l1_listoc8r_free_head->magic1 = l1_listoc8r_magic;
  l1_listoc8r_free_head->next = NULL;

  // printf("capacity: %lu\n", l1_listoc8r_free_head->capacity);
}

void l1_listoc8r_deinit() {
  /* TODO: Cleanup */
  free(l1_listoc8r_heap);
}

/* Find a feasible region, otherwise return NULL */
l1_listoc8r_meta *l1_listoc8r_find_feasible_region(size_t size) {
  l1_listoc8r_meta *temp = l1_listoc8r_free_head;

  while (temp) {
    if (temp->capacity >= size) break;

    temp = temp->next;
  }

  return temp;
}

void *l1_listoc8r_malloc(size_t req_size) {
  if(req_size == 0)
    return NULL;

  /* TODO: Implement your function here */
  /* Find a feasible region, otherwise set the errno and return NULL */
  l1_listoc8r_meta *meta_ptr = l1_listoc8r_find_feasible_region(req_size);

  if (!meta_ptr) {
    l1_errno = ERRNOMEM;
    fprintf(stderr, "l1_listoc8r_malloc(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return NULL;
  }

  /* Check if the region should be split */
  size_t aligned_req_size = ceil((double)req_size/sizeof(max_align_t))*sizeof(max_align_t);
  size_t min_reg_size = meta_size + ceil(1.0/sizeof(max_align_t))*sizeof(max_align_t);
  l1_listoc8r_meta *cur = l1_listoc8r_free_head;
  l1_listoc8r_meta *prev;
  
  if (meta_ptr->capacity >= aligned_req_size + min_reg_size) {
    if (l1_listoc8r_free_head == meta_ptr) {
      l1_listoc8r_free_head = (l1_listoc8r_meta *)((char *)meta_ptr + meta_size + aligned_req_size);
      cur = l1_listoc8r_free_head;
    } else {
      while (cur != meta_ptr) {
        prev = cur;
        cur = cur->next;
      }
      
      prev->next = (l1_listoc8r_meta *)((char *)meta_ptr + meta_size + aligned_req_size);
      cur = prev->next;
    }

    cur->magic0 = l1_listoc8r_magic;
    cur->capacity = meta_ptr->capacity - aligned_req_size - meta_size;
    cur->magic1 = l1_listoc8r_magic;
    cur->next = meta_ptr->next;

    meta_ptr->capacity = aligned_req_size;
  } else {
    if (l1_listoc8r_free_head == meta_ptr) {
      l1_listoc8r_free_head = meta_ptr->next;
    } else {
      while (cur != meta_ptr) {
        prev = cur;
        cur = cur->next;
      }

      prev->next = meta_ptr->next;
    }
  }

  // printf("heap: %lu\n", (size_t)l1_listoc8r_heap);
  // printf("return ptr - heap: %lu\n", (size_t)((char *)meta_ptr + meta_size) - (size_t)l1_listoc8r_heap);
  // printf("head - return ptr: %lu\n", (size_t)l1_listoc8r_free_head - (size_t)((char *)meta_ptr + meta_size));
  // printf("aligned size: %lu\n", aligned_req_size);
  // printf("free capacity: %lu\n", cur->capacity);
  // printf("head capacity: %lu\n", l1_listoc8r_free_head->capacity);
  // printf("\n");

  return (void *)((char *)meta_ptr + meta_size);
}

l1_error l1_listoc8r_free(void *ptr) {
  if(ptr == NULL)
    return SUCCESS;

  /* TODO: Implement your function here */
  /* Verify ptr is on the valid boundary */
  if (((size_t)ptr - (size_t)l1_listoc8r_heap) % sizeof(max_align_t) != 0 || 
      ptr < (void *)((char *)l1_listoc8r_heap + meta_size) || 
      ptr > (void *)((char *)l1_listoc8r_heap + ALLOC8R_HEAP_SIZE)) {
    l1_errno = ERRINVAL;
    fprintf(stderr, "l1_listorc8r_free(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return ERRINVAL;
  }

  /* Verify the magic numbers in the header */
  l1_listoc8r_meta *meta_ptr = (l1_listoc8r_meta *)((char *)ptr - meta_size);

  if (memcmp(&meta_ptr->magic0, &l1_listoc8r_magic, sizeof(max_align_t)) != 0 || 
      memcmp(&meta_ptr->magic1, &l1_listoc8r_magic, sizeof(max_align_t)) != 0) {
    l1_errno = ERRINVAL;
    fprintf(stderr, "l1_listorc8r_free(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return ERRINVAL;
  }

  /* Free the region */
  meta_ptr->next = l1_listoc8r_free_head;
  l1_listoc8r_free_head = meta_ptr;

  return SUCCESS;
}
