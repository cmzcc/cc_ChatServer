#pragma once

#include"Group.hpp"
#include<string>
#include<vector>

using namespace std;

class GroupModel
{
public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组
    void addGroup(int userId, int groupId, string role);
    //查询群组
    Group queryGroup(int groupId);
    //查询用户所在的群组列表
    vector<Group> queryUserGroups(int userId);
    //查询群组的成员列表(除自己，用于群组聊天)
    vector<int> queryGroupUsers(int userid,int groupId);
    vector<GroupUser> queryGroupUsers(int groupId);
    //从群里删除成员
    bool removeGroupUser(int userId, int groupId);
    //删除群组
    bool removeGroup(int groupId);
    
};