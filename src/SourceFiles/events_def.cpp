#include "events_def.h"
#include <iostream>
#include <sstream>

//手机验证码请求 构造函数
MobileCodeReqEv::MobileCodeReqEv(const string& mobile) :iEvent(EEVENTID_GET_MOBLIE_CODE_REQ, iEvent::generateSeqNo())
{
	msg_.set_mobile(mobile);
}

//手机验证码请求 dump 函数
std::ostream& MobileCodeReqEv::dump(std::ostream& out) const
{
	out << "MobileCodeReq sn=" << get_sn() << ",";  //输出序列号
	out << "mobile=" << msg_.mobile() << endl;      //输出手机号
	return out;
}

//手机验证码响应 构造函数
MobileCodeRspEv::MobileCodeRspEv(i32 code, i32 icode) :iEvent(EEVENTID_GET_MOBLIE_CODE_RSP, iEvent::generateSeqNo()) 
{
    printf("code: %d , icode:%d\n", code, icode);   //输出代号和验证码
	const char* tmp = getReasonByErrorCode(code);   //获取代号描述
	printf("tmp:%s\n", tmp);    //输出代码描述

	msg_.set_code(code);
	msg_.set_icode(icode);
	msg_.set_data(tmp);     //把代号描述设置到 portobuf 生成的 data 成员变量
}

//手机验证码响应 dump 函数
ostream& MobileCodeRspEv::dump(ostream& out) const
{
    out << "MobileCodeReqEv sn =" << get_sn() << ",";   //输出序列号
	out << "code=" <<msg_.code() << endl;               //输出响应代号          //出现问题,一旦调用,code_与icode就会变为随机值
	out << "icode=" << msg_.icode() << endl;            //输出验证码
	out << "describle = " << msg_.data() << endl;       //输出代号描述
	return out;
}

//登录请求构造函数
LoginReqEv::LoginReqEv(const string& mobile, i32 icode) : iEvent(EEVENTID_LOGIN_REQ, iEvent::generateSeqNo()) 
{
	msg_.set_mobile(mobile);
	msg_.set_icode(icode);
}

//登录请求 dump 函数
ostream& LoginReqEv::dump(std::ostream& out) const
{
	out << "LoginRe<q>Ev sn = " << get_sn() << ",";
	out << "mobile = " << msg_.mobile() << std::endl;
	out << "icode = " << msg_.icode() << std::endl;
	return out;
}

//登录响应构造函数
LoginRspEv::LoginRspEv(i32 code) : iEvent(EEVENTID_LOGIN_RSP,iEvent::generateSeqNo())
{
	msg_.set_code(code);
	msg_.set_desc(getReasonByErrorCode(code));
}

//登录响应 dump 函数
ostream& LoginRspEv::dump(std::ostream& out) const
{
	out << "LoginRe<s>Ev sn = " << get_sn() << ",";
	out << "code=" << msg_.code() << std::endl;
	out << "describe = " << msg_.desc() << std::endl;
	return out;
}