/**
 * @file thread_list.c
 * @bried Implementation of thread_list declarations.
 *
 * @author Adrien Ghosn
 */
#include  <stdlib.h>
#include "thread_list.h"

l1_thread_info* thread_list_remove(l1_thread_list* list, l1_thread_info* thread) {
  if (!list || !thread) {
    return NULL;
  }
  l1_thread_info* prev = thread->prev;
  l1_thread_info* next = thread->next;
  /* We are the only ones in the list. */
  if (!prev && !next) {
     list->head = list->tail = NULL;
     goto rm_prolog;
  }
  if (!prev) {
    list->head = next;
    next->prev = NULL;
    goto rm_prolog;
  }
  if (!next) {
    list->tail = prev; 
    prev->next = NULL;
    goto rm_prolog;
  }
  prev->next = next;
  next->prev = prev;
rm_prolog:
  list->size--;
  thread->prev = thread->next = NULL;
  return thread;
}


l1_thread_info* thread_list_find(l1_thread_list* list, l1_tid tid) {
  for (l1_thread_info* cur = list->head; cur != NULL; cur = cur->next) {
    if (cur->id == tid) {
      return cur;
    }
  }
  return NULL; 
}

l1_thread_info* thread_list_rotate(l1_thread_list* list) {
  if (thread_list_is_empty(list)) {
    return NULL;
  }
  l1_thread_info* elem = thread_list_pop(list);
  thread_list_add(list, elem);
  return elem;
}


l1_thread_info* thread_list_pop(l1_thread_list* list) {
  if (list == NULL) {
    return NULL;
  }
  return thread_list_remove(list, list->head);
}

void thread_list_add(l1_thread_list* list, l1_thread_info* thread) {
  if (list == NULL || thread == NULL) {
    return; 
  }
  if (thread->prev != NULL || thread->next != NULL) {
    fprintf(stderr, "Error: thread was not properly cleaned after being removed\n");
    exit(-1);
  }
  if (thread_list_is_empty(list)) {
    list->head = list->tail = thread;
    goto add_prolog;
  }

  list->tail->next = thread;
  thread->prev = list->tail;
  list->tail = thread;
add_prolog:
  thread->next = NULL;
  list->size++;
}

void thread_list_prepend(l1_thread_list* list, l1_thread_info* thread) {
  if (list == NULL || thread == NULL) {
    return;
  }
 if (thread->prev != NULL || thread->next != NULL) {
    fprintf(stderr, "Error: thread was not properly cleaned after being removed\n");
    exit(-1);
  }
  if (thread_list_is_empty(list)) {
    list->head = list->tail = thread;
    goto add_prolog;
  }

  list->head->prev = thread;
  thread->next = list->head;
  list->head = thread;
add_prolog:
  thread->prev = NULL;
  list->size++;
}

bool thread_list_is_empty(l1_thread_list* list) {
  if (list == NULL) {
    return false;
  }
  return list->size == 0;
}

l1_thread_info* thread_list_min_total_time(l1_thread_list* list) {
  if (thread_list_is_empty(list)) return NULL;

  l1_thread_info *temp = list->head;
  l1_tid min_id = list->tail->id;
  l1_time min_total_time = list->tail->total_time;

  while(temp != list->tail) {
    if (temp->total_time < min_total_time) {
      min_total_time = temp->total_time;
      min_id = temp->id;
    }

    temp = temp->next;
  }

  return thread_list_find(list, min_id);
}

void thread_list_boost_priority(l1_thread_list* list) {
  if (thread_list_is_empty(list)) return;

  l1_thread_info *temp = list->head;

  while (temp) {
    if (!temp->got_scheduled && temp->priority_level != TOP_PRIORITY) {
      l1_priority_increase(&temp->priority_level);
      l1_time_init(&temp->total_time);
    }

    temp = temp->next;
  }
}

l1_thread_info* thread_list_select_highest_priority(l1_thread_list* list) {
  if (thread_list_is_empty(list)) return NULL;
  
  l1_thread_info *temp = list->head;
  l1_thread_info *scheduled_thread = list->head;
  l1_priority highest_priority = list->head->priority_level;

  while (temp) {
    if (temp->priority_level == TOP_PRIORITY) return temp;

    if (temp->priority_level > highest_priority) {
      highest_priority = temp->priority_level;
      scheduled_thread = temp;
    }

    temp = temp->next;
  }
  
  return scheduled_thread;
}
