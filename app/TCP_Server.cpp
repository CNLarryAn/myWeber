#include "TCP_Server.h"

Server::Server(string IpAdress, uint16_t Port, string fileRoot) {
    _ipAdress = IpAdress;
    _port = Port;
    _fileRoot = fileRoot;
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

    /*********************************************************************************/
    //每个TCP服务器都应该设置SO_REUSEADDR，保证地址复用，防止有处于TIME_WAIT状态的连接时bind失败。
    int reuseaddr = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, static_cast<const void*>(&reuseaddr), sizeof(reuseaddr)) == -1) {
        char *perrorinfo = strerror(errno);
        printf("setsockopt(SO_REUSEADDR)返回值为%d, 错误码为:%d， 错误信息为:%s；\n", -1, errno, perrorinfo);
    }

    if(bind(listen_fd, reinterpret_cast<struct sockaddr*>(&my_addr), sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if(listen(listen_fd, 300) == -1) {
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
            if((connect_fd = accept4(listen_fd, reinterpret_cast<struct sockaddr*>(&other_addr), &sin_size, SOCK_NONBLOCK)) == -1) {
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
                httpRecvandSend(client_fd);
                FD_CLR(client_fd, &all_set);
                client[i] = -1;
                
                if(--nready <= 0)
                    break;
            }
        }
    }
    // while(waitpid(-1, NULL, WNOHANG) > 0);
}

void Server::httpRecvandSend(int fd) {


    // struct timeval timeout = {3, 0};
    // setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    HttpData cli_Handle(fd, _fileRoot);
    while(cli_Handle.HandleRead() > 0) {
        cli_Handle.ParseRequest();
        cli_Handle.InfoPrint();
        cli_Handle.HandleWrite();
    }   
    close(fd);
    cout << "closed!!!!!" << endl;
}
