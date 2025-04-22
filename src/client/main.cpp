#include"json.hpp"
#include<iostream>
#include<thread>
#include<string>
#include<vector>
#include<chrono>
#include<ctime>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include"Group.hpp"
#include"User.hpp"
#include"public.hpp"
using namespace std;
using json =nlohmann::json;

//记录当前系统登录的用户信息
User current_User;
//记录当前系统登录的好友信息
vector<User> current_Friend;
//记录当前系统登录的群组信息
vector<Group> current_Group;
void startMenu(int clientfd);

//接收线程
void readTaskHandler(int clientfd);
string getcurrentTime();
int cin_choice();
//主页面
void mainMenu(int clientfd);
static bool is_first = true;
//聊天客户端程序的实现，main线程用作发送线程，子线程用作接收线程
int main(int argc,char **argv)
{
    if(argc<3)
    {
        cerr<<"command invalid! example:./ChatClient 127.0.0.1 6000 "<<endl;
        return 0;
    }
    //1.创建socket
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(clientfd<0)
    {
        cerr<<"socket error"<<endl;
        return 0;
    }
    //2.连接服务器
    sockaddr_in serveraddr;
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(atoi(argv[2]));
    inet_pton(AF_INET,argv[1],&serveraddr.sin_addr.s_addr);
    if(connect(clientfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
    {
        cerr<<"connect error"<<endl;
        close(clientfd);
        return 0;
    }
    
    while(true)
    {
        //3.登录注册主页面
        startMenu(clientfd);

    }
    
    close(clientfd);
    return 0;
}
//主页面
void startMenu(int clientfd)
{
    cout<<"*********欢迎使用聊天系统*************"<<endl;
    cout<<"************1.登录*******************"<<endl;
    cout<<"************2.注册*******************"<<endl;
    cout<<"************3.退出*******************"<<endl;
    cout<<"*************************************"<<endl;
    int choice = cin_choice();
    switch(choice)
    {
        case 1:
        {
            // 登录
            {
                int id = 0;
                string password;
                cout << "请输入你的id:";
                cin >> id;
                cin.get();
                cout << "请输入你的密码:";
                cin >> password;
                // 发送登录请求
                {
                    json js;
                    js["msgid"] = LOGIN_MSG;
                    js["id"] = id;
                    js["password"] = password;

                    // 将数据序列化
                    string sendBuf = js.dump();
                    int len=send(clientfd, sendBuf.c_str(), sendBuf.size()+1, 0);
                    if(len<0)
                    {
                        cout<<"send error"<<endl;
                        close(clientfd);
                        return;
                    }
                    else
                    {
    
                        char buffer[1024] = {0};
                        int ret = recv(clientfd, buffer, 1024, 0);
                        if (ret < 0)
                        {
                            cout << "recv error" << endl;
                            close(clientfd);
                            return;
                        }
                        else
                        {
                            json responsejs = json::parse(buffer);
                            if (responsejs["errno"].get<int>() != 0)
                            {
                                cout << responsejs["errmsg"] << endl;
                                startMenu(clientfd);
                            }
                            else
                            {
                                // 登录成功
                                // 接收登录成功的消息
        
                                current_User.setId(responsejs["id"].get<int>());
                                current_User.setName(responsejs["name"]);
                                current_User.setState("online");
                                if (responsejs["friends"].size() > 0)
                                {
                                    cout<<"你有" << responsejs["friends"].size() << "个好友" << endl;
                                    vector<string> friendList = responsejs["friends"];
                                    for (string &friendUser : friendList)
                                    {
                                        json friendjs = json::parse(friendUser);
                                        User user;
                                        user.setId(friendjs["id"].get<int>());
                                        user.setName(friendjs["name"]);
                                        user.setState(friendjs["state"]);
                                        current_Friend.push_back(user);
                                    }
                                   
                                }
                                if (responsejs["groups"].size() > 0)
                                {
                                    vector<string> groupList = responsejs["groups"];
                                    cout << "你有" << responsejs["groups"].size() << "个群组" << endl;
                                    for (auto &group :groupList)
                                    {
                                        Group groupObj;
                                        json groupjs = json::parse(group);
                                        groupObj.setId(groupjs["id"].get<int>());
                                        groupObj.setName(groupjs["groupname"]);
                                        groupObj.setDesc(groupjs["groupdesc"]);
                         
                                        vector<string> groupUsers = groupjs["groupusers"];
                                        for (auto &groupUser : groupUsers)
                                        {
                                            cout<<"群组成员信息:"<<groupUser<<endl;
                                            json groupUserjs = json::parse(groupUser);
                                            GroupUser groupUserObj;
                                            groupUserObj.setId(groupUserjs["id"].get<int>());
                                            groupUserObj.setName(groupUserjs["name"]);
                                            groupUserObj.setState(groupUserjs["state"]);
                                            groupUserObj.setRole(groupUserjs["role"]);
                                            groupObj.getGroupUsers().push_back(groupUserObj);
                                        }
                                        //将群组对象添加到当前群组列表中
                                        current_Group.push_back(groupObj);
                                    }
                                }
                                cout << "登录成功！你好," << responsejs["name"] << endl;
                                if (responsejs["offlinemsg"].size() > 0)
                                {
                                    vector<string> offlineMsg = responsejs["offlinemsg"];
                                    cout << "你有" << responsejs["offlinemsg"].size() << "条离线消息" << endl;
                                    //读取离线消息
                                    for (string &msg : offlineMsg)
                                    {
                                        json msgjs = json::parse(msg);
                                        int msgtype=msgjs["msgid"].get<int>();
                                        if (msgtype==ONE_CHAT_MSG)
                                        {
                                            cout << "离线消息[" << msgjs["time"] << "] " << msgjs["sendname"] << ":" << msgjs["msg"] << endl;
                                        }
                                        else
                                        {
                                            cout << "离线群消息[" << msgjs["time"] << "]:" << msgjs["groupname"] << "-"<<msgjs["sendname"] <<":"<< msgjs["msg"] << endl;
                                        }
                                      
                                    }
                                }
                                if(is_first)
                                {
                                    // 启动接收线程
                                    thread readTask(readTaskHandler, clientfd);
                                    readTask.detach();
                                    is_first = false;
                                    //之所以只能启动一次，是因为
                                    //如果多次启动线程，会导致多次接收数据，造成数据混乱
                                }  

                                // 进入主页面
                                mainMenu(clientfd);
                            }
                        }
                       
                    }
                }
            }
            // 接收登录结果
            break;
        }  
        case 2:
        {
            //注册
            {
                string regname;
                string regpassword;
                cout << "请输入你的用户名:";
                cin >> regname;
                cin.get();
                cout << "请输入你的密码:";
                cin >> regpassword;
                // 发送注册请求
                {
                    json js;
                    js["msgid"] = REG_MSG;
                    js["name"] = regname;
                    js["password"] = regpassword;
                    // 将数据序列化
                    string sendBuf = js.dump();
                    int len =send(clientfd, sendBuf.c_str(), strlen(sendBuf.c_str())+1, 0);
                    if(len<0)
                    {
                        cerr<<"send error"<<endl;
                        close(clientfd);
                        return;
                    }
                    else
                    {
                        //接收注册结果
                        char buffer[1024] = {0};
                        int ret = recv(clientfd, buffer, 1024, 0);
                        if (ret < 0)
                        {
                            cerr << "recv error" << endl;
                            close(clientfd);
                            return;
                        }
                        else
                        {
                            json responsejs = json::parse(buffer);
                            if (responsejs["errno"].get<int>() != 0)
                            {
                                cout << "注册失败，请重新注册！" << endl;
                            }
                            else
                            {
                                cout << "注册成功,用户id为:" << responsejs["id"] << "，请登录" << endl;
                            }
                        }
                      
                    }
                }
            }
            break;
        }
        case 3:
        {
            //退出
            close(clientfd);
            exit(0);
        }
        default:
        {
            cout<<"输入错误，请重新输入！"<<endl;
            break;
        }
    }
}
void readTaskHandler(int clientfd)
{
    while (1)
    {
        char buffer[1024] = {0};
        int ret = recv(clientfd, buffer, 1024, 0);
        if (ret <= 0)
        {
            cerr << "recv error or server closed connection" << endl;
            close(clientfd);
            return;
        }

        // 检查缓冲区是否为空
        if (strlen(buffer) == 0)
        {
            cerr << "收到空数据，忽略" << endl;
            continue;
        }

        try
        {
            if (nlohmann::json::accept(buffer))
            {
                json js = json::parse(buffer);
                switch (js["msgid"].get<int>())
                {
                case ONE_CHAT_MSG:
                    cout << "收到[" << js["time"] << "] " << js["sendname"] << ":" << js["msg"] << endl;
                    break;
                case GROUP_CHAT_MSG:
                    cout << "收到群组[" << js["groupname"] << "]的消息[" << js["time"] << "] " << js["sendname"] << ":" << js["msg"] << endl;
                    break;
                default:
                    break;
                }
            }
            else
            {
                cerr << "无效 JSON 数据: " << buffer << endl;
            }
        }
        catch (const nlohmann::json::parse_error &e)
        {
            cerr << "解析错误: " << e.what() << endl;
        }
    }
}

//获取当前时间
string getcurrentTime()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return string(buffer);
}
void addFriend(int clientfd)
{
    cout << "请输入你要添加的好友id:" << endl;
    int friendId = 0;
    cin >> friendId;
    //发送添加好友请求
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["from"] = current_User.getId();
    js["to"] = friendId;
    js["sendname"] = current_User.getName();
    js["msg"] = "添加好友";
    js["time"] = getcurrentTime();
    //将数据序列化
    string sendBuf = js.dump();
    int len=send(clientfd, sendBuf.c_str(), sendBuf.size()+1, 0);
    if(len<0)
    {
        cerr<<"send error"<<endl;
        close(clientfd);
        return;
    }

    char buffer[1024] = {0};
    int ret = recv(clientfd, buffer, 1024, 0);
    if (ret < 0)
    {
        cout << "recv error" << endl;
        close(clientfd);
        return;
    }
    else
    {
        json responsejs = json::parse(buffer);
        if (responsejs["errno"].get<int>() == 0)
        {
            cout << "添加好友成功" << endl;
            // 将好友信息添加到当前好友列表中
            User friendUser;
            friendUser.setId(friendId);
            friendUser.setName(responsejs["friendname"]);
            friendUser.setState(responsejs["friendstate"]);
            //将好友对象添加到当前好友列表中
            current_Friend.push_back(friendUser);
        }
        else
        {
            cout<<responsejs["errmsg"]<<endl;
        }
    }
    cout << "你是否要继续添加好友？(y/n)" << endl;
    char choice;
    cin >> choice;
    if (choice == 'y' || choice == 'Y')
    {
        addFriend(clientfd);
    }
    else
    {
        mainMenu(clientfd);
    }
}
void showFriends(int clientfd)
{
    cout<<"************好友列表************"<<endl;
    for(auto &friendUser:current_Friend)
    {
        cout<<"用户id:"<<friendUser.getId()<<endl;
        cout<<"用户名:"<<friendUser.getName()<<endl;
        cout<<"用户状态:"<<friendUser.getState()<<endl;
        cout<<"********************************"<<endl;
    }
    cout <<"请输入你的聊天好友(输入-1退出)" << endl;
    int Id = 0;
    cin >> Id;
    if (Id == -1)
    {
        mainMenu(clientfd);
        return;
    }
    //单聊
    for (auto &friendUser : current_Friend)
    {
        if (friendUser.getId() == Id)
        {
            cout << "请输入你要发送的消息:" << endl;
            string msg;
            cin >> msg;
            //发送消息
            json js;
            js["msgid"] = ONE_CHAT_MSG;
            js["from"] = current_User.getId();
            js["to"] = friendUser.getId();
            js["time"] = getcurrentTime();
            js["sendname"]=current_User.getName();
            js["msg"] = msg;
            //将数据序列化
            string sendBuf = js.dump();
            int len=send(clientfd, sendBuf.c_str(), sendBuf.size()+1, 0);
            if(len<0)
            {
                cerr<<"send error"<<endl;
                close(clientfd);
                return;
            }
        }
    }
    cout<<"你是否要继续聊天？(y/n)"<<endl;
    char choice;
    cin>>choice;
    if(choice=='y'||choice=='Y')
    {
        showFriends(clientfd);
    }
    else
    {
        mainMenu(clientfd);
    }
}
//展示群组列表
void showGroups(int clientfd)
{
    cout<<"************群组列表************"<<endl;
    for(auto &group:current_Group)
    {
        cout<<"群组id:"<<group.getId()<<endl;
        cout<<"群组名称:"<<group.getName()<<endl;
        cout<<"群组描述:"<<group.getDesc()<<endl;
        cout<<"********************************"<<endl;
    }

    while (1)
    {
        cout << "请输入你选择的群组id:(输入-1退出)" << endl;
        int groupId = 0;
        cin >> groupId;
        if (groupId == -1)
        {
            mainMenu(clientfd);
            return;
        }
        //检查当前用户是否已经加入该群组
        bool isJoin = false;
        for (auto &group : current_Group)
        {
            if (group.getId() == groupId)
            {
                isJoin = true;
                break;
            }
        }
        if (!isJoin)
        {
            cout << "你还没有加入该群组，请重新输入！" << endl;
            continue;
        }
        cout << "请输入你的行为(1.查看群组成员 2.在该群中发送消息)" << endl;
        int choice = 0;
        cin >> choice;
        if (choice == 1)
        {
            // 查询群组成员
            for (auto &group : current_Group)
            {
                if (group.getId() == groupId)
                {
                    cout << "群组成员列表:" << endl;
                    cout << "用户id\t用户名\t当前状态\t角色" << endl;
                    for (auto &groupUser : group.getGroupUsers())
                    {
                        cout << groupUser.getId() << "\t" << groupUser.getName()
                             << "\t" << groupUser.getState() << "\t" << groupUser.getRole() << endl;
                    }
                    break;
                }
            }
        }
        else if (choice == 2)
        {
            // 发送群组消息
            cout << "请输入你要发送的消息:" << endl;
            string msg;
            cin >> msg;
            // 发送消息
            json js;
            js["msgid"] = GROUP_CHAT_MSG;
            js["from"] = current_User.getId();
            js["groupid"] = groupId;
            js["groupname"] = current_Group[groupId - 1].getName();
            js["time"] = getcurrentTime();
            js["sendname"] = current_User.getName();
            js["msg"] = msg;
            // 将数据序列化
            string sendBuf = js.dump();
            int len = send(clientfd, sendBuf.c_str(), sendBuf.size() + 1, 0);
            if (len < 0)
            {
                cerr << "send error" << endl;
                close(clientfd);
                return;
            }
        }
        cout << "你是否留在该群聊(y/n)" << endl;
        char choice2;
        cin >> choice2;
        if (choice2 == 'y' || choice2 == 'Y')
        {
            continue;
        }
        else
        {
            break;
        }
    }
    cout << "你是否要继续群聊？(y/n)" << endl;
    char choice;
    cin >> choice;
    if (choice == 'y' || choice == 'Y')
    {
        showGroups(clientfd);
    }
    else
    {
        mainMenu(clientfd);
    }

}
//创建群组
void createGroup(int clientfd)
{
    cout << "请输入群组名称:" << endl;
    string groupName;
    cin >> groupName;

    cout << "请输入群组描述:" << endl;
    string groupDesc;
    cin >> groupDesc;
    //发送创建群组请求
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["from"] = current_User.getId();
    js["groupname"] = groupName;
    js["groupdesc"] = groupDesc;
    js["time"] = getcurrentTime();
    //将数据序列化
    string sendBuf = js.dump();
    int len=send(clientfd, sendBuf.c_str(), sendBuf.size()+1, 0);
    if(len<0)
    {
        cerr<<"send error"<<endl;
        close(clientfd);
        return;
    }
    else
    {
        char buffer[1024] = {0};
        int ret = recv(clientfd, buffer, 1024, 0);
        if (ret < 0)
        {
            cout << "recv error" << endl;
            close(clientfd);
            return;
        }
        else
        {
            json responsejs = json::parse(buffer);
            if (responsejs["errno"].get<int>() != 0)
            {
                cout << responsejs["errmsg"] << endl;
                createGroup(clientfd);
            }
            else
            {
                // 创建成功
                cout << "创建群组成功,群组id为:" << responsejs["groupid"] << endl;
                Group group;
                group.setId(responsejs["groupid"].get<int>());
                group.setName(groupName);
                group.setDesc(groupDesc);
                current_Group.push_back(group);
                mainMenu(clientfd);
            }
        }
    }
}
void addGroup(int clientfd)
{
    cout << "请输入你要加入的群组id:" << endl;
    int groupId = 0;
    cin >> groupId;
    
    //检查当前用户是否已经加入该群组
    for (auto &group : current_Group)
    {
        if (group.getId() == groupId)
        {
            cout << "你已经加入该群组，请重新输入！" << endl;
            addGroup(clientfd);
            return;
        }
    }
  
    //发送获取群组信息的请求
    json reqjs;
    reqjs["msgid"] = SEARCH_GROUP_DATA_MSG;
    reqjs["groupid"] = groupId;
    string sendBuf = reqjs.dump();
    int len=send(clientfd, sendBuf.c_str(), sendBuf.size()+1, 0);
    if(len<0)
    {
        cerr<<"send error"<<endl;
        close(clientfd);
        return;
    }
    char buffer[1024] = {0};
    int ret = recv(clientfd, buffer, 1024, 0);
    if (ret < 0)
    {
        cout << "recv error" << endl;
        close(clientfd);
        return;
    }
    else
    {
        json responsejs = json::parse(buffer);
        if (responsejs["errno"].get<int>() != 0)
        {
            cout << responsejs["errmsg"] << endl;
            addGroup(clientfd);
            return;
        }
        else
        {
            // 获取群组信息成功
            cout << "群组名称:" << responsejs["groupname"] << endl;
            cout << "群组描述:" << responsejs["groupdesc"] << endl;
            vector<string> groupUsers = responsejs["groupusers"];
            cout << "群组成员列表:" << endl;
            cout << "用户id\t用户名\t当前状态\t角色" << endl;
            for (auto &groupUser : groupUsers)
            {
                json groupUserjs = json::parse(groupUser);
                cout << groupUserjs["id"] << "\t" << groupUserjs["name"]
                     << "\t" << groupUserjs["state"] << "\t" << groupUserjs["role"] << endl;
            }
        
    
            cout << "你是否要加入该群组？(y/n)" << endl;
            char choice;
            cin >> choice;
            if (choice == 'y' || choice == 'Y')
            {
                // 发送加入群组请求
                json js;
                js["msgid"] = ADD_GROUP_MSG;
                js["from"] = current_User.getId();
                js["groupid"] = groupId;
                js["time"] = getcurrentTime();
                //将数据序列化
                string sendBuf = js.dump();
                int len=send(clientfd, sendBuf.c_str(), sendBuf.size()+1, 0);
                if(len<0)
                {
                    cerr<<"send error"<<endl;
                    close(clientfd);
                    return;
                }
                Group group;
                group.setId(groupId);
                group.setName(responsejs["groupname"]);
                group.setDesc(responsejs["groupdesc"]);
                current_Group.push_back(group);
                cout<< "加入群组" << responsejs["groupname"] << "成功" << endl;
                mainMenu(clientfd);
            }
            else
            {
                mainMenu(clientfd);
            }
        }
       
    }

}

int cin_choice()
{
    int choice = 0;
    while (true)
    {
        cout << "请输入你的选择：";
        if (cin >> choice)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (choice >= 1 && choice <= 6)
            { // 范围检查
                return choice;
            }
            else
            {
                cout << "选择无效，请输入 1 到 6 之间的数字！" << endl;
            }
        }
        else
        {
            cout << "输入错误，请输入一个数字！" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

void loginout(int clientfd)
{
  
    //注销登录
    json js;
    js["msgid"] = LOGIN_OUT_MSG;
    js["id"] = current_User.getId();
    string sendBuf = js.dump();
    int len=send(clientfd, sendBuf.c_str(), sendBuf.size()+1, 0);
    if(len<0)
    {
        cerr<<"send error"<<endl;
        close(clientfd);
        return;
    }
    cout << "用户" << current_User.getName() << "注销登录,期待下次登录" << endl;
    //清空当前用户信息
    current_User.setId(0);
    current_User.setName("");
    current_User.setState("offline");
    //清空当前登录用户信息
    current_Friend.clear();
    current_Group.clear();
}
//展示登录后的基本信息
void mainMenu(int clientfd)
{

        cout << "************欢迎使用聊天系统************" << endl;
        cout << "当前登录用户:" << current_User.getName() << endl;
        cout << "当前登录用户id:" << current_User.getId() << endl;
        cout << "*************1.私聊********************" << endl;
        cout << "*************2.群聊********************" << endl;
        cout << "*************3.添加好友****************" << endl;
        cout << "*************4.创建群组****************" << endl;
        cout << "*************5.加入群组****************" << endl;
        cout << "*************6.注销登录****************" << endl;
        cout << "**************************************" << endl;
        int choice=cin_choice();
        switch (choice)
        {
        case 1:
            // 查看好友列表
            showFriends(clientfd);
            break;
        case 2:
            // 查看群组列表
            showGroups(clientfd);
            break;
        case 3:
            addFriend(clientfd);
            break;
        case 4:
            // 创建群组
            createGroup(clientfd);
            break;
        case 5:
            //加入群组
            addGroup(clientfd);
            break;
        case 6:
            // 注销登录
            loginout(clientfd);
            break;
        default:
            cout << "输入错误，请重新输入！" << endl;
            mainMenu(clientfd);
            break;
        }
    
   
}