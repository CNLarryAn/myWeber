#pragma once

#include <iostream>
#include <string>
#include <map>
#include <algorithm>

#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <arpa/inet.h>

using namespace std;

class HttpData
{
private:
    int _fd;
    string _recv_Buffer;
    string _send_Buffer;

    string _method;
    string _httpVersion;
    string _filename;
    map<string, string> _headers;
public:
    HttpData(int client_fd);
    ~HttpData();

public:
    ssize_t HandleRead();
    void HandleWrite();
    void ParseRequest();
    void InfoPrint();
};


