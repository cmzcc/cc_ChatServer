#pragma once

//公共文件

enum EnMsgType
{
    LOGIN_MSG = 1,     // 登录消息
    LOGIN_MSG_ACK,     // 登录消息确认
    REG_MSG,           // 注册消息
    REG_MSG_ACK,       // 注册消息确认
    ONE_CHAT_MSG,      // 单聊消息
    ADD_FRIEND_MSG,    // 添加好友消息
    CREATE_GROUP_MSG,  // 创建群组消息
    ADD_GROUP_MSG,     // 添加群组成员消息
    GROUP_CHAT_MSG,    // 群聊消息
    REMOVE_GROUP_MSG,  // 删除群组成员消息
    GROUP_CHAT_OUT_MSG, // 解散群组消息
    SEARCH_GROUP_DATA_MSG,// 查询群组成员消息
    LOGIN_OUT_MSG          // 登出消息
};