#pragma once

#include <iostream>
#include <string>

#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>


#include "HttpData.h"


using namespace std;


class Server{
public:
    Server(string IpAdress, uint16_t Port, string fileRoot);
    Server(const Server& otherServer) = delete;
    Server& operator=(const Server& otherServer) = delete;

    void ServerStart();
    void httpRecvandSend(int fd);
private:
    int listen_fd;
    struct sockaddr_in my_addr;
    string _ipAdress;
    uint16_t _port;
    string _fileRoot;

};