#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sqlite3.h>
#include <semaphore.h>

#include "functions.h"
#include "linked_list.h"
#include "database.h"
#include "ringbuffer.h"

#define SENSOR_MAP_FILE             "sensor_map.csv"
#define LOG_NAME                    "gateway.log"
#define DB_NAME                     "gateway.db"

#define MAX_SENSOR              50

int server_socket_fd;
int fifo_fd[2];

struct pollfd *fds;

Nodes *list_sensor = NULL;
data_sensor_t *sensor_data_list;

int current_sensor = 0;

pthread_t data_manager_thread_id, storage_data_manager_thread_id;

sem_t *share_data_sem;
sem_t *share_data_sem_2;

room_info_t *map_room_sensor = NULL;

static void *data_manager_thread(void *args);
static void *storage_data_management(void *args);

void server_initial(int port_no)
{
    int ret_value;
    int len = 1;
    struct sockaddr_in server_socket;

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket_fd == 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&server_socket, 0, sizeof(server_socket));

    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = INADDR_ANY;
    server_socket.sin_port = htons(port_no);

    if (bind(server_socket_fd, (struct sockaddr*)(&server_socket), sizeof(server_socket)) == -1)
    {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &len, sizeof(int)) == -1)
    {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_fd, MAX_SENSOR) ==  -1)
    {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    share_data_sem = (sem_t*)malloc(sizeof(sem_t));
    sem_init(share_data_sem, 0, 0);

    share_data_sem_2 = (sem_t*)malloc(sizeof(sem_t));
    sem_init(share_data_sem_2, 0, 0);

    ret_value = pthread_create(&data_manager_thread_id, NULL, &data_manager_thread, NULL);
    if (ret_value == -1)
    {
        perror("pthread_create()");
        exit(EXIT_FAILURE);
    }

    ret_value = pthread_create(&storage_data_manager_thread_id, NULL, &storage_data_management, NULL);
    if (ret_value == -1)
    {
        perror("pthread_create()");
        exit(EXIT_FAILURE);
    }

    manage_connection();

    return;
}

int make_fifo_file(void)
{
    return pipe(fifo_fd);
}

void reader_fifo(void)
{
    close(fifo_fd[1]);
}

void writer_fifo(void)
{
    close(fifo_fd[0]);
}

void manage_connection(void)
{
    int poll_ret, len;
    int ret_val;
    char id_first[5];
    char buffer_log[100];
    char recieve_buff[100];

    struct sockaddr_in sensor_sock;
    
    fds = (struct pollfd*)malloc(sizeof(struct pollfd));
    
    fds[0].fd = server_socket_fd;
    fds[0].events = POLLIN;
    while ((poll_ret = poll(fds, current_sensor + 1, -1))||(current_sensor))
    {
        if (fds[0].revents & POLLIN)
        {
            Nodes *new_connection = (Nodes *)malloc(sizeof(Nodes));
            new_connection->next = NULL;
            memset(buffer_log, 0, sizeof(buffer_log));
            memset(id_first, '\0', sizeof(id_first));
            len = sizeof(sensor_sock);
            new_connection->socket_info.socket_fd = accept(server_socket_fd, (struct sockaddr *)(&sensor_sock), (socklen_t*)&len);
            ret_val = read(new_connection->socket_info.socket_fd, id_first, sizeof(id_first));
            
            new_connection->socket_info.port_no = atoi(id_first);
            current_sensor += 1;
            new_connection->socket_info.position = current_sensor;
            fds = (struct pollfd*)realloc(fds, sizeof(struct pollfd)*(current_sensor+1));
            fds[current_sensor].fd = new_connection->socket_info.socket_fd;
            fds[current_sensor].events = POLLIN | POLLHUP;
            
            push_back(&list_sensor, new_connection);
            
            poll_ret--;
            sprintf(buffer_log, "A sensor node with ID %d has opened a new connection", new_connection->socket_info.port_no);
            ret_val = write(fifo_fd[1], buffer_log, strlen(buffer_log));       
        }

        for (int i = 0; (i < (current_sensor + 1))&&(poll_ret > 0); i++)
        {
            if (fds[i].revents & POLLIN)
            {
                memset(recieve_buff, '\0', sizeof(recieve_buff));
                int num_read = read(fds[i].fd, recieve_buff, sizeof(recieve_buff));
                
                if (num_read == -1)     printf("Read failure!!!");
                else{
                    if (strcmp(recieve_buff, "exit") == 0)
                    {
                        // close connection and delete
                        close(fds[i].fd);
                        remove_nodes(&list_sensor, i);
                        int id;
                        memset(buffer_log, 0, sizeof(buffer_log));
                        sprintf(buffer_log, "The sensor node with Id %d has closed the connection", id);
                        ret_val = write(fifo_fd[1], buffer_log, strlen(buffer_log));
                    }else{
                        push_data(&sensor_data_list, recieve_buff);
                        sem_post(share_data_sem);
                        poll_ret--;
                    }
                }
            } 
        }
    }
}

int get_num_of_room(char *buffer)
{
    int len = strlen(buffer);
    int ret_val = 0;

    for (int i = 0; i < len; i++)
    {
        if (buffer[i] == ',')   ret_val++;
    }
    return ret_val;
}

int get_room_with_id(room_info_t *list_of_room, int total_room, int id)
{
    for (int i = 0; i < total_room; i++)
    {
        if (list_of_room[i].id_sensor == id)
        {
            return i;
        }
    }

    return -1;
}

