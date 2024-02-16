#ifndef SHARED_BIKE_COMMON_INICONFIG_H
#define SHARED_BIKE_COMMON_INICONFIG_H

#include <string>

#include "configdef.h"

using namespace std;

class Iniconfig
{
protected:
    Iniconfig();    //默认构造函数

public:
    ~Iniconfig();   //析构函数

    static Iniconfig* getInstance();    //单例模式
    bool load_file(const string& path); //加载配置文件
    const st_env_config& get_config();  //获取配置信息 st_env_config 是存放了配置信息的结构体

private:
    st_env_config _config;      //保存配置解析到的结果
    bool _isload;               //初始化为 false 加载配置后至为 true 用于判断配置是否已经加载 防止重复加载
    static Iniconfig* config;   //单例对象
};

#endif