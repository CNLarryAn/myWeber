#pragma once

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
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
    
    string _file_Root;

    string _method;
    string _httpVersion;
    string _filename;
    map<string, string> _headers;
public:
    HttpData(int client_fd, string file_Root);
    ~HttpData();

public:
    ssize_t HandleRead();
    void HandleWrite();
    void HandleError(int err_num, string short_msg);
    void ParseRequest();
    void InfoPrint();
};

class MimeType 
{
private:
    static void init();
    static unordered_map<string, string> mime;
    MimeType();
    MimeType(const MimeType &m);

public:
    static std::string getMime(const std::string &suffix);

private:
    static pthread_once_t once_control;
};
