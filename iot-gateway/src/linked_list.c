#include "linked_list.h"
#include <string.h>


void push_back(Nodes **ptp, Nodes *value)
{
    if (*ptp == NULL)   *ptp = value;
    else{
        Nodes *temp = *ptp;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = value;
    }
    return;
}

void push_data(data_sensor_t **ptp, char *data)
{
    data_sensor_t *new_data = (data_sensor_t *)malloc(sizeof(data_sensor_t));
    strcpy(new_data->data, data);
    new_data->status_read = DISABLE;
    new_data->next = NULL;

    data_sensor_t *temp = *ptp;

    if (*ptp == NULL)   *ptp = new_data;
    else {
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = new_data;
    }
    return;
}

void get_data(data_sensor_t **data, char *buffer)
{
    data_sensor_t *temp = *data;
    temp->next = NULL;
    strcpy(buffer, temp->data);
    return;
}

void pop_data(data_sensor_t **ptp)
{
    data_sensor_t *temp = *ptp;
    (*ptp) = (*ptp)->next;
    free(temp);
    return;
}

int remove_nodes(Nodes **ptp, int pos)
{
    Nodes *temp = *ptp;
    int i = 0;

    if (temp == NULL)   return -1;

    if (temp->socket_info.position == pos)
    {
        (*ptp) = (*ptp)->next;
        temp->next = NULL;
        //free(temp);
        return i;
    }

    while(temp->next != NULL)
    {
        if (temp->next->socket_info.position == pos)
        {
            //Nodes *delete_node = temp->next;
            temp->next = temp->next->next;
            return i+1;
        }
        temp = temp->next;
        i++;
    }
    return -1;
}

void printf_list(Nodes *list)
{
    Nodes *temp = list;
    while(temp != NULL)
    {
        printf("Id: %d\n", temp->socket_info.port_no);
        temp = temp->next;
    }

    return;
}
