#pragma once

#include"User.hpp"

class GroupUser:public User
{
public:
    void setRole(string role) { _role = role; }
    string getRole() const { return _role; }
private:
    string _role; // 群组成员角色
};
