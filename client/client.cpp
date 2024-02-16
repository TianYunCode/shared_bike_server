#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<unistd.h>
 
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
 
#include<event.h>
#include<event2/util.h>
#include "bike.pb.h" 

typedef unsigned short u16;
typedef unsigned int   u32;     /* int == long */
typedef signed char    i8;
typedef signed short   i16;
typedef signed int     i32;     /* int == long */
 
i32 icode = 0;
 
using namespace tianyun;
using namespace std;
 
int connect_server(const char* server_ip, int port);
 
void cmd_read_data(int fd, short events, void* arg);
void socket_read_data(int fd, short events, void *arg);
 
int main(int argc, char** argv)
{
    if( argc < 3 )
    {
        printf("please input 2 parameters\n");
        return -1;
    }
 
    //两个参数依次是服务器端的IP地址、端口号
    int sockfd = connect_server(argv[1], atoi(argv[2]));
    if( sockfd == -1)
    {
        perror("tcp_connect error ");
        return -1;
    }
 
    printf("connect to server successfully\n");
 
    struct event_base* base = event_base_new();
 
    struct event *ev_sockfd = event_new(base, sockfd, EV_READ | EV_PERSIST, socket_read_data, NULL);
    event_add(ev_sockfd, NULL);
	
    //监听终端输入事件
    struct event* ev_cmd = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, cmd_read_data, (void*)&sockfd);
 
    event_add(ev_cmd, NULL);
 
    event_base_dispatch(base);
 
    printf("finished \n");
    return 0;
}
 

void cmd_read_data(int fd, short events, void* arg)
{
    char msg[1024];
	char cmd[1024];
    std::string proto_msg;
 
    int ret = read(fd, cmd, sizeof(cmd)-1);
    if( ret <= 0 )
    {
        perror("read fail ");
        exit(1);
    }
 
    int sockfd = *((int*)arg);
    if(cmd[ret - 1]=='\n') 
		cmd[ret - 1] = '\0';
    else 
		cmd[ret] = '\0';

	if(strcmp(cmd, "send_mr") == 0)
	{
		mobile_request mr;
		mr.set_mobile("15007819636");
    
		int len = mr.ByteSizeLong();   //获取字节数
		memcpy(msg, "BIKE", 4);
	
		*(u16*)(msg + 4) = 1;     //事件id
		*(i32*)(msg + 6) = len;   //消息长度
		mr.SerializeToArray(msg + 10, len);
	
		//给服务端发请求
		write(sockfd, msg, len + 10);
	}
	else if(strcmp(cmd, "send_lr") == 0)
	{
		login_request lr;	
		lr.set_mobile("15007819636");
		lr.set_icode(icode);
		
		int len = lr.ByteSizeLong();
		memcpy(msg, "BIKE", 4);
		
		*(u16*)(msg + 4) = 3;     //事件id
		*(i32*)(msg + 6) = len;   //消息长度
		lr.SerializeToArray(msg + 10, len);
		
		write(sockfd, msg, len + 10);
	}
}
 
void socket_read_data(int fd, short events, void *arg)
{
    char msg[1024];
 
    //为了简单起见，不考虑读一半数据的情况
    int len = read(fd, msg, sizeof(msg)-1);
	
	//cout << len << endl;
	msg[len] = '\0';
 
    //printf("recv from server [%s], len:%d \n", msg, len);
    
	if(strncmp(msg, "BIKE", 4) == 0)
	{
		u16 code = *(u16*)(msg + 4);
		i32 len = *(i32*)(msg + 6);
		
		if(code == 2)     //获取短信请求
		{
			mobile_response mr;
			mr.ParseFromArray(msg + 10, len);
			icode = mr.icode();
			printf("mobile_response: code: %d, icode: %d, data: %s\n", mr.code(), mr.icode(), mr.data().c_str());
		}
		else if(code == 4)  //登陆验证请求
		{
			login_response lr;
			lr.ParseFromArray(msg + 10, len);
			printf("login_response: code: %d, decs: %s\n", lr.code(), lr.desc().c_str());
		}
	}
    
}

typedef struct sockaddr SA;
int connect_server(const char* server_ip, int port)
{
    int sockfd, status, save_errno;
    struct sockaddr_in server_addr;
 
    memset(&server_addr, 0, sizeof(server_addr) );
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    status = inet_aton(server_ip, &server_addr.sin_addr);
 
    if( status == 0 ) //the server_ip is not valid value
    {
        errno = EINVAL;
        return -1;
    }
 
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if( sockfd == -1 )
        return sockfd;
 
 
    status = connect(sockfd, (SA*)&server_addr, sizeof(server_addr) );
 
    if( status == -1 )
    {
        save_errno = errno;
        close(sockfd);
        errno = save_errno; //the close may be error
        return -1;
    }
	
    //evutil_make_socket_nonblocking(sockfd);
 
    return sockfd;
}
