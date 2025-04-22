#include"ChatServer.hpp"
#include"json.hpp"
#include"public.hpp"
#include"ChatService.hpp"
using json=nlohmann::json;
ChatServer::ChatServer(EventLoop* loop,
                    const InetAddress& listenAddr,
                    const string& nameArg)
                    :_server(loop,listenAddr,nameArg)
{
    //注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));

    //注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

    //设置线程数量
    _server.setThreadNum(4);


}

//启动服务
void ChatServer::start()
{
    //启动服务
    _server.start();
}

//上报连接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }

}
//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn,
                        Buffer* buf,
                        Timestamp time)
{
    string msg(buf->retrieveAllAsString());
    //数据的解码（反序列化）
    json js=json::parse(msg);
    //完全解耦网络模块和业务模块
    //获取消息类型
    ChatService::instance()->getHandler(js["msgid"].get<int>())(conn,js,time);
}