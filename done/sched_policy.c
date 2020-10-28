/**
 * @file sched_policy.c
 * @brief Implementations for custom schedulers
 *
 * @author Atri Bhattacharyya, Mark Sutherland
 */
#include <stdio.h>
#include <stdlib.h>
#include "sched_policy.h"
#include "schedule.h"

/** Round robin just returns the oldest thread in RUNNABLE state */
l1_thread_info* l1_round_robin_policy(l1_thread_info* prev, l1_thread_info* next) {
  if (next != NULL) {
    return next;
  }
  l1_scheduler_info *scheduler = get_scheduler();
  return thread_list_rotate(&scheduler->thread_arrays[RUNNABLE]);
}

/** Schedules the thread with the smallest amount of cycles so far */
l1_thread_info* l1_smallest_cycles_policy(l1_thread_info* prev, l1_thread_info* next) {
  if (next) return next;

  l1_scheduler_info *scheduler = get_scheduler();

  return thread_list_min_total_time(&scheduler->thread_arrays[RUNNABLE]);
}

/** Schedules threads according to mlfq policy */
l1_thread_info* l1_mlfq_policy(l1_thread_info* prev, l1_thread_info* next) {
  l1_scheduler_info *scheduler = get_scheduler();

  /* Boost any thread applicable in RUNNABLE list*/
  if (!scheduler->sched_ticks) {
    thread_list_boost_priority(&scheduler->thread_arrays[RUNNABLE]);
  }

  /* Demote prev: time slice used up or threshold time exceeded(unless lowest priority)*/
  l1_time prev_slice_time;
  l1_time_diff(&prev_slice_time, prev->slice_end, prev->slice_start);

  if (l1_time_is_smaller(prev_slice_time, l1_priority_slice_size(prev->priority_level)) == 0 && 
      prev->priority_level != LOWEST_PRIORITY) {
    l1_priority_decrease(&prev->priority_level);
    prev->got_scheduled = 0;
    l1_time_init(&prev->total_time);
  } else if (l1_time_is_smaller(prev->total_time, TIME_PRIORITY_THRESHOLD) == 0 && 
             prev->priority_level != LOWEST_PRIORITY) {
    l1_priority_decrease(&prev->priority_level);
    prev->got_scheduled = 0;
    l1_time_init(&prev->total_time);
  }

  return thread_list_select_highest_priority(&scheduler->thread_arrays[RUNNABLE]);

  // /* Pop the head of the non-empty queue with highest priority */
  // l1_thread_info* scheduled_thread = thread_list_pop_highest_priority(&scheduler->thread_arrays[RUNNABLE]);

  // if (scheduled_thread) {
  //   thread_list_add(&scheduler->thread_arrays[RUNNABLE], scheduled_thread);
  // }

  // return scheduled_thread;
}

