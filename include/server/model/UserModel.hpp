#pragma once
#include"User.hpp"
class UserModel
{
public:
    //根据用户id查询用户信息
    User query(int id);
    //增加用户信息
    bool insert(User &user);
    //根据用户id更新用户状态
    bool updateState(User &user);
    //重置用户的状态信息
    void resetState();
};