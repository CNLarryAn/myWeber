#include<iostream>
#include<string>
#include<fstream>

#include<fcntl.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>

using namespace std;

int main(){
    int sock_fd, new_fd;
    
    struct sockaddr_in my_addr;
    struct sockaddr_in other_addr;

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(12345);
    // my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, "10.0.4.11", &my_addr.sin_addr.s_addr);
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
        sin_size = sizeof(struct sockaddr_in);
        if((new_fd = accept(sock_fd, reinterpret_cast<struct sockaddr*>(&other_addr), &sin_size)) == -1){
            perror("accept");
            continue;            
        }
        
        cout << "server: got connection from " << inet_ntoa(other_addr.sin_addr) << endl;


        string send_buf;
        //响应报文头部处理开始
        string header;
        header += "HTTP/1.1 200 OK\r\n";
        struct stat src_buf;
        string filename = "aHtmlFile.html";
        if(stat(filename.c_str(), &src_buf) < 0){
            perror("fileStat");
            exit(0);
        }
        header += "Content-Type: text/html;charset=utf-8\r\n";
        header += "Content-Length: " + to_string(src_buf.st_size) + "\r\n";
        header += "Server: AJL's Webserver\r\n";
        header += "\r\n";
        //响应报文头部处理结束
        send_buf += header;

        //打开资源文件
        ifstream file(filename, ios::in);
        if(!file){
            perror("fileOpen");
            exit(0);
        }
        // int src_fd = open(filename.c_str(), O_RDONLY, 0);
        // if(src_fd < 0){
        //     perror("fileOpen");
        //     exit(0);
        // }
        string file_buf;
        string line;
        while(getline(file, line)){
            file_buf += line + "\n";
        }
        //关闭资源文件
        file.close();

        send_buf += file_buf;

        char recv_buf[65535];
        
        if(!fork()){
            recv(new_fd, recv_buf, 65535, 0);
            std::cout << recv_buf << std::endl;    
            if(send(new_fd, send_buf.c_str(), strlen(send_buf.c_str()), 0) == -1){
                perror("send");
                close(new_fd);
                exit(0);
            }
            close(new_fd);
        }
    }
    while(waitpid(-1, NULL, WNOHANG) > 0);
}