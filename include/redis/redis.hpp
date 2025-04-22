#include<hiredis/hiredis.h>
#include<thread>
#include<functional>
using namespace std;
class redis
{
public:
    redis();
    ~redis();

    //连接redis服务器
    bool connect();

    //向redis指定的通道channel发布消息
    bool publish(const int channel, const string &message);

    //订阅指定的通道channel
    bool subscribe(const int channel);

    //取消订阅指定的通道channel
    bool unsubscribe(const int channel);
    
    //在独立线程中接受订阅通道的消息
    void observer_channel_message();

    //设置消息处理函数
    void init_notify_handler(function<void(int,string)> notify_message_handler)
    {
        _notify_message_handler = notify_message_handler;
    }
private:
    redisContext *_publish_context;
    redisContext *_subcribe_context;
    //这里
    function<void(int,string)> _notify_message_handler;
};