#ifndef SHARED_BIKE_COMMON_INICONFIG_DEF_H
#define SHARED_BIKE_COMMON_INICONFIG_DEF_H

#include<string>
using namespace std;

//保存配置文件加载后的信息
typedef struct _st_env_config
{
    //数据库的配置
    string db_ip;
    unsigned short db_port;
    string db_user;
    string db_passwd;
    string db_name;

    //服务器的配置
    unsigned short server_port;

    _st_env_config(){}  //默认构造函数

    //构造函数
    _st_env_config(const string& db_ip, unsigned int db_port, const string& db_user, const string& db_passwd, const string& db_name, unsigned short server_port)
    {
        this->db_ip         = db_ip;
        this->db_port       = db_port;
        this->db_user       = db_user;
        this->db_passwd     = db_passwd;
        this->db_name       = db_name;
        this->server_port   = server_port;
    }

    _st_env_config& operator =(const _st_env_config& config)    //重载赋值运算符
    {
        if(this != &config) //判断运算符两边如果不是同一个对象
        {
            this->db_ip         = config.db_ip;
            this->db_port       = config.db_port;
            this->db_user       = config.db_user;
            this->db_passwd     = config.db_passwd;
            this->db_name       = config.db_name;
            this->server_port   = config.server_port;
        }

        return *this;
    }

}st_env_config;

#endif