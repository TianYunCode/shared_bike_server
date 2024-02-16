#include "DispatchMsgService.h"
#include <algorithm>

#include "bike.pb.h"
#include "events_def.h"

DispatchMsgService* DispatchMsgService::DMS_ = nullptr;
queue<iEvent*> DispatchMsgService::response_events;
pthread_mutex_t   DispatchMsgService::queue_mutex;

DispatchMsgService::DispatchMsgService() :tp(nullptr),svr_exit_(FALSE)
{
    //tp = NULL;
    //NTIF_ = NULL;
}

DispatchMsgService::~DispatchMsgService()
{
}

BOOL DispatchMsgService::open()
{
    svr_exit_ = FALSE;          //代表服务未退出

    thread_mutex_create(&queue_mutex);      //初始化互斥锁
    tp = thread_pool_init();    //初始化线程池

    return tp ? TRUE : FALSE;   //如果初始化成功返回 TRUE 否则返回 FALSE
}

void DispatchMsgService::close()
{
    svr_exit_ = TRUE;           //代表服务退出

    thread_pool_destroy(tp);    //销毁线程池
    thread_mutex_destroy(&queue_mutex);     //销毁互斥锁
    subscribers_.clear();       //

    tp = nullptr;               //将线程池指针至空 防止操作野指针
}

//单例模式
DispatchMsgService* DispatchMsgService::getInstance() 
{
    if (DMS_ == nullptr)        //如果对象没有创建则创建一个对象 否则直接返回对象本身 从而保证该类只会有一个对象
    {
        DMS_ = new DispatchMsgService();
    }

    return DMS_;                //返回对象本身
}

//线程池回调函数 设为 static 的原因是因为 C++ 的调用方式会在形参加 this 指针 设置为静态则不会
void DispatchMsgService::svc(void* argv)
{
    DispatchMsgService* dms = DispatchMsgService::getInstance();    //获取实例
    iEvent* ev = (iEvent*)argv;     //
    if (!dms->svr_exit_)    //如果事件没有结束
    {
        LOG_DEBUG("DispatchMsgService::svc ...\n");
        iEvent* rsp = dms->process(ev);

        if (rsp)    //判断返回的响应是否存在
        {
            rsp->dump(cout);
            rsp->set_args(ev->get_args());
        }
        else
        {
            //生成终止响应事件
            rsp = new ExitRspEv();
            rsp->set_args(ev->get_args());
        }

        thread_mutex_lock(&queue_mutex);
        response_events.push(rsp);
        thread_mutex_unlock(&queue_mutex);
        //delete ev;
    }
}

//把事件投递到线程池中进行处理
i32 DispatchMsgService::enqueue(iEvent* ev)
{
    if (NULL == ev) //防御性编程
    {
        return -1;
    }

    thread_task_t* task = thread_task_alloc(0);

    task->handler = DispatchMsgService::svc;    //设置回调函数
    task->ctx = ev;                             //设置回调时带的参数

    return thread_task_post(tp, task);          //把任务投体到线程池

    //return msg_queue_.enqueue(ev, 0);
}

//事件订阅函数 eid 是事件ID handler 是事件处理器
void DispatchMsgService::subscribe(u32 eid, iEventHandler* handler)
{
    LOG_DEBUG("DispatchMsgService::subscribe eid: %u\n", eid);
    T_EventHandlersMap::iterator iter = subscribers_.find(eid);     //调用 find() 函数查找事件ID
    if (iter != subscribers_.end())     //如果迭代器不为空 说明该事件有注册 但是否是 handler 事件处理器还需往下走 如果为空 直接绑定事件处理器
    {
        // second 就是 T_EventHandlers容器（事件处理器组）
        T_EventHandlers::iterator hdl_iter = find(iter->second.begin(), iter->second.end(), handler);   //查找 handler 是否存在 如果不存在 返回 iter->second.end()
        if (hdl_iter == iter->second.end())     //判断 hdl_iter 是否不存在 如果不存在则绑定 handler 事件处理器
        {
            iter->second.push_back(handler);    //把 handler 添加到 T_EventHandlers容器（事件处理器组）中
        }
    }
    else    //事件ID为空 直接添加事件ID 并绑定事件处理器
    {
        subscribers_[eid].push_back(handler);   //把 handler 添加到 T_EventHandlers容器（事件处理器组）中
    }
}

//事件退订函数 eid 是事件ID handler 是事件处理器
void DispatchMsgService::unsubscribe(u32 eid, iEventHandler* handler)
{
    T_EventHandlersMap::iterator iter = subscribers_.find(eid);     //调用 find() 函数查找事件ID
    if (iter != subscribers_.end())     //如果迭代器不为空 代表找到事件ID 否则代表事件ID不存在 不存在就无需做任何处理 
    {
        T_EventHandlers::iterator hdl_iter = find(iter->second.begin(), iter->second.end(), handler);   //查找 handler 是否存在 如果不存在 返回 iter->second.end()
        if (hdl_iter != iter->second.end())     //如果 hdl_iter 不等于 iter->second.end() 代表找到该事件处理器
        {
            iter->second.erase(hdl_iter);       //把 handler 从事件处理器组中删除
        }
    }
}

