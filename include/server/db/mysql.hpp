#pragma once
#include<mysql/mysql.h>
#include<cc_muduo/Logger.h>
#include<string>
using namespace std;

//数据库操作类
class MySQL
{
public:
    //构造函数
    MySQL();
    //连接数据库
    ~MySQL();

    //连接数据库
    bool connect();
    //更新操作
    bool update(string sql);
    //查询操作
    MYSQL_RES* query(string sql);
    //获取连接
    MYSQL* getConnection()
    {
        return _conn;
    }

private:
    MYSQL *_conn;//MYSQL对象

};
