#pragma once

#include "GroupUser.hpp"
#include <vector>
#include <string>
using namespace std;

class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
        : _id(id), _name(name), _desc(desc) {}

    void setId(int id) { _id = id; }
    void setName(string name) { _name = name; }
    void setDesc(string desc) { _desc = desc; }

    int getId() const { return _id; }
    string getName() const { return _name; }
    string getDesc() const { return _desc; }

    // 修改为返回引用
    vector<GroupUser> &getGroupUsers() { return _groupUsers; }
    // 保留 const 版本用于只读场景
    const vector<GroupUser> &getGroupUsers() const { return _groupUsers; }

    string getRole(int userId) const
    {
        for (const auto &user : _groupUsers)
        {
            if (user.getId() == userId)
            {
                return user.getRole();
            }
        }
        return "";
    }

private:
    int _id;                       // 群组id
    string _name;                  // 群组名称
    string _desc;                  // 群组描述
    vector<GroupUser> _groupUsers; // 群组成员列表
};
