/**************************************************************************************************
* 负责分发消息服务模块，其实就是把外部收到的消息，转化成内部事件，也就是 data->msg->event 的解码过程，
* 然后再把事件投递至线池的消息队列，由线程池调用其 process 方法对事件进行处理，最终调用每个 event 的 handler 方法
* 来处理 event，此时每个 event handler 需要 subscribe 该 event 后才会被调用到.
***************************************************************************************************/

#ifndef BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_
#define BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_

#include <map>
#include <vector>
#include <queue>
#include "ievent.h"
#include "eventtype.h"
#include "iEventHandler.h"
#include "thread_pool.h"
#include "NetworkInterface.h"
#include "Logger.h"

class DispatchMsgService
{
protected:
    DispatchMsgService();
public:

    virtual ~DispatchMsgService();

    virtual BOOL open();
    virtual void close();

    virtual void subscribe(u32 eid, iEventHandler* handler);    //事件订阅函数 eid 是事件ID handler 是事件处理器
    virtual void unsubscribe(u32 eid, iEventHandler* handler);  //事件退订函数 eid 是事件ID handler 是事件处理器

    
    virtual i32 enqueue(iEvent* ev);            //把事件投递到线程池中进行处理

    static void svc(void* argv);                //线程池回调函数 设为 static 的原因是因为 C++ 的调用方式会在形参加 this 指针 设置为静态则不会

    virtual iEvent* process(const iEvent* ev);  //对具体的事件进行分发处理

    static DispatchMsgService* getInstance();   //单例模式

    iEvent* parseEvent(const char* message, u32 len, u32 eid);      //解析事件

    void handleAllResponseEvent(NetworkInterface* interface);       //处理所有响应事件

protected:

    thread_pool_t* tp;  //线程池 处理事件

    static DispatchMsgService* DMS_;    //单例模式的唯一对象

    typedef vector<iEventHandler*> T_EventHandlers;         //因为一个事件可以被多个事件处理器处理 T_EventHandlers 类型可以保存多个事件处理器 也就是事件处理器组
    typedef map<u32, T_EventHandlers > T_EventHandlersMap;  //保存多个事件ID所对应的一个或多个事件处理器 第一个参数表示键 事件ID 第二个参数表示值 事件处理器组
    T_EventHandlersMap subscribers_;                        //T_EventHandlersMap 数据类型的成员变量 用于保存记录多个事件ID所对应的事件处理器组

    bool svr_exit_;     //标记 代表服务状态 初始化为 false false 代表服务没有退出 true 代表服务退出

    static queue<iEvent*> response_events;    //队列 处理完请求的响应事件
    static pthread_mutex_t   queue_mutex;     //线程互斥锁 锁队列
};

#endif
