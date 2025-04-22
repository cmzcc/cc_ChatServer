#include"ChatServer.hpp"
#include<iostream>
#include<ChatService.hpp>
#include<signal.h>
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc,char **argv)

{
    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr(argv[1], atoi(argv[2]));
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}