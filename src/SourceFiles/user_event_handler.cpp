#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "sqlconnection.h"
#include "iniconfig.h"
#include "user_service.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


UserEventHandler::UserEventHandler():iEventHandler("UserEventHandler")
{
	//未来需要实现订阅事件的处理
    thread_mutex_create(&pm_);  //初始化互斥锁变量

    //订阅事件
    DispatchMsgService::getInstance()->subscribe(EEVENTID_GET_MOBLIE_CODE_REQ, this);
    DispatchMsgService::getInstance()->subscribe(EEVENTID_LOGIN_REQ, this);
}

UserEventHandler::~UserEventHandler()
{
	//未来需要实现退订事件的处理
    thread_mutex_destroy(&pm_); //销毁互斥锁变量

    //退订事件
    DispatchMsgService::getInstance()->unsubscribe(EEVENTID_GET_MOBLIE_CODE_REQ, this);
    DispatchMsgService::getInstance()->unsubscribe(EEVENTID_LOGIN_REQ, this);
}

iEvent* UserEventHandler::handle(const iEvent* ev)
{
	if (ev == NULL)
	{
		//LOG_ERROR("input ev is NULL");
		printf("input ev is NULL");
	}

	u32 eid = ev->get_eid();    //获取传进 ev 事件对应的 eid（事件ID）

	if (eid == EEVENTID_GET_MOBLIE_CODE_REQ)    //判断是什么请求事件
	{
		return handle_mobile_code_req((MobileCodeReqEv*)ev);    //如果是 EEVENTID_GET_MOBLIE_CODE_REQ 请求 则调用对应的处理函数进行处理 返回值为请求对应的响应
	}
	else if (eid == EEVENTID_LOGIN_REQ)
	{
		return handle_login_req((LoginReqEv*) ev);
	}
	else if (eid == EEVENTID_RECHARGE_REQ)
	{
		//return handle_recharge_req((RechargeEv*) ev);
	}
	else if (eid == EEVENTID_GET_ACCOUNT_BALANCE_REQ)
	{
		//return handle_get_account_balance_req((GetAccountBalanceEv*) ev);
	}
	else if (eid == EEVENTID_LIST_ACCOUNT_RECORDS_REQ)
	{
		//return handle_list_account_records_req((ListAccountRecordsReqEv*) ev);
	}

	return NULL;
}

MobileCodeRspEv* UserEventHandler::handle_mobile_code_req(MobileCodeReqEv* ev)  //处理短信验证请求 输入请求 Req 返回响应 Rsp
{
	i32  icode = 0;     //定义一个暂存验证码的变量 并初始化
	string mobile = ev->get_mobile();  //获取该事件的手机号
	LOG_DEBUG("try to get moblie phone %s validate code .", mobile.c_str());     //输出该事件的手机号

    thread_mutex_lock(&pm_);    //加锁

	icode = code_gen();     	//产生一个验证码 并暂存
	m2c_[mobile] = icode;  		//把手机号和验证码保存到私有成员变量 m2c_ 中 mobile 为键 icode 为值

    thread_mutex_unlock(&pm_);  //解锁

	printf("mobile: %s, code: %d\n", mobile.c_str(), icode);   //输出手机号和验证码

	return new MobileCodeRspEv(ERRC_SUCCESS, icode);    //返回一个响应 事件响应代号为 200 成功 
}

i32 UserEventHandler::code_gen()    //产生验证码
{
	i32 code = 0;
	srand((unsigned int)time(NULL));

	code = (unsigned int)(rand() % (999999 - 100000) + 100000);

	return code;
}

 LoginRspEv* UserEventHandler::handle_login_req(LoginReqEv * ev)	//处理登录验证请求 输入请求 Req 返回响应 Rsp
 {
	LoginRspEv* loginEv = nullptr;

	string mobile = ev->get_mobile();	//拿出手机号码
	i32 icode = ev->get_icode();		//拿出验证码

	LOG_DEBUG("try to handle login ev, mobile =  %s, icode = %d\n", mobile.c_str(), icode);

	thread_mutex_lock(&pm_);    //加锁

	auto iter = m2c_.find(mobile);
	if(((iter!=m2c_.end()) && (icode!=iter->second)) || (iter==m2c_.end()))
	{
		loginEv = new LoginRspEv(ERRC_INVALID_DATA);	//返回的响应为无效的数据
	}

    thread_mutex_unlock(&pm_);  //解锁

	if(loginEv)		//如果不为空 就是被设置了无效数据 反之没有
	{
		return loginEv;
	}

	//如果验证成功 判断用户是否存在 不存在则插入用户记录（注册） 注：用户第一次登录共享单车会自动注册 登录即注册
	shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);			//创建 mysql 句柄

	st_env_config conf_args = Iniconfig::getInstance()->get_config();	//拿到配置文件信息
	//用配置信息初始化连接
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(), conf_args.db_passwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. EXIT!\n");
		return new LoginRspEv(ERRO_PROCCESS_FAILED);		//返回服务器处理失败响应
	}

	UserService us(mysqlconn);
	bool result = false;

	if (!us.exist(mobile))		//查询 mobile 用户是否存在
	{
		result = us.insert(mobile);		//用户不存在 插入一条用户信息（注册）
		if (!result)	//判断是否插入（注册）失败
		{
			LOG_ERROR("insert user mobile(%s) to db failed.", mobile.c_str());
			return new LoginRspEv(ERRO_PROCCESS_FAILED);	//返回服务器处理失败响应
		}
	}
	
	//查询成功或插入成功
	return new LoginRspEv(ERRC_SUCCESS);	//返回响应成功
 }