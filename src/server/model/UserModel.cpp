#include"UserModel.hpp"
#include"db/mysql.hpp"

bool UserModel::insert(User &user)
{
    //拼接sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            //获取插入成功的id
            user.setId(mysql_insert_id(mysql.getConnection()));
            LOG_INFO("insert user %s ok", user.getName().c_str());
            return true;
        }
    }
    return false;
}
//根据用户id查询用户信息
User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id=%d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user(atoi(row[0]), row[1], row[2], row[3]);
                mysql_free_result(res);//释放结果集
                LOG_INFO("query user %d ok", id);
                return user;
            }
        }
    }
    return User();
}
//根据用户id更新用户状态
bool UserModel::updateState(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "update User set state='%s' where id=%d", 
            user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            LOG_INFO("update user %d state to %s", user.getId(), user.getState().c_str());
            return true;
        }
    }
    return false;
}
void UserModel::resetState()
{
    char sql[1024] = "update User set state='offline' where state='online'";
    //拼接sql语句
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
        LOG_INFO("reset all user state to offline");
    }
}
