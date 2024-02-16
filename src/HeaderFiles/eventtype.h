#ifndef BRKS_COMMON_EVENT_TYPE_H
#define BRKS_COMMON_EVENT_TYPE_H

#include "glo_def.h"

typedef struct EErrorReason_
{
	i32  code;
	const char* reason;
}EErrorReason;

/*事件ID*/
enum EventID
{
	EEVENTID_GET_MOBLIE_CODE_REQ        = 0x01,     //获取短信请求的事件ID      mobile_request
	EEVENTID_GET_MOBLIE_CODE_RSP        = 0x02,     //获取短信响应的事件ID      mobile_response

	EEVENTID_LOGIN_REQ                  = 0x03,     //登录验证请求的事件ID      login_request
	EEVENTID_LOGIN_RSP                  = 0x04,     //登录验证响应的事件ID      login_response

	EEVENTID_RECHARGE_REQ               = 0x05,     //充值请求的事件ID          recharge_request
	EEVENTID_RECHARGE_RSP               = 0x06,     //充值响应的事件ID          recharge_response

	EEVENTID_GET_ACCOUNT_BALANCE_REQ    = 0x07,     //余额查询请求的事件ID       account_balance_request
	EEVENTID_ACCOUNT_BALANCE_RSP        = 0x08,     //余额查询响应的事件ID       account_balance_response

	EEVENTID_LIST_ACCOUNT_RECORDS_REQ   = 0x09,     //充值查询请求的事件ID       list_account_records_request
	EEVENTID_ACCOUNT_RECORDS_RSP        = 0x10,     //充值查询响应的事件ID       list_account_records_response

	EEVENTID_LIST_TRAVELS_REQ           = 0x11,     //骑行记录查询请求的事件ID    list_travel_records_request
	EEVENTID_LIST_TRAVELS_RSP           = 0x12,     //骑行记录查询响应的事件ID    list_travel_records_response

	EEVENTID_EXIT_RSP                   = 0xFE,     //
	EEVENTID_UNKOWN                     = 0xFF      //
};

/*事件响应代号*/
enum EErrorCode
{
	ERRC_SUCCESS                = 200,      //响应成功
	ERRC_INVALID_MSG            = 400,      //无效的消息
	ERRC_INVALID_DATA           = 404,      //无效的数据
	ERRC_METHOD_NOT_ALLOWED     = 405,      //方法不允许 比如没有登录却查询账户余额
	ERRO_PROCCESS_FAILED        = 406,      //服务器端处理失败
	ERRO_BIKE_IS_TOOK           = 407,      //单车解锁
	ERRO_BIKE_IS_RUNNING        = 408,      //单车正在骑行
	ERRO_BIKE_IS_DAMAGED        = 409,      //单车损坏
	ERR_NULL                    = 0
};

const char* getReasonByErrorCode(i32 code);      //查表 根据代号 查找对应的描述

#endif