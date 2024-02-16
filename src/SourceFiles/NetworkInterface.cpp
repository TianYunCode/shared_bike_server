#include "NetworkInterface.h"
#include "DispatchMsgService.h"
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//切记：ConnectSession 必须是C类型的成员变量
static ConnectSession* session_init(int fd, struct bufferevent* bev) 
{
    ConnectSession* temp = nullptr;
    temp = new ConnectSession();

    if (!temp) {
        fprintf(stderr, "malloc failed. reason: %m\n");
        return nullptr;
    }

    memset(temp, '\0', sizeof(ConnectSession));
    temp->bev = bev;
    temp->fd = fd;

    return temp;
}

//释放会话
void session_free(ConnectSession* cs)
{
    if (cs)
    {
        if (cs->read_buf)
        {
            delete[] cs->read_buf;
            cs->read_buf = nullptr;
        }

        if (cs->write_buf)
        {
            delete[] cs->write_buf;
            cs->write_buf = nullptr;
        }
        delete cs;
    }
}

//重置会话
void session_reset(ConnectSession* cs)
{
    if (cs)     //判断 cs 是否为空
    {
        if (cs->read_buf)   //判断 cs->read_buf 是否为空
        {
            delete[] cs->read_buf;
            cs->read_buf = nullptr;
        }

        if (cs->write_buf) //判断 cs->write_buf 是否为空
        {
            delete[] cs->write_buf;
            cs->write_buf = nullptr;
        }

        cs->session_stat = SESSION_STATUS::SS_REQUEST;      //把会话状态设置为 请求状态
        cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;      //把传输状态设置为 读取头部

        cs->message_len = 0;            //总读消息长度设为 0
        cs->read_message_len = 0;       //已读消息长度设为 0
        cs->read_header_len = 0;        //已读头部长度设为 0

    }
}


NetworkInterface::NetworkInterface()
{
    base_ = nullptr;
    listener_ = nullptr;
}

NetworkInterface::~NetworkInterface()
{
    close();
}

bool NetworkInterface::start(int port)
{
    struct sockaddr_in sin;         //本机跟外部的通信信息（IP、Port、Family）
    memset(&sin, 0, sizeof(struct sockaddr_in));    //初始化为 0 IP为 0 代表本机所有IP
    sin.sin_family = AF_INET;       //通信类型为 IPv4
    sin.sin_port = htons(port);     //绑定端口

    base_ = event_base_new();

    //启动监听 设置回调
    listener_ = evconnlistener_new_bind(base_, NetworkInterface::listener_cb, base_,    //listener_cb 为回调函数 用来接收客户端连接
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        512, (struct sockaddr*)&sin,
        sizeof(struct sockaddr_in));
}

void NetworkInterface::close()
{
    if (base_)
    {
        event_base_free(base_);
        base_ = nullptr;
    }

    if (listener_)
    {
        evconnlistener_free(listener_);
        listener_ = nullptr;
    }
}

//回调函数 用来接收客户端连接
void  NetworkInterface::listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sock, int socklen, void* arg) 
{
    struct event_base* base = (struct event_base*)arg;

    LOG_DEBUG("accept a client %d\n", fd);

    //为这个客户端分配一个bufferevent  
    struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    ConnectSession* cs = session_init(fd, bev);         //初始化会话
    cs->session_stat = SESSION_STATUS::SS_REQUEST;      //设置会话状态为请求状态
    cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;      //设置传输状态为读取头部

    strcpy(cs->remote_ip, inet_ntoa(((sockaddr_in*)sock)->sin_addr));   //把网络地址转换成字符地址
    LOG_DEBUG("remote ip : %s\n", cs->remote_ip);

    //设置回调 收到请求回调 handle_request 数据发送成功回调 handle_response 出错回调 handle_error
    bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);      //设置允许读并且持久
    bufferevent_settimeout(bev, 60, 60);                //超时值应设置在配置文件
}

