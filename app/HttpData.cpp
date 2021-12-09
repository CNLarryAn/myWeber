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


    return read_Length;
    
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