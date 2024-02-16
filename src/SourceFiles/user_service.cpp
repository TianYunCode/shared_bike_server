#include "user_service.h"

UserService::UserService(shared_ptr<MysqlConnection> sql_conn) :sql_conn_(sql_conn)
{


}

//查找用户是否存在
bool UserService::exist(string& mobile)
{
	char sql_content[1024] = { 0 };     //SQL 语句

	sprintf(sql_content, "SELECT * FROM userinfo WHERE mobile = %s", mobile.c_str());   //格式化储存到 sql_content 中

	SqlRecordSet record_set;    //创建一个结果集
	if (!sql_conn_->Execute(sql_content, record_set))   //执行 sql_content 结果存在 record_set 中 判断是否执行成功
	{
		return false;   //查询失败 返回 false
	}

    //查询成功
	return (record_set.GetRowCount() != 0);     //获取结果集的行数 大于等于1代表有数据存在
}

//插入用户
bool UserService::insert(string& mobile)
{
	char sql_content[1024] = { 0 };     //SQL 语句

	sprintf(sql_content, "INSERT INTO userinfo (mobile) VALUES (%s)", mobile.c_str());     //格式化储存到 sql_content 中

    return sql_conn_->Execute(sql_content);     //执行插入语句 成功返回 true 执行失败返回 false
}