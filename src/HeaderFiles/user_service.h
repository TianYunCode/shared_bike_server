#ifndef BRKS_SVR_USER_SERVICE_H_
#define BRKS_SVR_USER_SERVICE_H_

#include <memory>
#include "sqlconnection.h"

class UserService
{
public:
	UserService(shared_ptr<MysqlConnection> sql_conn);      //sql_conn用于建立到MySQL数据库的连接
	bool exist(string& mobile);                             //查找用户是否存在
	bool insert(string& mobile);                            //插入用户

private:
	shared_ptr<MysqlConnection> sql_conn_;
};

#endif