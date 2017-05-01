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

void tcpServerDownloadFile (int connFd, struct sockaddr_in clientAddr, char *fileServerRoot);

int main(int argc, char *argv[]) {
    /*char serverIP[IP_SIZE];

    if (argc >= 2) {

    }*/
    char serverIP[IP_SIZE] = "127.0.0.1";
    int sockFd = 0;
    struct sockaddr_in serverAddr;

    bzero(&serverAddr, sizeof(serverAddr));

    /* create sock */
    sockFd = socket(AF_INET, SOCK_STREAM, 0)；
    if (sockFd < 0) {
        perror("sock init failed!\n");
        exit(-1);
    } else {
        printf("create socket success!\n");
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
    serverAddr.sin_port   = htons(SERVER_LISTEN_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    //bind
    if (bind(sockFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) > 0) {
        perror("bind error\n");
        exit(-1);
    } else {
        printf("server bind success!\n");
    }

    //listen
    if(listen(sockFd, LISTEN_QUEUE)) {
        printf("server listen failed, err is %d\n", errno);
        exit(-1);
    } else {
        printf("listen success\n");
    }

    struct sockaddr_in clientAddr;
    socklen_t          length = sizeof(sockaddr_in);
    int                connFd = 0;

    while (1) {
        //accept
        connFd = accept(sockFd, (struct sockaddr *)clientAddr, &length);

        if (connFd == -1) {
            printf("accpet err\n");
            continue;
        }

        printf("get connection\n");
        //pull file

        close(connFd);       
    }


    return 0;
}

void tcpServerDownloadFile (int connFd, struct sockaddr_in clientAddr, char *fileServerRoot) {
    char buffer[BUFFER_SIZE];
    char filename[MAX_FILE_NAME_LEN];
    char fileServerPath[MAX_FILE_NAME_LEN];
    FILE *stream = NULL;

    int count = 0;
    int dataLen = 0;
    int writeLen = 0;
    int flag = 0;

    bzero(buffer, BUFFER_SIZE);

    count = recv(connFd, buffer, BUFFER_SIZE, 0);

    if (count < 0) {
        printf("recv filename failed!\n");
        exit(1);
    }

    strncpy(filename, buffer, strlen(buffer) > MAX_FILE_NAME_LEN? MAX_FILE_NAME_LEN : strlen(buffer));
    strcpy(fileServerPath, fileServerRoot);
    strcpy(fileServerPath, filename);
    printf("file path:\n %s\n", fileServerPath);

    //open a file
    stream = fopen(fileServerPath, "w");
    if (NULL == stream) {
        printf("create file failed\n");
        exit(-1);
    }
    bzero(buffer, BUFFER_SIZE);

    printf("start recv file\n");

    while((dataLen = recv(connFd, buffer, BUFFER_SIZE, 0)) > 0) {
        //if (dataLen)
        writeLen = fwrite(buffer, sizeof(char), dataLen, stream);
        if (writeLen != dataLen) {
            printf("write error, upload interupt\n")
            exit(-1);
        }

        bzero(buffer, BUFFER_SIZE);
    }

    printf("upload file ok\n");

    fclose(stream);
}

