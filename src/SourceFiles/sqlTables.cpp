#include "sqlTables.h"

//传进一个共享指针 对成员共享指针初始化
SqlTables::SqlTables(shared_ptr<MysqlConnection> sqlConn) :sqlconn_(sqlConn)
{

}

SqlTables::~SqlTables()
{

}

//创建用户信息表
bool SqlTables::CreateUserInfo()
{
	const char* pUserInfoTable = " \
								CREATE TABLE IF NOT EXISTS userinfo( \
								id            int(16)               NOT NULL primary key auto_increment, \
								mobile        varchar(16)           NOT NULL default '13000000000',      \
								username      varchar(128)          NOT NULL default '',                 \
								verify        int(4)                NOT NULL default '0',                \
								registertm    timestamp			    NOT NULL default CURRENT_TIMESTAMP,  \
								money		  int(4)			    NOT NULL default 0,                  \
								INDEX         mobile_index(mobile) \
								)";

	if (!sqlconn_->Execute(pUserInfoTable))     //执行 pUserInfoTable 如果不成功 出错
	{
		LOG_ERROR("<SqlTable.h> : create table pUserInfoTable table failed. error msg : %s \n",sqlconn_->GetErrInfo());
		return false;
	}

	return true;
}

//创建单车表
bool SqlTables::CreateBikeTable()
{
    const char* pBikeInfoTable = " \
                                CREATE TABLE IF NOT EXISTS bikeinfo ( \
                                id					int				NOT    NULL primary key auto_increment, \
                                devno				int				NOT NULL, \
                                status				tinyint(1)		NOT NULL default 0, \
                                trouble				int				NOT NULL default 0, \
                                tmsg            	varchar(256)    NOT NULL default '',\
                                latitude			double(10,6)    NOT NULL default 0, \
                                longitude			double(10,6)    NOT NULL default 0, \
                                unique(devno) \
                                )";

	if (!sqlconn_->Execute(pBikeInfoTable))     //执行 pBikeInfoTable
	{
		LOG_ERROR("<SqlTable.h> : create table pBikeInfoTable table failed. error msg : %s \n", sqlconn_->GetErrInfo());
		return false;
	}

	return true;
}