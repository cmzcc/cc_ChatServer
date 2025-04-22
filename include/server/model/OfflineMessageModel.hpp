#pragma once


#include <string>
#include<vector>
using namespace std;
class OfflineMsgModel
{
public:
    //存储用户的离线消息
    void insert(int userId, string msg);
    //删除用户的离线消息
    void remove(int userId);
    //查询用户的离线消息
    vector<string> query(int userId);
    //删除所有用户的离线消息
    void removeAll(int userId);
};