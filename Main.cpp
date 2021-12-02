#include "TCP_Server.h"
#include "c_conf.h"

int main() {

    CConfig *p_config = CConfig::GetInstence();
    if(p_config->Load("myWebServer.conf") == false) {
        perror("config error!");
        exit(1);
    }

    Server myServer(p_config->GetString("ip"), p_config->GetInt("listenport", 12345));
    myServer.ServerStart();
    return 0;
}