#include "eventtype.h"

//代号描述表
static EErrorReason EER[] =
{
	{ERRC_SUCCESS,              "Ok."},                     //ERRC_SUCCESS == 200
	{ERRC_INVALID_MSG,          "Invalid message."},        //ERRC_INVALID_MSG == 400
	{ERRC_INVALID_DATA,         "Invalid data."},           //ERRC_INVALID_DATA == 404
	{ERRC_METHOD_NOT_ALLOWED,   "Method not allowed."},     //ERRC_METHOD_NOT_ALLOWED == 405
	{ERRO_PROCCESS_FAILED,      "Proccess failed."},        //ERRO_PROCCESS_FAILED == 406
	{ERRO_BIKE_IS_TOOK,         "Bike is took."},           //ERRO_BIKE_IS_TOOK == 407
	{ERRO_BIKE_IS_RUNNING,      "Bike is running."},        //ERRO_BIKE_IS_RUNNING == 408
	{ERRO_BIKE_IS_DAMAGED,      "Bike is damaged."},        //ERRO_BIKE_IS_DAMAGED == 409
	{ERR_NULL,                  "Undefined"}                //ERR_NULL == 0
};

const char* getReasonByErrorCode(i32 code)      //查表 根据代号 查找对应的描述
{
	i32 i = 0;
	for (i = 0; EER[i].code != ERR_NULL; i++)
	{
		if (EER[i].code == code)
		{
			return EER[i].reason;
		}
	}

	return EER[i].reason;  //"Undefined"
}