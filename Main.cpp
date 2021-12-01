#include "TCP_Server.h"

int main(){
    Server myServer("10.0.4.11", 12345);
    myServer.ServerStart();
    return 0;
}