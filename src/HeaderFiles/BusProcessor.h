#ifndef BRKS_BUS_MAIN_H_
#define BRKS_BUS_MAIN_H_

#include "user_event_handler.h"
#include "sqlconnection.h"

class BusinessProcessor {
public:
	BusinessProcessor(shared_ptr<MysqlConnection>conn);		//传进一个共享指针来初始化 mysqlconn_
	bool init();
	virtual ~BusinessProcessor();
private:
	shared_ptr<MysqlConnection>mysqlconn_;     //数据库连接的共享指针
	shared_ptr<UserEventHandler>ueh_;          //用户事件处理器的共享指针
};

#endif