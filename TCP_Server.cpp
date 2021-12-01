#include"TCP_Server.h"

Server::Server(string myIpAdress, uint16_t myPort){
    _ipAdress = myIpAdress;
    _port = myPort;
}

void Server::ServerStart(){
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(_port);
    // my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, _ipAdress.c_str(), &my_addr.sin_addr.s_addr);
    bzero(&(my_addr.sin_zero), 8);

    if(bind(sock_fd, reinterpret_cast<struct sockaddr*>(&my_addr), sizeof(struct sockaddr)) == -1){
        perror("bind");
        exit(1);
    }

    if(listen(sock_fd, 10) == -1){
        perror("listen");
        exit(1);
    }
    unsigned int sin_size;
    while(1){
        int new_fd;
        struct sockaddr_in other_addr;
        sin_size = sizeof(struct sockaddr_in);
        if((new_fd = accept(sock_fd, reinterpret_cast<struct sockaddr*>(&other_addr), &sin_size)) == -1){
            perror("accept");
            continue;            
        }
        cout << "server: got connection from " << inet_ntoa(other_addr.sin_addr) << endl;

        pthread_t t_id;
        if(pthread_create(&t_id, NULL, httpRecvandSend, (void*)&new_fd) != 0){
            perror("pthread_create");
            exit(1);
        }

    }
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void* Server::httpRecvandSend(void *arg){
    int new_fd = *((int*) arg);
    struct timeval timeout = {3, 0};
    setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    char recv_buf[65535];
    while(recv(new_fd, recv_buf, 65535, 0) > 0){
        std::cout << recv_buf << std::endl;
        memset(recv_buf, 0, 65535);
        string send_buf;
        //响应报文头部处理开始
        string header;
        header += "HTTP/1.1 200 OK\r\n";
        struct stat src_stat;
        string filename = "aHtmlFile.html";
        if(stat(filename.c_str(), &src_stat) < 0){
            perror("fileStat");
            exit(0);
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
        int src_fd = open(filename.c_str(), O_RDONLY);
        if(src_fd < 0){
            perror("fileOpen");
            exit(0);
        }
        //有多种读取方法，<</getline/成员函数getline/成员函数get等等，目前来说getline最好用。
        //听说商用的代码使用系统函数那套比较多，改成open版本的。
        // string file_buf;
        // string line;
        // while(getline(file, line)){
        //     file_buf += line + "\n";
        // }
        char file_buf[1024];
        ssize_t read_size = read(src_fd, file_buf, 1024);
        cout << read_size << endl;
        //关闭资源文件
        close(src_fd);
        // send_buf += string(file_buf, file_buf + read_size);
        send_buf += file_buf;

        if(send(new_fd, send_buf.c_str(), strlen(send_buf.c_str()), 0) == -1){
            perror("send");
            close(new_fd);
            exit(0);
        }
    }   
    close(new_fd);
    cout << "closed!!!!!" << endl;
    return nullptr;
}
