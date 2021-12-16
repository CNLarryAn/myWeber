#include "TCP_Server.h"
#include "c_conf.h"

int main(int argc, char **argv) {

    //加载配置文件
    CConfig *p_config = CConfig::GetInstence();
    if(p_config->Load("myWebServer.conf") == false) {
        perror("config error!");
        exit(1);
    }
    string ip = p_config->GetString("ip");
    int port = p_config->GetInt("listenport", 12345);
    string fileRoot = p_config->GetString("fileroot");

    //开始服务
    Server myServer(ip, port, fileRoot);
    myServer.ServerStart();
    return 0;
}