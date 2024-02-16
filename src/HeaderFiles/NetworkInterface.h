#ifndef BRK_INTERFACE_NETWORK_INTERFACE_H_
#define BRK_INTERFACE_NETWORK_INTERFACE_H_

#include <event.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <string>

#include "glo_def.h"
#include "ievent.h"

#define MESSAGE_HEADER_LEN  10          //消息传输格式的头部总长度      （头部标识（4字节）、事件ID（2字节）、数据长度（4字节））——————————数据内容
#define MESSAGE_HEADER_ID   "BIKE"      //消息传输格式的头部标识

//会话状态
enum class SESSION_STATUS
{
	SS_REQUEST,            //请求状态
	SS_RESPONSE            //响应状态
};

//传输状态
enum class MESSAGE_STATUS
{
	MS_READ_HEADER = 0,     //读取头部
	MS_READ_MESSAGE = 1,    //消息传输未开始
	MS_READ_DONE = 2,       //消息传输完毕
	MS_SENDING = 3          //消息传输中
};

//保存需要发送的消息的各种状态信息
typedef struct _ConnectSession {

	char remote_ip[32];                     //IP地址

	SESSION_STATUS  session_stat;           //状态 请求或响应

	iEvent* request;
	MESSAGE_STATUS  req_stat;               //请求的消息传输状态

	iEvent* response;
	MESSAGE_STATUS  res_stat;               //响应的消息传输状态

	u16  eid;                               //保存当前请求的事件id
	i32  fd;                                //保存当前传送的文件句柄

	struct bufferevent* bev;                //用来读缓冲或写缓冲

	char* read_buf;                         //保存读消息的缓冲区
	u32  message_len;                       //当前读写消息的长度
	u32  read_message_len;                  //已经读取的消息长度

	char header[MESSAGE_HEADER_LEN + 1];    //保存头部，10字节+1字节（便于存一个字符串结束符）
	u32 read_header_len;                    //已读取的头部长度

	char* write_buf;                        //保存写消息的缓冲区
	u32  sent_len;                          //已经发送的长度

}ConnectSession;

class NetworkInterface
{
public:
	NetworkInterface();
	~NetworkInterface();

	bool start(int port);       //初始化 启动
	void close();

	static void listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sock, int socklen, void* arg);    //接收连接回调

	static void handle_request(struct bufferevent* bev, void* arg);                 //读请求回调
	static void handle_response(struct bufferevent* bev, void* arg);                //发送响应回调
	static void handle_error(struct bufferevent* bev, short event, void* arg);      //出错回调 连接异常 连接超时

	void network_event_dispatch();          			//对事件进行处理 需要不断调用

	void send_response_message(ConnectSession* cs);     //发送响应消息

private:
	struct evconnlistener* listener_;
	struct event_base* base_;
};

#endif
