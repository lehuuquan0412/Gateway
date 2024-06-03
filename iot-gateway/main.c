#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "functions.h"

extern int fifo_fd[2];

int main(int argc, char const *argv[])
{
    int port_no;
    pid_t pid_child;
    char buffer[10];
    if (make_fifo_file() < 0)
    {
        printf("Error fifo\n");
        exit(EXIT_FAILURE);
    }

    pid_child = fork();
    

    if (pid_child == 0)
    {
        writer_fifo();
        if (argc < 2)
        {
            printf("No port provided.\nCommand: ./chat <port number> \n");
            exit(EXIT_FAILURE);
        }else{
            port_no = atoi(argv[1]);
        } 
        server_initial(port_no);
    }else{
        reader_fifo();
        
        log_managament();
    }
    
    return 0;
}
