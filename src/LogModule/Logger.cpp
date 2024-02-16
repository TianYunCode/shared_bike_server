#include "Logger.h"

Logger Logger::instance_;   //静态成员需要在外部进行定义

//初始化 log.conf 文件 用 log4cpp的接口对 log.conf 文件进行读取、配置
bool Logger::init(const string& log_conf_file)
{
    try
    {
        log4cpp::PropertyConfigurator::configure(log_conf_file);    //使用 log4cpp 的接口初始化 log.conf 文件
    }
    catch(log4cpp::ConfigureFailure& f)     //如果失败
    {
        cerr<<" load log config file "<<log_conf_file.c_str()<<" failed with result : "<<f.what()<<endl;    // f.what() 函数可以获取错误原因
        return false;
    }

    //如果成功 执行以下代码
    category_ = &log4cpp::Category::getRoot();
    
    return true;
}