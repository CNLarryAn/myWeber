#pragma once

#include <iostream>
#include <string>
// #include<fstream>

#include <pthread.h>
// #include <fcntl.h>
#include <arpa/inet.h>
// #include <string.h>
// #include <unistd.h>
#include <sys/time.h>
// #include <sys/wait.h>
// #include <sys/stat.h>

#include "HttpData.h"


using namespace std;


class Server{
public:
    Server(string myIpAdress, uint16_t myPort);
    Server(const Server& otherServer) = delete;
    Server& operator=(const Server& otherServer) = delete;

    void ServerStart();
    static void* httpRecvandSend(void *arg);
private:
    int listen_fd;
    struct sockaddr_in my_addr;
    string _ipAdress;
    uint16_t _port;

};