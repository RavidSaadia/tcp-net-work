/*
	TCP/IP client
*/


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>

#if defined _WIN32

// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>


#endif

#define SERVER_PORT 5068
#define SERVER_IP_ADDRESS "127.0.0.1"
#define SIZE 1024

void send_file(FILE *fp, int sockfd) {
    fp = fopen("1gb.txt", "r");
    char data[SIZE] = {0};
    size_t bytes_sent;
    if(fp == NULL){
        perror("File");
        exit(2);
    }
    while((bytes_sent = fread(data, 1, SIZE, fp)) > 0){
        if (send(sockfd, data, sizeof(data), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
    }
}


int main() {



    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Could not create socket : %d", errno);// opening socket.
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));// reset all the structure

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT); // convert to big endian.
    int rval = inet_pton(AF_INET, (const char *) SERVER_IP_ADDRESS, &serverAddress.sin_addr);
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }

    // Make a connection to the server with socket SendingSocket.

    if (connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("connect() failed with error code : %d", errno);
    }
    printf("connected to server\n");
    // Sends some data to server
    FILE *fp;


    fp = fopen("1gb.txt", "r");
    if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);
    }


    //get the current CC algo
    char buf[256];
    socklen_t len;
    len = sizeof(buf);
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, buf, &len) != 0) {
        perror("getsockopt");
        return -1;
    }

    printf("Current: %s\n", buf); //cubic

    //send the file
    for (int i = 0; i < 5; i++) {
        send_file(fp, sockfd);
        printf("[+]File data sent successfully.\n");
    }

//    sleep(2);
    //changing CC to reno
    strcpy(buf, "reno");
    len = strlen(buf);
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, buf, len) != 0) {
        perror("setsockopt");
        return -1;
    }
    len = sizeof(buf);
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, buf, &len) != 0) {
        perror("getsockopt");
        return -1;
    }
    printf("New: %s\n", buf);//reno

    //send the file
    for (int i = 0; i < 5; i++) {
        send_file(fp, sockfd);
        printf("[+]File data sent successfully.\n");
    }

//    printf("[+]Closing the connection.\n");
//    close(sockfd);

    return 0;
}
