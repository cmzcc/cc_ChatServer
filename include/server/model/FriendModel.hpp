#pragma once
#include<vector>
#include"User.hpp"

using namespace std;
class FriendModel
{
public:
    //添加好友关系
    void insert(int userId, int friendId);
    //返回用户的好友列表
    vector<User> query(int userId);
    //删除好友关系
    bool remove(int userId, int friendId);
    User SearchFriend(int userId,int friendId);
};