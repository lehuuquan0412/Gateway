#ifndef RINGBUFFER_H__
#define RINGBUFFER_H__

#include <stdio.h>

typedef struct
{
    int value[5];
    int post;
    int capacity;
}ring_buffer_t;

void ring_buffer_init(ring_buffer_t *new_buffer);
void ring_buffer_set_value(ring_buffer_t *buffer, int value);
int averange_buffer(ring_buffer_t *buffer);

#endif