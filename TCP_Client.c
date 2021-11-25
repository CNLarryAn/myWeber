#include<stdio.h>
#include<stdlib.h>
#include<string.h>
// #include<sys/types.h>
// #include<sys/socket.h>
// #include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>

int main() {
    int sockfd;
    struct sockaddr_in my_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("socket");
        exit(-1);
    }
    // my_addr.sin_family = AF_INET;
    // my_addr.sin_port = htons(0);
    // my_addr.sin_addr.s_addr = inet_addr("101.34.179.178");
    // bzero(&(my_addr.sin_zero), 8);

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(80);
    dest_addr.sin_addr.s_addr = inet_addr("104.193.88.77");
    bzero(&(dest_addr.sin_zero), 8);

    if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1){
        perror("connect");
        exit(-1);
    }

    char *msg = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n";

    int bytes_sent = send(sockfd, msg, strlen(msg), 0);

    char recv_buffer[1000000] = {0};
    int bytes_recv = recv(sockfd, recv_buffer, 1000000, 0);
    printf("%s\n", recv_buffer);

    close(sockfd);

    return 0;
    
    
}