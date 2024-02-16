#include "bike.pb.h"
#include <string>
#include <iostream>


using namespace std;
using namespace tianyun;


int main(void)
{
    std::string data;   //存储序列化的消息

    //客户端发送请求
    {
        mobile_request mr;
        mr.set_mobile("18684518289");

        mr.SerializeToString(&data);
        cout<<"序列化后的数据["<<data.length()<<"]: "<< data << endl;
        cout<<hex<<(int)*((char*)data.c_str())<<endl;
        cout<<hex<<(int)*((char*)data.c_str() + 1)<<endl;
        //客户端发送data  send(sockfd, data.c_str(), data.length());
    }
    //服务器端接受请求
    {
        //receive(sockfd, data, ...);
        mobile_request mr;
        mr.ParseFromString(data);
        cout<<"客户端手机号码: " << mr.mobile() << endl;

    }

    return 0;
}