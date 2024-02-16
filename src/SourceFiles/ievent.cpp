#include "ievent.h"

iEvent::iEvent(u32 eid, u32 sn) : eid_(eid), sn_(sn)
{
}

iEvent::~iEvent()
{
}

//产生序列号
u32 iEvent::generateSeqNo() {
	static u32 sn = 0;
	return sn++;
}