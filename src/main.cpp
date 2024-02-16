#include <iostream>
#include <unistd.h>
#include <memory>

#include "iniconfig.h"
#include "configdef.h"
#include "Logger.h"

#include "bike.pb.h"
#include "ievent.h"
#include "events_def.h"
#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "sqlconnection.h"
#include "BusProcessor.h"

using namespace std;

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        printf("Please input shared_bike<config file path> <log file config>!\n");
        return -1;
    }

    if(!Logger::instance()->init((string)argv[2]))   //如果初始化日志模块失败
    {
        fprintf(stderr, "init log module failed.\n");
        return -2;
    }

    Iniconfig* config = Iniconfig::getInstance();
    bool is_load_config = config->load_file((string)argv[1]);

    if(!is_load_config) //如果加载配置文件失败
    {
        //printf("load %s failed.\n", argv[1]);
        LOG_ERROR("load %s failed.\n", argv[1]);    //初始化日志模块成功后 就可以使用 LOG_ERROR 了
        return -3;
    }

    st_env_config config_info = config->get_config();

    LOG_INFO("[database]:ip = %s, [database]:port = %u, [database]:user = %s, [database]:passwd = %s, [database]:name = %s, [server]:port = %u\n", 
    config_info.db_ip.c_str(), config_info.db_port, config_info.db_user.c_str(), config_info.db_passwd.c_str(), config_info.db_name.c_str(), config_info.server_port);

    tianyun::mobile_request msg;
    msg.set_mobile("15007819636");
    cout<<msg.mobile()<<endl;

    //iEvent* ie = new iEvent(EEVENTID_GET_MOBLIE_CODE_REQ,2);

    MobileCodeReqEv me("15007819636");
    me.MobileCodeReqEv::dump(cout);

    //测试用例
    // {
    //     cout<<"-----------------------"<<endl;
    //     MobileCodeRspEv mcre1(ERRC_SUCCESS,666666);
    //     mcre1.dump(cout);
    //     cout<<"-----------------------"<<endl;

    //     MobileCodeRspEv mcre2(ERRO_PROCCESS_FAILED,777777);
    //     mcre2.dump(cout);
    //     cout<<"-----------------------"<<endl;

    //     MobileCodeRspEv mcre3(ERRO_BIKE_IS_DAMAGED,888888);
    //     mcre3.dump(cout);
    //     cout<<"-----------------------"<<endl;
    // }

    //MysqlConnection mysql_conn;
    shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
    if(!mysqlconn->Init(config_info.db_ip.c_str(),config_info.db_port,config_info.db_user.c_str(),config_info.db_passwd.c_str(),config_info.db_name.c_str()))
    {
        LOG_ERROR("Database init failed. exit!\n");
        return -4;
    }

    BusinessProcessor busPro(mysqlconn);
    busPro.init();
    //UserEventHandler ueh1;
    //ueh1.handle(&me);

    DispatchMsgService* DMS = DispatchMsgService ::getInstance();
    DMS->open();
    // MobileCodeReqEv *pmcre = new MobileCodeReqEv("15007819636");
    // DMS->enqueue(pmcre);

    NetworkInterface* NTIF = new NetworkInterface();
    NTIF->start(config_info.server_port);

    int i = 1;
    while(1)
    {
        NTIF->network_event_dispatch();
        sleep(1);
        LOG_DEBUG("network_event_dispatch...[%d]\n", i);
        i++;
    }

    sleep(5);
    DMS->close();
    sleep(5);

    return 0;
}