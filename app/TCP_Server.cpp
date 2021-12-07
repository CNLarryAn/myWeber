#include "TCP_Server.h"

Server::Server(string myIpAdress, uint16_t myPort) {
    _ipAdress = myIpAdress;
    _port = myPort;
}

void Server::ServerStart() {

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    cout << listen_fd << endl;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(_port);
    // my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, _ipAdress.c_str(), &my_addr.sin_addr.s_addr);
    bzero(&(my_addr.sin_zero), 8);

    if(bind(listen_fd, reinterpret_cast<struct sockaddr*>(&my_addr), sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if(listen(listen_fd, 10) == -1) {
        perror("listen");
        exit(1);
    }

    // select模型相关参数初始化
    int maxfd = listen_fd;
    int maxi = -1;
    int client[FD_SETSIZE];
    memset(client, -1, FD_SETSIZE);
    fd_set read_set, all_set;
    FD_ZERO(&all_set);
    FD_SET(listen_fd, &all_set);

    unsigned int sin_size;
    int connect_fd;
    int client_fd;
    int i;
    while(1) {
        read_set = all_set;
        int nready = select(maxfd + 1, &read_set, nullptr, nullptr, nullptr);

        if(FD_ISSET(listen_fd, &read_set)) {
            
            struct sockaddr_in other_addr;
            sin_size = sizeof(struct sockaddr_in);
            if((connect_fd = accept(listen_fd, reinterpret_cast<struct sockaddr*>(&other_addr), &sin_size)) == -1) {
                perror("accept");
                continue;            
            }
            cout << "server: got connection from " << inet_ntoa(other_addr.sin_addr) << endl;

            //select模型相关
            for(i = 0; i < FD_SETSIZE; i++) {
                if(client[i] < 0) {
                    client[i] = connect_fd;
                    break;
                }
            }
            if(i == FD_SETSIZE) {
                perror("too many client!");
                exit(1);
            }
            FD_SET(connect_fd, &all_set);
            if(connect_fd > maxfd)
                maxfd = connect_fd;
            if(i > maxi)
                maxi = i;
            if(--nready <= 0) 
                continue;
        }

        for(i = 0; i <= maxi; i++) {
            if( (client_fd = client[i]) < 0) {
                continue;
            }
            if(FD_ISSET(client_fd, &read_set)) {
                cout << client_fd << endl;
                pthread_t t_id;
                if(pthread_create(&t_id, NULL, httpRecvandSend, (void*)&client_fd) != 0) {
                    perror("pthread_create");
                    continue;
                }
                if(--nready <= 0)
                    break;
            }
        }
    }
    // while(waitpid(-1, NULL, WNOHANG) > 0);
}

void* Server::httpRecvandSend(void *arg) {

    int client_fd = *((int*) arg);
    struct timeval timeout = {3, 0};
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    char recv_buf[65535];
    while(recv(client_fd, recv_buf, 65535, 0) > 0) {
        std::cout << recv_buf << std::endl;
        memset(recv_buf, 0, 65535);
        string send_buf;
        //响应报文头部处理开始
        string header;
        header += "HTTP/1.1 200 OK\r\n";
        struct stat src_stat;
        string filename = "aHtmlFile.html";
        if(stat(filename.c_str(), &src_stat) < 0) {
            perror("fileStat");
            return nullptr;
        }
        header += "Content-Type: text/html;charset=utf-8\r\n";
        header += "Content-Length: " + to_string(src_stat.st_size) + "\r\n";
        header += "Server: AJL's Webserver\r\n";
        header += "\r\n";
        //响应报文头部处理结束
        send_buf += header;

        //打开资源文件
        // ifstream file(filename, ios::in);
        // if(!file){
        //     perror("fileOpen");
        //     exit(0);
        // }

        // int src_fd = open(filename.c_str(), O_RDONLY);
        // if(src_fd < 0){
        //     perror("fileOpen");
        //     exit(0);
        // }
        FILE* src_fp;
        src_fp = fopen(filename.c_str(), "r");
        if(src_fp == nullptr) {
            perror("fopen");
            return nullptr;
        }
        //有多种读取方法，<</getline/成员函数getline/成员函数get等等，目前来说getline最好用。
        //听说商用的代码使用系统函数那套比较多，添加open和fopen版本的。
        // string file_buf;
        // string line;
        // while(getline(file, line)){
        //     file_buf += line + "\n";
        // }

        // char file_buf[1024];
        // ssize_t read_size = read(src_fd, file_buf, 1024);
        // cout << read_size << endl;
        char line_buf[500];
        string file_buf;
        while(!feof(src_fp)) {
            if(fgets(line_buf, 500, src_fp) == nullptr) {
                continue;
            }
            file_buf += line_buf;
        }
        //关闭资源文件
        // close(src_fd);
        fclose(src_fp);
        // send_buf += string(file_buf, file_buf + read_size);
        send_buf += file_buf;

        if(send(client_fd, send_buf.c_str(), strlen(send_buf.c_str()), 0) == -1) {
            perror("send");
            close(client_fd);
            return nullptr;
        }
    }   
    close(client_fd);
    cout << "closed!!!!!" << endl;
    return nullptr;
}
