#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>     //  Chứa cấu trúc cần thiết cho socket. 
#include <netinet/in.h>     //  Thư viện chứa các hằng số, cấu trúc khi sử dụng địa chỉ trên internet
#include <arpa/inet.h>
#include <unistd.h>

#define BUFF_SIZE 256
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)
		
/* Chức năng chat */
void chat_func(int server_fd, int id)
{
    int numb_write, numb_read;
    int value;
    char recvbuff[BUFF_SIZE];
    char sendbuff[BUFF_SIZE];

    memset(sendbuff, '\0', sizeof(sendbuff));
    sprintf(sendbuff, "%d", id);
    
    numb_write = write(server_fd, sendbuff, strlen(sendbuff));
    
    while(numb_write < 0)
    {
        numb_write = write(server_fd, sendbuff, strlen(sendbuff));
    }

    sleep(2);

    while (1) {
        value = rand()%60;
        //memset(sendbuff, 0, sizeof(sendbuff));
        sprintf(recvbuff, "%d_%d", id, value);
        printf("message: %s\n", recvbuff);
        numb_write = write(server_fd, recvbuff, strlen(recvbuff)+1);
        sleep(1);
    }
    close(server_fd); /*close*/ 
}

int main(int argc, char *argv[])
{
    int portno;
    int server_fd;
    int id;
    struct sockaddr_in serv_addr;
	memset(&serv_addr, '0',sizeof(serv_addr));
	
    /* Đọc portnumber từ command line */
    if (argc < 4) {
        printf("command : ./sensor <server address> <port number> <sensor id>\n");
        exit(1);
    }
    portno = atoi(argv[2]);
    id = atoi(argv[3]);
	
    /* Khởi tạo địa chỉ server */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(portno);
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) == -1) 
        handle_error("inet_pton()");
	
    /* Tạo socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        handle_error("socket()");
	
    /* Kết nối tới server*/
    if (connect(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        handle_error("connect()");
	
    chat_func(server_fd, id);

    return 0;
}
