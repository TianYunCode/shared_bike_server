#include "sqlconnection.h"

SqlRecordSet::SqlRecordSet() : m_pRes(NULL)
{

}

SqlRecordSet::SqlRecordSet(MYSQL_RES* pRes)// 不能隐式构造
{
	m_pRes = pRes;
}

MYSQL_RES* SqlRecordSet::MysqlRes()   //返回结果集指针
{
	return m_pRes;
}

SqlRecordSet::~SqlRecordSet() 
{
	if (m_pRes)     //结果集为真才执行
	{
		mysql_free_result(m_pRes);      //释放结果集
	}
}

/*
* 你已经设置了结果集,此时若要再次设置结果集,那么之前的结果集就访问不到了.(之前的结果集还没来的及释放,就是内存泄漏)
* 所以,你要设置结果集的前提是:结果集是空的.
* 不是空的,咱们不让他设置
*/
//设置结果集
inline void SqlRecordSet::SetResult(MYSQL_RES* pRes)
{
	//如果此时已经保存了结果集,那么就应该让程序报错,防止内存泄漏
	assert(m_pRes == NULL);     //断言结果集为空 否则出错
	if (m_pRes)
	{
		LOG_WARN("the MYSQL_RES has already stored result , maybe will case memory leak\n");
	}
	m_pRes = pRes;
}

//获取结果集
inline MYSQL_RES* SqlRecordSet::GetResult()
{
	return m_pRes;
}

//获取里面的行
void SqlRecordSet::FetchRow(MYSQL_ROW& row)
{
	row = mysql_fetch_row(m_pRes);
}

//返回结果集具体行的数量
i32 SqlRecordSet::GetRowCount()
{
	return m_pRes->row_count;
}

/*---------------------------------------------------MysqlConnection-------------------------------------------------------*/

MysqlConnection::MysqlConnection() : mysql_(nullptr)
{
    mysql_ = (MYSQL*)malloc(sizeof(MYSQL));
}

MysqlConnection::~MysqlConnection()
{
    if (mysql_ != NULL)
    {
        mysql_close(mysql_);
        free(mysql_);
        mysql_ = NULL;
    }

    return;
}

//获取 mysql 的句柄
MYSQL* MysqlConnection::Mysql()
{
	return mysql_;
}

//初始化
bool MysqlConnection::Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb)
{
    LOG_INFO("enter Init.\n");

    //初始化
    if (mysql_init(mysql_) == NULL) {
        LOG_ERROR("init mysql failed %s , %d",this->GetErrInfo(),errno);
        return false;
    }

    //连接
    if (mysql_real_connect(mysql_, szHost, szUser, szPasswd, szDb, nPort, NULL, 0) == NULL)
    {
        LOG_ERROR("connect mysql failed : %s ", this->GetErrInfo());
    }

    //MYSQL_OPT_RECONNECT 是网络等原因 断开后自动重连
    char cAuto = 1;
    if (!mysql_options(mysql_ , MYSQL_OPT_RECONNECT , &cAuto) )
    {
        LOG_ERROR("MysqlConnection::Init:mysql_options MYSQL_OPT_RECONNEC failed.(因网络等原因，断开后自动重连，如果数据库还可操作就不必理会)");
    }

    return true;
}

//执行 SQL 语句 包括增删改查 不需要拿到结果集
bool MysqlConnection::Execute(const char* szSql)
{
    if ( mysql_real_query(mysql_, szSql, strlen(szSql)) != 0 )
    {
        if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR )   //断开连接就重连
        {
            Reconnect();    //重连函数
        }
        return false;
    }

    return true;
}

//执行 SQL 语句 包括增删改查 需要拿到结果集 结果集保存在 recordSet 在 MySQL 中 结果集的类型是 MYSQL_RES *
bool MysqlConnection::Execute(const char* szSql, SqlRecordSet& recordSet)
{
    //先进行 sql 查询 看是否能够执行成功
    if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
    {
        if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
        {
            Reconnect();
        }

        return false;
    }

    //执行成功了 就将查到的结果设置到结果集中
    MYSQL_RES* pRes = mysql_store_result(mysql_);
    if (!pRes) 
    {
        return NULL;        //设置失败 返回空 说明 mysql_store_result 失败了
    }
    recordSet.SetResult(pRes);      //将结果放入结果集

    return true;
}
	
//将 pSrc 特殊字符进行转义 一些特殊字符如果不转义 sql 查询就会报错
int MysqlConnection::EscapeString(const char* pSrc, int nSrcLen, char* pDest)   //pSrc 是转义前的字符 pDest 是转义后的字符
{
    if (!mysql_)
    {
        return 0;
    }

    return mysql_real_escape_string(mysql_,pDest,pSrc,nSrcLen);     //将源 src 转义到目标子串 dest
}

void MysqlConnection::Close()
{

}

//得到错误信息的方法
const char* MysqlConnection::GetErrInfo()
{
    return mysql_error(mysql_);     //返回上一个 MySQL 操作产生的文本错误信息
}

//服务断掉了 重连
void MysqlConnection::Reconnect()
{
    mysql_ping(mysql_);     //重新连接 MySQL
}