/*****************************************************
*            4字节         2个字节         4个字节
*  请求格式：BIKE	       事件ID	      数据长度N	    数据内容

*  1.包标识：  包头部的特殊标识, 用来标识包的开始
*  2.事件类型：事件ID, 固定两个字节表示
*  3.数据长度：数据包的大小, 固定长度4字节。
*  4.数据内容：数据内容, 长度为数据头定义的长度大小。
*
* ****************************************************/
//读请求回调
void NetworkInterface::handle_request(struct bufferevent* bev, void* arg)
{
    ConnectSession* cs = (ConnectSession*)arg;

    if (cs->session_stat != SESSION_STATUS::SS_REQUEST)     //如果传进来的会话状态不是请求状态 出错
    {
        LOG_WARN("NetworkInterface::handle_request - wrong session state[%d].", cs->session_stat);
        return;
    }

    if (cs->req_stat == MESSAGE_STATUS::MS_READ_HEADER)     //读取头部
    {
        i32 len = bufferevent_read(bev, cs->header + cs->read_header_len, MESSAGE_HEADER_LEN - cs->read_header_len);    //读数据 存到 cs->header 
        cs->read_header_len += len;     //已读的长度加刚读取的长度

        cs->header[cs->read_header_len] = '\0';     //把最后一个字符设为 '\0'
        LOG_DEBUG("recv from client<<<< %s\n", cs->header);

        //读取消息头部
        if (cs->read_header_len == MESSAGE_HEADER_LEN)          //判断已读取的长度是否等于头部长度
        {
            if (strncmp(cs->header, MESSAGE_HEADER_ID, strlen(MESSAGE_HEADER_ID)) == 0)     //做验证 判断前面 4字节 是不是 “BIKE” 
            {
                cs->eid = *((u16*)(cs->header + 4));            //从读取到的消息提取事件ID
                cs->message_len = *((i32*)(cs->header + 6));    //从读取到的消息提取数据长度

                LOG_DEBUG("NetworkInterface::handle_request - read  %d bytes in header, message len: %d\n", cs->read_header_len, cs->message_len);

                //对数据长度做判断 防止外部知道协议后 对服务器进行攻击 数据长度不能小于1和大于设定的最大消息长度
                if (cs->message_len<1 || cs->message_len > MAX_MESSAGE_LEN)     //MAX_MESSAGE_LEN 在 glo_def.h 中有定义 长度为 367280
                {
                    LOG_ERROR("NetworkInterface::handle_request wrong message, len: %u\n", cs->message_len);
                    bufferevent_free(bev);
                    session_free(cs);
                    return;
                }

                cs->read_buf = new char[cs->message_len];           //用来存储读取到的数据
                cs->req_stat = MESSAGE_STATUS::MS_READ_MESSAGE;     //更新请求状态 从读取头部更新为读取数据
                cs->read_message_len = 0;                           //初始化已读取数据的长度

            }
            else    //如果头部 4字节 不是“BIKE” 直接关闭请求
            {
                LOG_ERROR("NetworkInterface::handle_request - Invalid request from %s\n", cs->remote_ip);
                //直接关闭请求，不给予任何响应,防止客户端恶意试探
                bufferevent_free(bev);
                session_free(cs);
                return;
            }
        }
    }

    //读取数据      判断请求状态是否是读取数据状态 并且获取缓冲区还有多少数据 且大于零 否则虽然是读取数据状态 但已无数据可读
    if (cs->req_stat == MESSAGE_STATUS::MS_READ_MESSAGE && evbuffer_get_length(bufferevent_get_input(bev)) > 0)
    {
        i32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, cs->message_len - cs->read_message_len);   //读取数据到 cs->read_buf
        cs->read_message_len += len;    //更新已读取的数据长度
        LOG_DEBUG("NetworkInterface::handle_request - bufferevent_read: %d bytes, message len: %d read len: %d\n", len, cs->message_len, cs->read_message_len);

        if (cs->message_len == cs->read_message_len)        //判断已读取的数据长度是否等于头部获取到的数据长度
        {
            cs->session_stat = SESSION_STATUS::SS_RESPONSE; //更新会话状态为响应状态
            iEvent* ev = DispatchMsgService::getInstance()->parseEvent(cs->read_buf, cs->read_message_len, cs->eid);    //解析事件

            delete[] cs->read_buf;
            cs->read_buf = nullptr;
            cs->read_message_len = 0;

            if (ev)     //判断 ev 请求是否合理
            {
                ev->set_args(cs);   //用 iEvent::args_ 成员变量指向 cs
                DispatchMsgService::getInstance()->enqueue(ev);     //投递到消息队列 被线程池处理
            }
            else        //ev 请求有问题 为空
            {
                LOG_ERROR("NetworkInterface::handle_request - ev is null. remote ip: %s, eid: %d\n", cs->remote_ip, cs->eid);
                //直接关闭请求，不给予任何响应,防止客户端恶意试探
                bufferevent_free(bev);
                session_free(cs);
                return;
            }
        }
    }
}

//发送响应回调
void NetworkInterface::handle_response(struct bufferevent* bev, void* arg)
{
    LOG_DEBUG("NetworkInterface::handle_response ...\n");
}

//超时、连接关闭、读写出错等异常情况指定的回调函数
void NetworkInterface::handle_error(struct bufferevent* bev, short event, void* arg)
{
    ConnectSession* cs = (ConnectSession*)arg;

    LOG_DEBUG("NetworkInterface::handle_error ...\n");

    //判断是什么异常类型
    if (event & BEV_EVENT_EOF)      //连接关闭
    {
        LOG_DEBUG("connection closed\n");
    }
    else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_READING))        //读超时
    {
        LOG_WARN("NetworkInterface::reading timeout ...\n");
    }
    else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_WRITING))        //写超时
    {
        LOG_WARN("NetworkInterface::writting timeout ...\n");
    }
    else if (event & BEV_EVENT_ERROR)       //其他异常
    {
        LOG_ERROR("NetworkInterface:: some other error ...\n");
    }

    //这将自动close套接字和free读写缓冲区
    bufferevent_free(bev);
    session_free(cs);
}

 //对事件进行处理 需要不断调用
void NetworkInterface::network_event_dispatch()
{
    event_base_loop(base_, EVLOOP_NONBLOCK);    //循环处理请求事件 EVLOOP_NONBLOCK 为非阻塞
    //处理响应事件，回复响应消息 - 未完待续
    DispatchMsgService::getInstance()->handleAllResponseEvent(this);

}

//发送响应消息
void NetworkInterface::send_response_message(ConnectSession* cs)
{
    if (cs->response == nullptr)
    {
        bufferevent_free(cs->bev);
        if (cs->request)
        {
            delete cs->request;
        }

        session_free(cs);
    }
    else
    {
        bufferevent_write(cs->bev, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
        session_reset(cs);      //重置会话
    }
}