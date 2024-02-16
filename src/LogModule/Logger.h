#ifndef DISTRIBUTED_LOGGER_H
#define DISTRIBUTED_LOGGER_H

#include <iostream>
#include <string>

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RemoteSyslogAppender.hh>
#include <log4cpp/PropertyConfigurator.hh>


using namespace std;

class Logger
{
public:
    bool init(const string& log_conf_file); //初始化 log.conf 文件 用 log4cpp的接口对 log.conf 文件进行读取、配置
    static Logger* instance() {return &instance_;}   //单例模式 在任何地方都只有一个对象

    log4cpp::Category* GetHandle() {return category_;}

protected:
    static Logger instance_;    //单例模式对象
    log4cpp::Category* category_;   //通过该对象进行日志的操作
};

#define LOG_INFO  Logger::instance()->GetHandle()->info     //info  为 category_ 对象的成员函数
#define LOG_DEBUG Logger::instance()->GetHandle()->debug    //debug 为 category_ 对象的成员函数
#define LOG_ERROR Logger::instance()->GetHandle()->error    //error 为 category_ 对象的成员函数
#define LOG_WARN  Logger::instance()->GetHandle()->warn     //warn  为 category_ 对象的成员函数

#endif