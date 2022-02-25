#include"server.hpp"

int main()
{
    server myserver;

    while(myserver.setServerSocket()!=true){};

    myserver.work();

    return 0;
}