#ifndef _JERRY_EVENTS_H_
#define _JERRY_EVENTS_H_

#include "jerryscript.h"

#ifndef CONFIG_JZ_EVENT_QUEUE_SIZE
#define CONFIG_JZ_EVENT_QUEUE_SIZE 10
#endif

typedef struct jz_event_queue_st {
    jerry_value_t *start;
    jerry_value_t *head;
    jerry_value_t *tail;
    jerry_value_t *end;
} jz_event_queue;

void
jz_event_queue_init( jz_event_queue *queue,
                    jerry_value_t *queue_buffer,
                    unsigned int buffer_size);

int
jz_event_queue_num_entries (jz_event_queue *queue);

int
jz_event_queue_free_space (jz_event_queue *queue);

void
jz_event_queue_push (jz_event_queue *queue, jerry_value_t new_value);

jerry_value_t
jz_event_queue_pop (jz_event_queue *queue);

#endif //_JERRY_EVENTS_H_