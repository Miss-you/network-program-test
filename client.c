#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#define IP_SIZE             (20)
#define SERVER_LISTEN_PORT  (10000)
#define BUFFER_SIZE (4096)
#define MAX_FILE_NAME_LEN (256)

void tcpClientUploadFile(int sockFd, struct sockaddr_in *serverAddr, char *filePath);

int main(int argc, char *argv[]) {
    int sockFd = 0;
    char serverIP[IP_SIZE];
    struct sockaddr_in serverAddr;

    if (argc == 2) {
        strcpy(serverIP, argv[1]);
    } else if (argc == 1){
        strcpy(serverIP, "127.0.0.1");
    } else {
        printf("error command\n");
        exit(-1);
    }

    //create socket
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
        printf("create socket failed\n");
        exit(-1);
    }

    /* set serverAddr */
    /* 
    struct sockaddr
    {
        unsigned short sa_family;
        char sa_data[14];
    };

    struct sockaddr_in {
        short            sin_family;       // 2 bytes e.g. AF_INET, AF_INET6
        unsigned short   sin_port;    // 2 bytes e.g. htons(3490)
        struct in_addr   sin_addr;     // 4 bytes see struct in_addr, below
        char             sin_zero[8];     // 8 bytes zero this if you want to
    };

    struct in_addr {
        unsigned long s_addr;          // 4 bytes load with inet_pton()
    };
    */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_LISTEN_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    //connect the server
    if (connect(sockFd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) < 0) {
        printf("connect failed\n");
        exit(-1);
    }

    printf("connect server success\n");

    //push file
    tcpClientUploadFile(sockFd, &serverAddr, "./test.go")

    close(sockFd);

    return 0;
}

void tcpClientUploadFile(int sockFd, struct sockaddr_in *serverAddr, char *filePath) {
    FILE *stream;
    char buffer[BUFFER_SIZE];
    char filename[MAX_FILE_NAME_LEN];
    int count = 0;

    bzero(buffer, BUFFER_SIZE);
    strcpy(filename, strrchr(filePath, '/') + 1);
    printf("filename is %s\n", filename);
    strncpy(buffer, filename, strlen(filename) > MAX_FILE_NAME_LEN ? MAX_FILE_NAME_LEN : strlen(filename));
    
    //send filename
    send(sockFd, buffer, BUFFER_SIZE, 0);

    stream = fopen(filePath, "r");
    if (NULL == stream) {
        printf("open file %s failed\n", filePath);
        exit(-1);
    }

    while((count  = fread(buffer, 1, BUFFER_SIZE, stream)) > 0) {
        //printf
        if (send(sockFd, buffer, count, 0) < 0) {
            printf("send failed\n");
            break;
        }

        bzero(buffer, BUFFER_SIZE);
    }

    printf("upload success\n");

    fclose(stream);
}