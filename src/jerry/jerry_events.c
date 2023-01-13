
#include "jerry_events.h"

#include <zephyr/zephyr.h>

void
jz_event_queue_init( jz_event_queue *queue,
                    jerry_value_t *queue_buffer,
                    unsigned int buffer_size)
{
    queue->start = queue_buffer;
    queue->head = queue_buffer;
    queue->tail = queue_buffer;
    queue->end = queue_buffer + buffer_size;
}

int
jz_event_queue_num_entries (jz_event_queue *queue)
{
    int tNumEntries = 0;
    if (queue->head > queue->tail)
    {
        tNumEntries = queue->head - queue->tail;
    } else 
    if (queue->head < queue->tail)
    {
        tNumEntries = queue->end - queue->head + queue->tail - queue->start;
    }
    return tNumEntries;
}

int
jz_event_queue_free_space (jz_event_queue *queue)
{
    int tFreeSpace = queue->end - queue->start;
    if (queue->head > queue->tail)
    {
        tFreeSpace = queue->end - queue->head + queue->tail - queue->start;
    } else 
    if (queue->head < queue->tail)
    {
        tFreeSpace = queue->tail - queue->head;
    }
    return tFreeSpace;
}

void
jz_event_queue_push (jz_event_queue *queue, jerry_value_t new_value)
{
    *(queue->head) = new_value;
    queue->head++;
    if(queue->head == queue->end)
    {
        queue->head = queue->start;
    }
    if(queue->head == queue->tail)
    {
        //fatal: overflow
        printk("jz_event_queue_push fatal error: overflow\n");
    }
}

jerry_value_t
jz_event_queue_pop (jz_event_queue *queue)
{
    if(queue->head == queue->tail)
    {
        //fatal: underflow
        printk("jz_event_queue_pop fatal error: underflow\n");
    }
    jerry_value_t tLastValue = *(queue->tail);
    queue->tail++;
    if(queue->tail == queue->end)
    {
        queue->tail = queue->start;
    }
    return tLastValue;
}