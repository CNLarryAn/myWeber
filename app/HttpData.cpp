#include "HttpData.h"

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
unordered_map<string, string> MimeType::mime;

void MimeType::init() {

    mime[".html"] = "text/html;charset=utf-8";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html;charset=utf-8";
    mime[".js"] = "application/javascript";
    mime[".css"] = "text/css";
}

std::string MimeType::getMime(const std::string &suffix) {

    pthread_once(&once_control, MimeType::init);//只执行一次init()
    if (mime.find(suffix) == mime.end())
        return mime["default"];
    else
        return mime[suffix];
}

HttpData::HttpData(int client_fd, string file_root) : _fd(client_fd), _file_Root(file_root) {
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
            if(_filename.back() == '/') _filename += "index.html";
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
    string filename = _file_Root + _filename;
    if(stat(filename.c_str(), &src_stat) < 0) {
        HandleError(404, "Not Fount!");
        perror("fileStat");
        return;
    }
    int dot_pos = _filename.find('.');
    string filetype;
    if (dot_pos < 0)
        filetype = MimeType::getMime("default");
    else
        filetype = MimeType::getMime(_filename.substr(dot_pos));
    header += "Content-Type: " + filetype + "\r\n";
    header += "Content-Length: " + to_string(src_stat.st_size) + "\r\n";
    header += "Server: AJL's Webserver\r\n";
    header += "\r\n";
    //响应报文头部处理结束
    send_buf += header;


    int src_fd = open(filename.c_str(), O_RDONLY, 0);
    if (src_fd < 0) {

        HandleError(404, "Not Found!");
        return;
    }
    //常规文件操作需要从磁盘到页缓存再到用户主存的两次数据拷贝。而mmap操控文件，只需要从磁盘到用户主存的一次数据拷贝过程。因此在读取时mmap效率更高。
    void *mmapRet = mmap(NULL, src_stat.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    cout << src_stat.st_size << endl;
    close(src_fd);
    if (mmapRet == (void *)-1) {
        munmap(mmapRet, src_stat.st_size);
        HandleError(404, "Not Found!");
        return;
    }
    char *src_addr = static_cast<char *>(mmapRet);
    send_buf += string(src_addr, src_addr + src_stat.st_size);

    munmap(mmapRet, src_stat.st_size);
    cout << send_buf.size() << endl;

    int send_len = 0;
    send_len = writen(_fd, send_buf);

    if(send_len == -1) {
        perror("send");
        close(_fd);
        return;
    }
    cout << send_len << endl;
}

void HttpData::HandleError(int err_num, string short_msg) {

    short_msg = " " + short_msg;
    string send_buff;
    string body_buff, header_buff;
    body_buff += "<html><title>哎~出错了</title>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += to_string(err_num) + short_msg;
    body_buff += "<hr><em> AJL's Webserver</em>\n</body></html>";
    header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-Type: text/html;charset=utf-8\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: AJL's Webserver\r\n";
    header_buff += "\r\n";
    
    send_buff += header_buff + body_buff;
    send(_fd, send_buff.c_str(), strlen(send_buff.c_str()), 0);
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