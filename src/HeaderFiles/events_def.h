#ifndef BRKS_COMMON_EVENTS_DEF_H_
#define BRKS_COMMON_EVENTS_DEF_H_

#include "ievent.h"
#include <string>
#include "bike.pb.h"

//手机验证码请求
class MobileCodeReqEv : public iEvent
{
public:
	MobileCodeReqEv(const string& mobile);

	const string& get_mobile() { return msg_.mobile(); };   //获取手机号
	virtual ostream& dump(ostream& out) const;
	virtual i32 ByteSize() {return msg_.ByteSize();}		//消息格式化后所占的长度
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };		//序列号到数组
    
private:
	tianyun::mobile_request msg_;   //protobuf 的短信请求对象

};

//手机验证码响应
class MobileCodeRspEv : public iEvent
{
public:
	MobileCodeRspEv(i32 code, i32 icode);

	const i32 get_code() { return msg_.code(); };
	const i32 get_icode() { return msg_.icode(); };
	const string& get_data() { return msg_.data(); };

	virtual ostream& dump(ostream& out) const;
	virtual i32 ByteSize() {return msg_.ByteSize();}		//消息格式化后所占的长度
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };		//序列号到数组

private:
	//string mobile_;
	tianyun::mobile_response msg_;	//protobuf 的短信请响应对象
};

//退出事件
class ExitRspEv :public iEvent
{
public:
	ExitRspEv() :iEvent(EEVENTID_EXIT_RSP, iEvent::generateSeqNo()){}
};

//登录请求
class LoginReqEv :public iEvent
{
public:
	LoginReqEv(const string& mobile, i32 icode);	//手机号和验证码

	const string& get_mobile() { return msg_.mobile(); }	//获取手机号
	const i32 get_icode() { return msg_.icode(); }			//获取验证码

	virtual ostream& dump(ostream& out)const;
	virtual i32 ByteSize() {return msg_.ByteSize(); }		//消息格式化后所占的长度
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); }		//序列号到数组
private:
	tianyun::login_request msg_;	//protobuf 的登录请求对象
};

//登录响应
class LoginRspEv :public iEvent 
{
public:
	LoginRspEv(i32 code);		//响应代号
	const i32 get_code() { return msg_.code(); }			//获取响应代号
	const string& get_desc() { return msg_.desc(); }		//获取失败描述信息

	virtual ostream& dump(ostream& out) const;
	virtual i32 ByteSize() { return msg_.ByteSize(); }		//消息格式化后所占的长度
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); }		//序列号到数组
	
private:
	tianyun::login_response msg_;	//protobuf 的登录响应对象
};

#endif