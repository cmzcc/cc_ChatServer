#include"mysql.hpp"

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "cmz050514";
static string database = "chat";
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}
// 初始化数据库连接
MySQL::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
    _conn = nullptr;
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), database.c_str(), 3306, NULL, 0);

    if (p != nullptr)
    {
        mysql_query(_conn, "set names utf8");
    }
    return p;
}
// 更新操作
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()) != 0)
    {
        LOG_ERROR("update error:%s", mysql_error(_conn));
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()) != 0)
    {
        LOG_ERROR("query error:%s", mysql_error(_conn));
        return nullptr;
    }
    return mysql_store_result(_conn);
}
