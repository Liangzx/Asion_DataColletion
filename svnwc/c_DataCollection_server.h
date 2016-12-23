/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 主服务线程池程序
File Name   : c_DataCollection_server.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#ifndef C_DataCollection_SERVER_H
#define C_DataCollection_SERVER_H
//---------------------------------------------------------------------------
#include "c_server_thread_pool.h"
class TCDataCollectionServer : public TCServerThreadPool
{
public:
    TCDataCollectionServer(){}
    TCDataCollectionServer(TCString sHostName, TCString sService,
                       long nPort, long nQueue,
                       long nMinThread, long nMaxThread);
    ~TCDataCollectionServer();
    TCThreadServerSock * GetNewThreadSever(int nSock);
    void DestroyAThreadServer(TCThreadServerSock *pServerSock);
};
#endif
