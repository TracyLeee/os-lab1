/**
 * @file sched_policy.c
 * @brief Implementations for custom schedulers
 *
 * @author Atri Bhattacharyya, Mark Sutherland
 */
#include "sched_policy.h"
#include "schedule.h"

/** Round robin just returns the oldest thread in RUNNABLE state */
l1_thread_info* l1_round_robin_policy() {
  l1_scheduler_info *scheduler = get_scheduler();
  return thread_list_rotate(&scheduler->thread_arrays[RUNNABLE]);
}

