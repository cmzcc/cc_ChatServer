#include"FriendModel.hpp"
#include"db/mysql.hpp"
void FriendModel::insert(int userId, int friendId)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d,%d)", userId, friendId);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<User> FriendModel::query(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "select f.friendid,u.name,u.state from Friend f inner join User u on u.id=f.friendId where f.userId=%d", userId);
    MySQL mysql;
    vector<User> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user(atoi(row[0]), row[1], "", row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
User FriendModel::SearchFriend(int userId, int friendId)
{
    char sql[1024] = {0};
    sprintf(sql, "SELECT f.friendid, u.name, u.state FROM Friend f INNER JOIN User u ON u.id = f.friendId WHERE f.userId = %d AND f.friendId = %d", userId, friendId);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user(atoi(row[0]), row[1], "", row[2]);
                mysql_free_result(res);
                return user;
            }
            mysql_free_result(res);
        }
    }
    return User();
}
