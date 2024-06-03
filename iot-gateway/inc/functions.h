#ifndef FUNCTIONS_H__
#define FUNCTIONS_H__


#include <stdio.h>
#include "ringbuffer.h"


typedef struct
{
    int id_sensor;
    int room_no;
    ring_buffer_t arv_buffer;
}room_info_t;

void server_initial(int port_no);
int make_fifo_file(void);
void close_fifo(void);
void manage_connection(void);
int get_num_of_room(char *buffer);
int get_room_with_id(room_info_t *list_of_room, int total_room, int id);

room_info_t *get_sensor_map(char *buffer, room_info_t *map_sensor);
void writer_fifo(void);
void reader_fifo(void);
void log_managament(void);

void exit_handle(void);


#endif