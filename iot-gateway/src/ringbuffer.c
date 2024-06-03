#include "ringbuffer.h"
#include <stdio.h>
#include <stdlib.h>

void ring_buffer_init(ring_buffer_t *new_buffer)
{
    new_buffer->post = 0;
    new_buffer->capacity = 0;
    return;
}

void ring_buffer_set_value(ring_buffer_t *buffer, int value)
{
    buffer->value[buffer->post] = value;
    if (buffer->post == 4) buffer->post = 0;
    else buffer->post++;

    if (buffer->capacity < 5) buffer->capacity++;
    return;
}

int averange_buffer(ring_buffer_t *buffer)
{
    int avr_value = 0;
    if (buffer->capacity < 5)    return -1;
    else{
        for (int i = 0; i < buffer->capacity; i++)
        {
            avr_value += buffer->value[i];
        }

        avr_value = avr_value/5;
    }
    return avr_value;
}
