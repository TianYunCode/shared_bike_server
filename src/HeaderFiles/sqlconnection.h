#ifndef DATASTORE_MYSQL_CONNECTION_H_
#define DATASTORE_MYSQL_CONNECTION_H_

#include <mysql/mysql.h>
#include <string>
#include <string.h>
#include <mysql/errmsg.h>
#include <assert.h>

#include "glo_def.h"
#include "Logger.h"

//封装结果集 MYSQL_RES *
class SqlRecordSet 
{
public:
	SqlRecordSet();

	explicit SqlRecordSet(MYSQL_RES* pRes);     // 不能隐式构造

	MYSQL_RES* MysqlRes();                      //返回结果集指针

	~SqlRecordSet();

	inline void SetResult(MYSQL_RES* pRes);     //设置结果集

	inline MYSQL_RES* GetResult();              //获取结果集

	void FetchRow(MYSQL_ROW& row);              //获取里面的行

	i32 GetRowCount();                   //返回结果集具体行的数量

private:
	MYSQL_RES* m_pRes;  //MySQL 结果集的指针变量
};

class MysqlConnection
{
public:
	MysqlConnection();
	~MysqlConnection();

	MYSQL* Mysql();      //获取 mysql 的句柄

    //初始化
	bool Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb);

    //执行 SQL 语句 包括增删改查 不需要拿到结果集
	bool Execute(const char* szSql);

	//执行 SQL 语句 包括增删改查 需要拿到结果集 结果集保存在 recordSet 在 MySQL 中 结果集的类型是 MYSQL_RES *
	bool Execute(const char* szSql, SqlRecordSet& recordSet);
	
	//将 pSrc 特殊字符进行转义 一些特殊字符如果不转义 sql 查询就会报错
	int EscapeString(const char* pSrc, int nSrcLen, char* pDest);

	void Close();

	//得到错误信息的方法
	const char* GetErrInfo();

	//服务断掉了,重连
	void Reconnect();

private:
	MYSQL* mysql_;  //mysql 的句柄 用于操作数据库
};

#endif