#pragma once
#include"cc_muduo/TcpServer.h"
#include"cc_muduo/EventLoop.h"
#include"cc_muduo/Logger.h"
#include"model/UserModel.hpp"
#include"json.hpp"
#include"model/OfflineMessageModel.hpp"
#include "model/FriendModel.hpp"
#include "model/GroupModel.hpp"
#include"redis/redis.hpp"
#include<unordered_map>
#include<functional>
#include<string>
#include<mutex>
using json =nlohmann::json;
using namespace std;
//处理消息的回调函数类型
using MsgHandler=std::function<void(const TcpConnectionPtr&,
                                    json&, 
                                    Timestamp)>;
class ChatService
{
public:
    //获取单例对象
    static ChatService* instance();
    //获取消息处理器
    MsgHandler getHandler(int msgid);

    //注册消息处理器
    void regHandler(int msgid,MsgHandler handler);
    //处理登录请求
    void login(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //处理注册请求
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理聊天请求
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理添加好友请求
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理群聊请求
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理群组创建请求
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理群组添加成员请求
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理群组删除成员请求
    void removeGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理群组解散请求
    void groupChat_out(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理查询群组请求
    void SearchGroupData(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //处理注销登录请求
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //处理redis订阅消息
    void handleRedisSubscribeMessage(int channel, string message);
    //处理服务器异常退出
    void reset();

private:
    unordered_map<int, MsgHandler> _msgHandlerMap;
    //私有化构造函数
    ChatService();

    UserModel _userModel;//用户操作类对象

    //存储在线用户的连接
    unordered_map<int,TcpConnectionPtr>_userConnMap;
    
    //互斥锁
    mutex _connMutex;
    OfflineMsgModel _offlineMsgModel;//离线消息操作类对象
    FriendModel _friendModel;//好友操作类对象
    GroupModel _groupModel;//群组操作类对象
    redis _redis;
};