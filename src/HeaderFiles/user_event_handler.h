#ifndef BRKS_BUS_USERM_HANDLER_H_
#define BRKS_BUS_USERM_HANDLER_H_

#include "glo_def.h"
#include "iEventHandler.h"
#include "events_def.h"
#include "thread.h"
#include "DispatchMsgService.h"

class UserEventHandler : public iEventHandler
{
public:
    UserEventHandler();
    virtual ~UserEventHandler();
    virtual iEvent* handle(const iEvent* ev);   //在 iEventHandler 接口类中定义 子类必须实现

private:
    MobileCodeRspEv* handle_mobile_code_req(MobileCodeReqEv* ev);   //处理短信验证请求 输入请求 Req 返回响应 Rsp
    LoginRspEv* handle_login_req(LoginReqEv * ev);                  //处理登录验证请求 输入请求 Req 返回响应 Rsp

    i32 code_gen();     //产生验证码

private:
    //string mobile_;
    //用哈希表保存手机号码对应的验证码
    map<string, i32> m2c_;  //第一个参数是手机号码 第二个参数是验证码
    pthread_mutex_t pm_;    //互斥锁变量
};

#endif