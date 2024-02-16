#include "iniconfig.h"
#include <iniparser/iniparser.h>

Iniconfig* Iniconfig::config = nullptr;     //静态成员变量要在类外初始化

//默认构造函数
Iniconfig::Iniconfig():_isload(false),_config(st_env_config())  //给 _config 变量初始化 防止访问越界
{

}

//析构函数
Iniconfig::~Iniconfig()
{

}

//单例模式
Iniconfig* Iniconfig::getInstance()
{
	if (nullptr == config)
	{
		config = new Iniconfig();
	}

	return config;
}

//加载配置文件
bool Iniconfig::load_file(const string& path)
{
    //如果 _isload 为 true 则代表已经加载过了 不可重复加载
    if(_isload) return false;

    dictionary* ini = nullptr;  //字典 保存加载后的配置信息

    ini = iniparser_load(path.c_str()); //加载配置信息
    if(ini == nullptr)
    {
        fprintf(stderr, "cannot parse file: %s\n", path.c_str());
        return false;
    }

    //第一项是字典 第二项是要取的配置文件信息 第三项是默认值（如果第二项读取失败 则用默认值）
    const char*   ip = iniparser_getstring(ini, "database:ip", "127.0.0.1");
    int         port = iniparser_getint(ini, "database:port", 3306);
    const char *user = iniparser_getstring(ini, "database:user", "root");
    const char  *pwd = iniparser_getstring(ini, "database:pwd", "88888888");
    const char   *db = iniparser_getstring(ini, "database:db", "shared_bike");
    int        sport = iniparser_getint(ini, "server:port", 9090);

    _config = st_env_config((string)ip, port, (string)user, (string)pwd, (string)db, sport);    //把读取到的信息用st_env_config结构体存储起来

    iniparser_freedict(ini);    //释放字典

    _isload = true; //加载成功把标记设为 true 防止重复加载

    return _isload;
}

//获取配置信息 st_env_config 是存放了配置信息的结构体
const st_env_config& Iniconfig::get_config()
{
    return _config; //加载配置文件后 会把信息存在该变量里 返回该变量即可
}