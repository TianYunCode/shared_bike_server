#ifndef NS_IEVENT_HANDLER_H_
#define NS_IEVENT_HANDLER_H_

#include "ievent.h"
#include "eventtype.h"

class iEventHandler //接口类 被继承
{
public:
    virtual iEvent* handle(const iEvent* ev) { return NULL; };
    virtual ~iEventHandler() {};
    iEventHandler(const char* name) :name_(name) {};
    string& get_name() { return name_; };

private:
    string name_;
};

#endif