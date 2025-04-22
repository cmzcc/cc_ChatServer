#include "redis/redis.hpp"
#include "iostream"
using namespace std;

redis::redis()
{
    _publish_context = nullptr;
    _subcribe_context = nullptr;
}
redis::~redis()
{
    if (_publish_context)
    {
        redisFree(_publish_context);
        _publish_context = nullptr;
    }
    if (_subcribe_context)
    {
        redisFree(_subcribe_context);
        _subcribe_context = nullptr;
    }
}
bool redis::connect()
{
    // 连接redis服务器
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (_publish_context == nullptr)
    {
        cerr << "redis connect error!" << endl;
        return false;
    }
    _subcribe_context = redisConnect("127.0.0.1", 6379);
    if (_subcribe_context == nullptr)
    {
        cerr << "redis connect error!" << endl;
        return false;
    }
    // 在单独的线程中，监听通道上的消息，有消息给业务层进行上报
    thread observer_thread([this]()
                           { observer_channel_message(); });
    observer_thread.detach();
    cout << "redis connect success!" << endl;
    return true;
}

// 向redis指定的通道channel发布消息
bool redis::publish(const int channel, const string &message)
{
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        cerr << "redis publish error!" << endl;
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "redis publish error!" << endl;
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}
// 订阅指定的通道channel
bool redis::subscribe(const int channel)
{
    // subsrcibe命令本身会造成线程阻塞等待通道里面发生消息，所以只订阅通道，不接受消息
    // 消息的接受专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server上的响应消息，否则和notifyMsg线程抢占响应资源
    // 这里的this->_subcribe_context是一个独立的redisContex
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1)
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "subscribe command failed" << endl;
            return false;
        }
    }
    return true;
}

bool redis::unsubscribe(const int channel)
{
    if (REDIS_ERR==redisAppendCommand(this->_subcribe_context,"UNSUBSCRIBE %d",channel))
    {
        cerr<<"unsubscribe command failed!"<<endl;
        return false;
    }
    int done=0;
    while (!done)
    {
        if (REDIS_ERR==redisBufferWrite(this->_subcribe_context,&done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        } 
    }
    return true;
}
// 在独立线程中接受订阅通道的消息
//这样做的原因是因为redis的subscribe命令会阻塞当前线程，
// 直到有消息到来才会返回，所以我们需要在一个独立的线程中去接受消息 
void redis::observer_channel_message()
{
    redisReply *reply=nullptr;
    while (REDIS_OK==redisGetReply(this->_subcribe_context,(void **)&reply))
    {
        if(reply!=nullptr&&reply->element[2]!=nullptr&&reply->element[2]->str!= nullptr)
        {
            // 这里的reply->element[2]->str就是我们订阅的消息
            // 这里的reply->element[1]->str就是我们订阅的通道
            // 这里的reply->element[0]->str就是我们订阅的消息类型
            
            string message(reply->element[2]->str, reply->element[2]->len);
            if (_notify_message_handler)
            {
                _notify_message_handler(atoi(reply->element[1]->str), message);
            }
        }
        freeReplyObject(reply);
    }
    cerr << "redis observer_channel_message error!" << endl;
}
