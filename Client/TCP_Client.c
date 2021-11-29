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
    //创建套接字
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1) {
        perror("socket");
        exit(-1);
    }

    // struct sockaddr_in my_addr;
    // my_addr.sin_family = AF_INET;
    // my_addr.sin_port = htons(0);
    // my_addr.sin_addr.s_addr = inet_addr("101.34.179.178");
    // bzero(&(my_addr.sin_zero), 8);

    //创建服务器端结构体
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(12345);
    // inet_addr已废弃，应使用inet_aton或inet_pton
    // dest_addr.sin_addr.s_addr = inet_addr("104.193.88.77");
    inet_pton(AF_INET, "101.34.179.178", &dest_addr.sin_addr.s_addr);
    bzero(&(dest_addr.sin_zero), 8);

    //连接服务器端，无需bind，会自动为sockfd绑定一个端口
    if(connect(sock_fd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1){
        perror("connect");
        exit(-1);
    }

    //发送
    char *msg = "GET / HTTP/1.1\r\nHost: 101.34.179.178\r\n\r\n";
    // char msg[65535] = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en,zh-CN;q=0.9,zh;q=0.8,en-US;q=0.7\r\nUser-Agent: Mozilla/5.0 (X11; CrOS x86_64 14324.13.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/97.0.4692.20 Safari/537.36\r\n\r\n";
    int bytes_sent = send(sock_fd, msg, strlen(msg), 0);


    struct timeval timeout = {3, 0};
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    
    //接收
    char recv_buffer[1024];
    int total_recv_bytes = 0, bytes_recv = 0;
    while(1){
        if((bytes_recv = recv(sock_fd, recv_buffer, 1024, 0)) == -1){
            if(errno == EWOULDBLOCK || errno == EAGAIN){
                printf("recv timeout..\n");
                break;
            }
            else if(errno == EINTR){
                printf("interrupt bu signal\n");
                continue;
            }
            else if(errno == ENOENT){
                printf("recv RST segement...\n");
                break;
            }
            else{
                printf("unknown error :%d\n", errno);
                exit(1);
            }
        }
        else if(bytes_recv == 0){
            printf("peer closed...\n");
            break;
        }
        else{
            total_recv_bytes += bytes_recv;
            recv_buffer[bytes_recv] = '\0';//不加这一行会重复输出最后一段？
            printf("%s", recv_buffer);
        }
    }
    printf("一共接收%dbyte数据\n", total_recv_bytes);

    // printf("%s\n", recv_buffer);

    //关闭连接
    close(sock_fd);

    return 0;
    
    
}