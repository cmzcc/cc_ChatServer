#include "ChatService.hpp"
#include "public.hpp"
#include "model/UserModel.hpp"
#include "model/OfflineMessageModel.hpp"
#include<vector>
using namespace std;
using namespace placeholders;
// 获取单例对象
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息及其handler回调操作
ChatService::ChatService()
{
    // 注册消息处理器
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({REMOVE_GROUP_MSG,std::bind(&ChatService::removeGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_OUT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({SEARCH_GROUP_DATA_MSG,std::bind(&ChatService::SearchGroupData,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGIN_OUT_MSG, std::bind(&ChatService::loginout, this, _1,_2,_3)});
    // 其他消息处理器注册...
    if(_redis.connect())
    {
        //设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }
}
MsgHandler ChatService::getHandler(int msgid)
{
    // 查找消息处理器
    auto it = _msgHandlerMap.find(msgid);
    // 如果没有找到
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &,
                   json &,
                   Timestamp)
        {
            LOG_ERROR("msgid:%d can not find handler", msgid);
        };
    }
    // 返回消息处理器
    return it->second;
}
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO("logining...");
    // 1.获取用户名和密码
    int id = js["id"];
    string password = js["password"];
    // 2.查询用户
    User user = _userModel.query(id);
    if (user.getId() == id && user.getPassword() == password)
    {
        if (user.getState() == "online")
        {
            // 用户已在线
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "用户已在线，请勿重复登录";
            // 发送错误信息
            conn->send(response.dump());
            return;
        }
        else
        {
            // 登录成功，记录连接信息(注意线程安全，因为可能有多个线程同时登录)
            {
                lock_guard<mutex> lock(_connMutex);
                // 记录连接信息
                _userConnMap.insert({id, conn});
            }
            //id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);


            // 更新用户状态
            user.setState("online");
            _userModel.updateState(user);
        
            // 登录成功
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询离线消息
            vector<string> offlineMsg = _offlineMsgModel.query(id);
            if(!offlineMsg.empty())
            {
                response["offlinemsg"] = offlineMsg;
                //读取用户的离线消息后，删除离线消息
                _offlineMsgModel.remove(id);
            }
            vector<User> friendList = _friendModel.query(id);
            if(!friendList.empty())
            {
                vector<string> friendListStr;
                for(auto &friendUser:friendList)
                {
                    json friendJson;
                    friendJson["id"] = friendUser.getId();
                    friendJson["name"] = friendUser.getName();
                    friendJson["state"] = friendUser.getState();
                    friendListStr.push_back(friendJson.dump());
                }
                response["friends"] = friendListStr;
            }
            vector<Group> groupList = _groupModel.queryUserGroups(id);
            if(!groupList.empty())
            {
                vector<string> groupListStr;
                for(auto &group:groupList)
                {
                    json groupJson;
                    groupJson["id"] = group.getId();
                    groupJson["groupname"] = group.getName();
                    groupJson["groupdesc"] = group.getDesc();
                    vector<string> groupUsers;
                    for(auto &groupUser:group.getGroupUsers())
                    {
                        json js;
                        js["id"] = groupUser.getId();
                        js["name"] = groupUser.getName();
                        js["state"] = groupUser.getState();
                        js["role"] = groupUser.getRole();
                        groupUsers.push_back(js.dump());
                    }
                    groupJson["groupusers"] = groupUsers;
                    groupListStr.push_back(groupJson.dump());
                }
                response["groups"] = groupListStr;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误，请重新登录";
        conn->send(response.dump());
    }
}
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO("reg...");
    // 1.获取用户名和密码
    string name = js["name"];
    string password = js["password"];
    // 2.注册用户
    User user;
    user.setName(name);
    user.setPassword(password);
    if (_userModel.insert(user))
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    // 1.获取用户id
    int userId = 0;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                userId = it->first;
                _userConnMap.erase(it);
                break;
            }
        }
    }
    _redis.unsubscribe(userId);
    // 2.更新用户状态
    if (userId != 0)
    {
        User user;
        user.setId(userId);
        user.setState("offline");
        _userModel.updateState(user);
    }
}
void ChatService::reset()
{
    // 1.更新所有用户状态为离线
    _userModel.resetState();
    // 2.清空在线用户连接
    {
        lock_guard<mutex> lock(_connMutex);
        _userConnMap.clear();
    }
}
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1.获取发送者id和接收者id
    int fromId = js["from"].get<int>();
    int toId = js["to"].get<int>();
    // 2.查找接收者id是否存在
    User user = _friendModel.SearchFriend(fromId,toId);
    if (user.getId() == -1)
    {
        // 找到接收者连接，发送消息
        json response;
        User user = _userModel.query(toId);
        response["msgid"] = ONE_CHAT_MSG;
        response["errno"] = 1;
        response["errmsg"] = "你没有这个好友";
        conn->send(response.dump());
        return;
    }

    // 2.查找接收者连接
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toId);
        if (it != _userConnMap.end())
        {
   
            it->second->send(js.dump());
            return;
        }
        //不在这个服务器上，查询是否在另一台主机
        User user = _userModel.query(toId);
        if(user.getState()=="online")
        {
            _redis.publish(toId, js.dump());
            return;
        }
        // 没有找到接收者连接，存储离线消息
        _offlineMsgModel.insert(toId, js.dump());
    }
}
//添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1.获取发送者id和接收者id
    int fromId = js["from"].get<int>();
    int toId = js["to"].get<int>();
    string msg = js["msg"];
    // 2.查找接收者id是否存在
    User user=_userModel.query(toId);
    if(user.getId()==-1)
    {
        // 找到接收者连接，发送消息
        json response;
        User user = _userModel.query(toId);
        response["msgid"] = ADD_FRIEND_MSG;
        response["errno"] = 1;
        response["errmsg"]="该用户不存在";
        conn->send(response.dump());
        return;
    }
    
    _friendModel.insert(fromId, toId);
    _friendModel.insert(toId, fromId);
    // 2.查找接收者连接
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toId);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump()); 
        }
        else
        {
            // 没有找到接收者连接，存储离线消息
            _offlineMsgModel.insert(toId, js.dump());
        }
        // 找到接收者连接，发送消息
        json response;
        User user = _userModel.query(toId);
        response["msgid"] = ADD_FRIEND_MSG;
        response["errno"] = 0;
        response["friendname"] = user.getName();
        response["friendstate"] = user.getState();
        conn->send(response.dump());
    }
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1.获取发送者id和接收者id
    int fromId = js["from"].get<int>();
    string groupName = js["groupname"].get<string>();
    string groupDesc = js["groupdesc"].get<string>();
    // 2.创建群组
    Group group;
    group.setName(groupName);
    group.setDesc(groupDesc);
    if (_groupModel.createGroup(group))
    {
        // 创建成功
        json response;
        response["msgid"] = CREATE_GROUP_MSG;
        response["errno"] = 0;
        response["groupid"] = group.getId();
        conn->send(response.dump());
        _groupModel.addGroup(group.getId(),fromId, "creator");
    }
    else
    {
        // 创建失败
        json response;
        response["msgid"] = CREATE_GROUP_MSG;
        response["errno"] = 1;
        response["errmsg"] = "创建群组失败,已存在该群组";
        conn->send(response.dump());
    }
}
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1.获取发送者id和接收者id
    int fromId = js["from"].get<int>();
    int groupId = js["groupid"].get<int>();
    if(_groupModel.queryGroup(groupId).getId()==-1)
    {
        json response;
        response["msgid"] = ADD_GROUP_MSG;
        response["errno"] = 1;
        response["groupid"] = groupId;
        response["errmsg"] = "该群组不存在";
        conn->send(response.dump());
    }
    // 2.添加群组成员
    _groupModel.addGroup(groupId, fromId, "normal");
    
    // 3.告知发送者已成功成为群组成员
    json response;
    response["msgid"] = ADD_GROUP_MSG;
    response["errno"] = 0;
    response["groupid"] = groupId;
    response["msg"] = "已成功加入群组";
    conn->send(response.dump());

}
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1.获取发送者id和接收者id
    int fromId = js["from"].get<int>();
    int groupId = js["groupid"].get<int>();
    vector<int> groupUsers = _groupModel.queryGroupUsers(fromId, groupId);
    // 2.查找群组成员连接
    {
        lock_guard<mutex> lock(_connMutex);
        for (int id:groupUsers)
        {
            auto it = _userConnMap.find(id);
            if (it != _userConnMap.end())
            {

                it->second->send(js.dump());
            }
            else
            {
                User user = _userModel.query(id);
                if(user.getState()=="online")
                {
                    _redis.publish(id, js.dump());
                }
                else
                {
                    // 没有找到群组成员连接，存储离线消息
                    _offlineMsgModel.insert(id, js.dump());
                }
            }
        }
    }

}
void ChatService::removeGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1.获取发送者id和接收者id
    int fromId = js["from"].get<int>();
    int groupId = js["groupid"].get<int>();
    // 2.删除群组成员
    _groupModel.removeGroupUser(fromId, groupId);
   
    // 3.告知发送者已成功删除群组成员
    json response;
    response["msgid"] = REMOVE_GROUP_MSG;
    response["errno"] = 0;
    response["groupid"] = groupId;
    response["msg"] = "已成功删除群组成员";
    conn->send(response.dump());

}
void ChatService::groupChat_out(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1.获取发送者id和接收者id
    int fromId = js["from"].get<int>();
    int groupId = js["groupid"].get<int>();
    //检测发送者是否是群主
    Group group = _groupModel.queryGroup(groupId);
    if(group.getRole(fromId)!="creator")
    {
        json response;
        response["msgid"] = GROUP_CHAT_OUT_MSG;
        response["errno"] = 1;
        response["errmsg"] = "你不是群主，不能解散群组";
        conn->send(response.dump());
        return;
    }
    // 2.解散群组
    _groupModel.removeGroup(groupId);
   
    // 3.告知发送者已成功解散群组
    json response;
    response["msgid"] = GROUP_CHAT_OUT_MSG;
    response["errno"] = 0;
    response["groupid"] = groupId;
    response["msg"] = "已成功解散群组";
    conn->send(response.dump());

}
void ChatService::SearchGroupData(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
   int groupId = js["groupid"].get<int>();
    // 2.查询群组成员
    Group group = _groupModel.queryGroup(groupId);
    // 3.告知发送者已成功查询群组成员
    json response;
    response["msgid"] = SEARCH_GROUP_DATA_MSG;
    response["errno"] = 0;
    response["groupname"] = group.getName();
    response["groupdesc"] = group.getDesc();
    vector<string> groupUsers;
    for(auto &groupUser:_groupModel.queryGroupUsers(groupId))
    {
        json js;
        js["id"] = groupUser.getId();
        js["name"] = groupUser.getName();
        js["state"] = groupUser.getState();
        js["role"] = groupUser.getRole();
        cout<<"id:"<<groupUser.getId()<<endl;
        groupUsers.push_back(js.dump());
    }
    response["groupusers"] = groupUsers;
    conn->send(response.dump());

}
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1.获取发送者id和接收者id
    int fromId = js["id"].get<int>();
    // 2.更新用户状态
    User user;
    user.setId(fromId);
    user.setState("offline");
    _userModel.updateState(user);
    // 3.删除用户连接
    {
        lock_guard<mutex> lock(_connMutex);
        _userConnMap.erase(fromId);
    }
    _redis.unsubscribe(fromId);

}
void ChatService::handleRedisSubscribeMessage(int channel, string message)
{
    // 1.获取发送者id和接收者id
    int fromId = channel;
    // 2.查找接收者连接
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(fromId);
        if (it != _userConnMap.end())
        {
            it->second->send(message);
            return;  
        }
        // 没有找到接收者连接，存储离线消息
        _offlineMsgModel.insert(fromId, message);
    }
}