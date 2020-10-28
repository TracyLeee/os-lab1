/**
 * @file week03.c
 * @brief Outlines in C for student week 3
 *
 * @author Mark Sutherland
 */
#include <stdlib.h>
#include "malloc.h"
#include "schedule.h"
#include "stack.h"
#include "thread.h"
#include "thread_info.h"
#include "priority.h"

/* This is a function that calls a new thread's start_routine and stores the
 * return value in the function's info struct. This function is put on top
 * of the constructed stack for all new threads.
 */
void l1_start(void) {
  l1_thread_info* cur = get_scheduler()->current;
  /* enter execution */
  void* ret = cur->thread_func(cur->thread_func_args); 
  cur->retval = ret; 
  cur->state = ZOMBIE;
  /* Let the scheduler do the cleanup */
  yield(-1); 
}

l1_error l1_thread_create(l1_tid *thread, void *(*start_routine)(void *), void *arg) {
  l1_tid new_tid = get_uniq_tid();
  /* TODO: Allocate l1_thread_info struct for new thread,
   * allocate stack for the thread. */
  l1_thread_info *new_t_info = (l1_thread_info *)libc_malloc(sizeof(l1_thread_info));

  if (!new_t_info) {
    l1_errno = ERRNOMEM;
    fprintf(stderr, "l1_thread_create(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return l1_errno;
  }

  new_t_info->id = new_tid;
  new_t_info->state = RUNNABLE;
  new_t_info->thread_func = start_routine;
  new_t_info->thread_func_args = arg;
  new_t_info->thread_stack = l1_stack_new();

  /* Initialize l1_time and scheduling-related variables */
  new_t_info->priority_level = TOP_PRIORITY;
  new_t_info->got_scheduled = 0;
  new_t_info->total_time = 0;

  /* TODO: Setup stack for new task. At the bottom of the stack is a fake stack 
   * frame for l1_start, as described in the handout. This will allow the 
   * new thread to return to l1_start after it finished working. 
   * Hint: To figure out which registers need to be saved on the stack, and the 
   * order in which they need to be stored, explore the state of the stack in 
   * `switch_asm`. Also, what are the default values required by `l1_start`.
   * Binaries compiled by `gcc` use the System-V ABI which defines which registers
   * are saved by a function's caller and callee. 
   * See https://wiki.osdev.org/System_V_ABI
   * In `switch_asm`, we only save the registers are not already saved.
   */
  l1_stack_push(new_t_info->thread_stack, 0);
  l1_stack_push(new_t_info->thread_stack, (uint64_t)&l1_start);
  l1_stack_push(new_t_info->thread_stack, (uint64_t)new_t_info->thread_stack->base);  // %rbp
  l1_stack_push(new_t_info->thread_stack, 0);  // %r15
  l1_stack_push(new_t_info->thread_stack, 0);  // %r14
  l1_stack_push(new_t_info->thread_stack, 0);  // %r13
  l1_stack_push(new_t_info->thread_stack, 0);  // %r12
  l1_stack_push(new_t_info->thread_stack, 0);  // %rbx

  /* TODO: Add the new task for scheduling */
  add_to_scheduler(new_t_info, RUNNABLE);

  /* Give the user the right thread id */
  *thread = new_tid;

  return SUCCESS;
}

l1_error l1_thread_join(l1_tid target, void **retval) {
  /* TODO: Setup necessary metadata and block yourself */
  l1_scheduler_info* sched_info = get_scheduler();
  l1_thread_info *cur_t_info = sched_info->current;

  if (!thread_list_find(sched_info->thread_arrays+RUNNABLE, target) && 
      !thread_list_find(sched_info->thread_arrays+BLOCKED, target) && 
      !thread_list_find(sched_info->thread_arrays+ZOMBIE, target)) {
    l1_errno = ERRINVAL;
    fprintf(stderr, "l1_thread_join(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return l1_errno;
  }

  if (thread_list_find(sched_info->thread_arrays+DEAD, target)) return SUCCESS;

  cur_t_info->state = BLOCKED;
  cur_t_info->joined_target = target;
  cur_t_info->errno = SUCCESS;
  cur_t_info->join_recv = libc_malloc(sizeof(void *));

  if (!cur_t_info->join_recv) {
    l1_errno = ERRNOMEM;
    fprintf(stderr, "l1_thread_join(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return l1_errno;
  }

  yield(cur_t_info->joined_target);
  
  if (cur_t_info->errno) {
    fprintf(stderr, "l1_thread_join(): errno %d %s\n", cur_t_info->errno, l1_strerror(cur_t_info->errno));
    return cur_t_info->errno;
  }

  cur_t_info->retval = *cur_t_info->join_recv;

  if (retval) *retval = cur_t_info->retval;

  cur_t_info->joined_target = 0;
  cur_t_info->errno = SUCCESS;
  cur_t_info->retval = NULL;

  if (libc_free(cur_t_info->join_recv) != SUCCESS) {
    l1_errno = MAX_ERROR;
    fprintf(stderr, "l1_thread_join(): errno %d %s\n", l1_errno, l1_strerror(l1_errno));
    return l1_errno;
  }

  return SUCCESS;
}
