#include"GroupModel.hpp"
#include"db/mysql.hpp"

bool GroupModel::createGroup(Group& group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s','%s')",
            group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            LOG_INFO("create group %s ok", group.getName().c_str());
            return true;
        }
    }
    return false;
}
void GroupModel::addGroup(int groupId, int userId, string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values(%d,%d,'%s')", groupId, userId, role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
Group GroupModel::queryGroup(int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select id,groupname,groupdesc from AllGroup where id=%d", groupid);
    MySQL mysql;
    Group group;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                mysql_free_result(res);
                return group;
            }

        }
    }
    return Group();
}
vector<Group> GroupModel::queryUserGroups(int userId)
{
    char sql[1024] = {0};
    /*
    先根据用户id查询用户所在的群组id，再根据群组id查询群组信息
    */
    sprintf(sql, "select g.id,g.groupname,g.groupdesc from GroupUser gu inner join AllGroup g on gu.groupid=g.id where gu.userid=%d", userId);
    MySQL mysql;
    vector<Group> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group(atoi(row[0]), row[1], row[2]);
                vec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    for (Group &group : vec)
    {
        sprintf(sql, "select gu.userid, u.name, u.state, gu.grouprole from GroupUser gu inner join User u on gu.userid = u.id where gu.groupid = %d", group.getId());
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;

            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser groupUser;
                groupUser.setId(atoi(row[0]));
                groupUser.setName(row[1]);
                groupUser.setState(row[2]);
                groupUser.setRole(row[3]);
                group.getGroupUsers().push_back(groupUser);
            }

            mysql_free_result(res);
        }

    }

    return vec;
}
vector<GroupUser> GroupModel::queryGroupUsers(int groupId)
{
    char sql[1024] = {0};
    sprintf(sql, "select gu.userid,u.name,u.state,gu.grouprole from GroupUser gu inner join User u on gu.userid=u.id where gu.groupid=%d", groupId);
    MySQL mysql;
    vector<GroupUser> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser groupUser;
                groupUser.setId(atoi(row[0]));
                groupUser.setName(row[1]);
                groupUser.setState(row[2]);
                groupUser.setRole(row[3]);
                vec.push_back(groupUser);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
vector<int> GroupModel::queryGroupUsers(int userid, int groupId)
{
    char sql[1024] = {0};
    sprintf(sql, "select gu.userid from GroupUser gu where gu.groupid=%d and gu.userid!=%d", groupId, userid);
    MySQL mysql;
    vector<int> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
bool GroupModel::removeGroupUser(int userid,int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from GroupUser where userid=%d and groupid=%d", userid, groupid);
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            LOG_INFO("remove group user %d ok", userid);
            return true;
        }
    }
    return false;
}
bool GroupModel::removeGroup(int groupId)
{
    char sql[1024] = {0};  
    sprintf(sql, "delete from AllGroup where id=%d", groupId);
    //删除群组成员
    sprintf(sql, "delete from GroupUser where groupid=%d", groupId);
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            LOG_INFO("remove group %d ok", groupId);
            return true;
        }
    }
    return false;
}