room_info_t *get_sensor_map(char *buffer, room_info_t *map_sensor)
{
    int num_of_room = get_num_of_room(buffer);
    char *p;
    if (num_of_room == 0)   return map_sensor;

    map_sensor = (room_info_t*)realloc(map_sensor, num_of_room*sizeof(room_info_t));

    p = strtok(buffer, ",");
    map_sensor[0].room_no = atoi(p);
    p = strtok(NULL, "\n");
    
    if (p == NULL)  return NULL;
    else map_sensor[0].id_sensor = atoi(p);
    ring_buffer_init(&map_sensor[0].arv_buffer);

    for (int i = 1; i < num_of_room; i++)
    {
        p = strtok(NULL, ",");
        if (p == NULL)  return NULL;
        else map_sensor[i].room_no = atoi(p);

        if (p == NULL)  return NULL;
        else map_sensor[i].id_sensor = atoi(p);
        ring_buffer_init(&map_sensor[i].arv_buffer);
    }

    return map_sensor;
}

static void *data_manager_thread(void *args)
{
    int sensor_map_fd, sensor_id, sensor_value;
    int room_number = -1;
    int num_of_room = 0;
    int ret_value;
    char buffer[1024];
    char data_mess[10];
    sensor_map_fd = open(SENSOR_MAP_FILE, O_RDONLY , 0667);
    memset(buffer, 0, sizeof(buffer));
    int num_read = read(sensor_map_fd, buffer, sizeof(buffer));
    num_of_room = get_num_of_room(buffer);
    
    map_room_sensor = get_sensor_map(buffer, map_room_sensor);
    if (map_room_sensor == NULL)
    {
        perror("get_sensor_map()");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        sem_wait(share_data_sem);
        while(sensor_data_list->status_read);
        memset(data_mess, 0, sizeof(data_mess));
        get_data(&sensor_data_list, data_mess);

        if (sensor_data_list->status_read == DISABLE)
        {
            sensor_data_list->status_read = ENABLE;
        }else{
            pop_data(&sensor_data_list);
        }
        sem_post(share_data_sem_2);
        
        room_number = -1;
        char buffer_log[100];
        memset(buffer_log, 0, sizeof(buffer_log));
        char *sensor_id_t = strtok(data_mess, "_");
        sensor_id = atoi(sensor_id_t);
        char *sensor_value_t = strtok(NULL, "\0");
    
        room_number = get_room_with_id(map_room_sensor, num_of_room, sensor_id);
        if (room_number == -1)      
        {
            sprintf(buffer_log, "Recieved sensor node with invalid sensor node ID %d", sensor_id);
            ret_value = write(fifo_fd[1], buffer_log, strlen(buffer_log));
            continue; //Send log with invalid id sensor.
        }
        if (strcmp(sensor_value_t, "exit") == 0)
        {
            // send log Exit with sensor id.
            continue;
        }else{
            sensor_value = atoi(sensor_value_t);
            ring_buffer_set_value(&(map_room_sensor[room_number].arv_buffer), sensor_value);
        } 
        int avr_temp = averange_buffer(&(map_room_sensor[room_number].arv_buffer));

        if (avr_temp == -1)     continue;

        if (avr_temp > 37)
        {
            sprintf(buffer_log, "A sensor node with ID %d reports it's too hot(running avg temperature = %d)", sensor_id, avr_temp);
            ret_value = write(fifo_fd[1], buffer_log, strlen(buffer_log));                
        }else if (avr_temp < 20)
        {
            sprintf(buffer_log, "A sensor node with ID %d reports it's too cold(running avg temperature = %d)", sensor_id, avr_temp);
            ret_value = write(fifo_fd[1], buffer_log, strlen(buffer_log));
        }
    }
}

void log_managament(void)
{
    int ret_val, log_fd;
    int no_log = 0;
    char buffer_log[100];
    char buffer_write_log[100];
    time_t curtime = 1;
    char time_buffer[50];

    
    while (1)
    {
        memset(buffer_log, 0, sizeof(buffer_log));
        ret_val = read(fifo_fd[0], buffer_log, sizeof(buffer_log));
    
        if (ret_val == -1)  continue;
        
        log_fd = open(LOG_NAME, O_APPEND | O_WRONLY | O_CREAT, 0777);
        no_log++;
        memset(buffer_write_log, '\0', sizeof(buffer_log));
        memset(time_buffer, '\0', sizeof(time_buffer));
        strcpy(time_buffer, ctime(&curtime));
        sprintf(buffer_write_log, "[%d] %s %s\n", no_log, time_buffer, buffer_log);
        ret_val = write(log_fd, buffer_write_log, strlen(buffer_write_log));
        close(log_fd);
    }
}

void exit_handle(void)
{

}

static void *storage_data_management(void *args)
{
    int id_sensor;
    char *data_sensor;
    char data_mess[10];
    int num_write;

    char log_buffer_db[] = "Unable to connect SQL server\n";

    if (database_initialize(DB_NAME) == -1)
    {
        num_write = write(fifo_fd[1], log_buffer_db, strlen(log_buffer_db));
        pthread_cancel(pthread_self());
    }

    while(1)
    {
        sem_wait(share_data_sem_2);
        memset(data_mess, 0, sizeof(data_mess));
        get_data(&sensor_data_list, data_mess);

        if (sensor_data_list->status_read == DISABLE)
        {
            sensor_data_list->status_read = ENABLE;
        }else{
            pop_data(&sensor_data_list);
        }
        char *id_sensor_num = strtok(data_mess, "_");
        id_sensor = atoi(id_sensor_num);

        data_sensor = strtok(NULL, "\0");
        if (data_sensor[strlen(data_sensor) - 1] == '\n')
        {
            data_sensor[strlen(data_sensor) - 1] = '\0';
        }
        if (insert_value(id_sensor, data_sensor) == -1)
        {
            char log_lost_connect[] = "Connect to SQL server is lost\n";
            num_write = write(fifo_fd[1], log_lost_connect, strlen(log_lost_connect));
        }
    }
}
