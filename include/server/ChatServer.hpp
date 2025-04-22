#pragma once
#include<cc_muduo/TcpServer.h>
#include<cc_muduo/EventLoop.h>
#include<functional>

using namespace std;
using namespace placeholders;
//聊天服务器的主类
class ChatServer
{
private:
    TcpServer _server;//cc_muduo库
    EventLoop *_loop;//mainLoop对象的指针
    //上报连接相关信息的回调函数
    void onConnection(const TcpConnectionPtr&);
    //上报读写事件相关信息的回调函数
    void onMessage(const TcpConnectionPtr&,
                        Buffer*,
                        Timestamp);
public:
    //初始化聊天服务器对象
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
    //启动服务
    void start();

};