//对具体的事件进行分发处理 由线程池回调
iEvent* DispatchMsgService::process(const iEvent* ev)
{
    LOG_DEBUG("DispatchMsgService::process - ev: %p\n", ev);
    if (NULL == ev)
    {
        return NULL;
    }

    u32 eid = ev->get_eid();    //暂存事件ID

    LOG_DEBUG("DispatchMsgService::process - eid: %u\n", eid);

    if (eid == EEVENTID_UNKOWN) //如果事件ID是无效的 就返回空
    {
        LOG_WARN("DispatchMsgService : unknow evend id %d", eid);
        return NULL;
    }

    T_EventHandlersMap::iterator handlers = subscribers_.find(eid);     //查找在 subscribers_ 是否存在这个事件ID
    if (handlers == subscribers_.end())     //如果查找结果为 subscribers_.end() 代表没找到 不存在 则返回空
    {
        LOG_WARN("DispatchMsgService : no any event handler subscribed %d", eid);
        return NULL;
    }

    iEvent* rsp = NULL;
    for (auto iter = handlers->second.begin(); iter != handlers->second.end(); iter++)      //遍历
    {
        iEventHandler* handler = *iter;
        LOG_DEBUG("DispatchMsgService : get handler: %s\n", handler->get_name().c_str());

        rsp = handler->handle(ev);      //推荐使用vector 或list 返回多个rsp 
    }

    return rsp;
}

//解析事件
iEvent* DispatchMsgService::parseEvent(const char* message, u32 len, u32 eid)
{
    if (!message)       //如果消息为空 出错
    {
        LOG_ERROR("DispatchMsgService::parseEvent - message is null[eid: %d].\n", eid);
        return nullptr;
    }

    //判断事件ID是什么
    if (eid == EEVENTID_GET_MOBLIE_CODE_REQ)        //是否为 获取短信请求
    {
        tianyun::mobile_request  mr;
        if (mr.ParseFromArray(message, len))        //解析 成功返回 true 失败返回 false
        {
            MobileCodeReqEv* ev = new MobileCodeReqEv(mr.mobile());     //创建短信请求事件
            return ev;
        }
    }
    else if (eid == EEVENTID_LOGIN_REQ)             //是否为 登录验证请求
    {
        tianyun::login_request lr;
        if (lr.ParseFromArray(message, len))        //解析 成功返回 true 失败返回 false
        {
            LoginReqEv* ev = new LoginReqEv(lr.mobile(),lr.icode());    //创建登录请求事件
            return ev;
        }
    }

    return nullptr;
}

//处理所有响应事件
void DispatchMsgService::handleAllResponseEvent(NetworkInterface* interface)
{
    bool is_done = false;      //因为队列里有多个元素 设置循环条件 

    while (!is_done)           //循环判断是否处理完成
    {
        iEvent* ev = nullptr;   //事件指针
        thread_mutex_lock(&queue_mutex);
        if (!response_events.empty())   //判断队列是否为空
        {
            ev = response_events.front();   //获取队列第一个元素
            response_events.pop();          //队首元素出栈
        }
        else    //为空则处理完成
        {
            is_done = true;
        }
        thread_mutex_unlock(&queue_mutex);

        if (!is_done)       //判断是否处理完成
        {
            //判断事件类型
            if (ev->get_eid() == EEVENTID_GET_MOBLIE_CODE_RSP)      //判断是否是 获取短信响应
            {
                MobileCodeRspEv* mcre = static_cast<MobileCodeRspEv*>(ev);  //把队列获取到的事件强转为 MobileCodeRspEv 类型
                LOG_DEBUG("DispatchMsgService::handleAllResponseEvent - id:EEVENTID_GET_MOBLIE_CODE_RSP\n");

                ConnectSession* cs = (ConnectSession*)ev->get_args();   //拿到连接会话
                cs->response = ev;      //把队列获取到的事件作为会话的响应

                //系列化请求数据
                cs->message_len = mcre->ByteSize();     //消息格式化后所占的长度
                cs->write_buf = new char[cs->message_len + MESSAGE_HEADER_LEN];     //为发送的数据申请空间 消息格式化后长度加上头部长度


                //组装头部
                int message_head_len = sizeof(MESSAGE_HEADER_ID);
                memcpy(cs->write_buf, MESSAGE_HEADER_ID, message_head_len);
                *(u16*)(cs->write_buf + message_head_len) = EEVENTID_GET_MOBLIE_CODE_RSP;
                *(i32*)(cs->write_buf + 6) = cs->message_len;   

                mcre->SerializeToArray(cs->write_buf + MESSAGE_HEADER_LEN, cs->message_len);    //序列号到数组

                interface->send_response_message(cs);       //发送响应消息
            }
            else if (ev->get_eid() == EEVENTID_EXIT_RSP)    //判断是否是 退出响应
            {
                ConnectSession* cs = (ConnectSession*)ev->get_args();
                cs->response = ev;
                interface->send_response_message(cs);       //发送响应消息
            }
        }
    }
}
