#ifndef NS_EVENT_H_
#define NS_EVENT_H_

#include "glo_def.h"
#include "eventtype.h"
#include <string>
#include <iostream>

using namespace std;

class iEvent
{
public:
	iEvent(u32 eid, u32 sn);

	virtual ~iEvent();
	virtual ostream& dump(ostream& out) const { return out; };
	virtual  i32 ByteSize() { return 0; };		//消息格式化后所占的长度
	virtual bool SerializeToArray(char* buf, int len) { return true; };		//序列号到数组

	u32 generateSeqNo();    //产生序列号
	
	u32 get_eid() const { return eid_; };   //获取事件ID
	void set_eid(u32 eid) { eid_ = eid; };  //设置事件ID

	u32 get_sn()  const { return sn_; };    //获取序列号

	void set_args(void* args) { args_ = args; };
	void* get_args() { return args_; };

private:
	u32 eid_;   	//事件ID
	u32 sn_;    	//序列号
	void* args_;	//
};

#endif