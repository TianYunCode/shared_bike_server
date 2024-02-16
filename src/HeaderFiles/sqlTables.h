#ifndef BRKS_COMMON_DATASEVER_SQLTABLES_H
#define BRKS_COMMON_DATASEVER_SQLTABLES_H

#include <memory>
#include "sqlconnection.h"
#include "glo_def.h"

class SqlTables 
{
public:
	SqlTables(shared_ptr<MysqlConnection> sqlConn);
	~SqlTables();

	bool CreateUserInfo();       //创建用户信息表
	bool CreateBikeTable();      //创建单车表

private:
	shared_ptr<MysqlConnection> sqlconn_;       //共享指针 当对象最后一次释放才会真正释放
};

#endif