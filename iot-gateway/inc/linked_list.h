#ifndef LINKED_LIST_H__
#define LINKED_LIST_H__

#include <stdio.h>
#include <stdlib.h>

#define ENABLE      1
#define DISABLE     0

typedef struct
{
    int socket_fd;
    int position;
    int port_no;
}socket_info_t;

typedef struct Nodes
{
    socket_info_t socket_info;
    struct Nodes *next;
}Nodes;

typedef struct data_sensor_t
{
    char data[20];
    int status_read;
    struct data_sensor_t *next;
}data_sensor_t;


void push_back(Nodes **ptp, Nodes *value);
void push_data(data_sensor_t **ptp, char *data);
void get_data(data_sensor_t **data, char *buffer);
void pop_data(data_sensor_t **ptp);

int remove_nodes(Nodes **ptp, int pos);
void printf_list(Nodes *list);

#endif