#include "HttpData.h"

HttpData::HttpData(int client_fd) : _fd(client_fd) {
}

HttpData::~HttpData() {
}

ssize_t HttpData::HandleRead() {

    char recv_buf[65535];
    ssize_t read_Length = recv(_fd, recv_buf, 65535, 0);
    if(read_Length <= 0) {
        return read_Length;
    }
    _recv_Buffer = string(recv_buf, recv_buf + read_Length);
    return read_Length;
}

void HttpData::ParseRequest() {
    
    string& str = _recv_Buffer;
    //裁取请求行
    size_t pos_line = str.find('\r');
    if(pos_line < 0) {
        
    }
    string request_line = str.substr(0, pos_line);
    if(str.size() > pos_line + 2) {
        str = str.substr(pos_line + 2);
    }
    else {
        str.clear();
    }
    //获取请求行中的数据
    size_t pos_method = request_line.find(' ');
    _method = request_line.substr(0, pos_method);

    size_t pos_file_front = request_line.find('/', pos_method);
    if(pos_file_front < 0) {
        _filename = "index.html";
        _httpVersion = "1.1";
    }
    else {
        size_t pos_file_tail = request_line.find(' ', pos_file_front);
        if(pos_file_tail - pos_file_front > 1) {
            _filename = request_line.substr(pos_file_front + 1, pos_file_tail - pos_file_front - 1);
            size_t pos_qm = _filename.find('?');
            if(pos_qm > 0) {
                _filename = _filename.substr(0, pos_qm);
            }
        }
        else {
            _filename = "index.html";
        }

        size_t pos_version = request_line.find('/', pos_file_tail);
        _httpVersion = request_line.substr(pos_version + 1, 3);
        
    }

    //处理请求头
    size_t i = 0;
    size_t pos = 0;
    while(pos < str.size()) {
        if(str[pos] == '\r' && str[pos + 1] == '\n') {
            pos += 2;
            break;
        }

        i = str.find(':', pos);
        string key = str.substr(pos, i - pos);
        while(str[i + 1] == ' ') i++;
        pos = i++;
        i = str.find('\r', pos);
        string value = str.substr(pos, i - pos);
        pos = i + 2;
        _headers[key] = value;
    }
}

void HttpData::HandleWrite() {

    string send_buf;
    //响应报文头部处理开始
    string header;
    header += "HTTP/1.1 200 OK\r\n";
    struct stat src_stat;
    string filename = "index.html";
    if(stat(filename.c_str(), &src_stat) < 0) {
        perror("fileStat");
        return;
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
        return;
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

    if(send(_fd, send_buf.c_str(), strlen(send_buf.c_str()), 0) == -1) {
        perror("send");
        close(_fd);
        return;
    }
}

void HttpData::InfoPrint() {
    
    cout << "method: " << _method << endl;
    cout << "filename: " << _filename << endl;
    cout << "httpVersion: " << _httpVersion << endl;

    for(auto i : _headers) {
        cout << i.first << ": " << i.second << endl;
    }
    cout << endl;
    _headers.clear();
}