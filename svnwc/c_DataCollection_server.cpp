/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 主服务线程池程序
File Name   : c_DataCollection_server.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/
#pragma hdrstop

#include "c_DataCollection_server.h"
#include "c_DataCollection_task.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//==========================================================================
// 函数 : TCDataCollectionServer::TCDataCollectionServer
// 用途 : 构造函数
// 原型 : TCDataCollectionServer(TCString sHostName, TCString sService,
//                       long nPort, long nQueue,
//                       long nMinThread, long nMaxThread)
// 参数 : sHostName  -- 主机名
//        sService   -- 服务名
//        nPort      -- 端口
//        nQueue     -- Listen队列大小
//        nMinThread -- 最小线程数
//        nMaxThread -- 最大线程数
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionServer::TCDataCollectionServer(TCString sHostName, TCString sService,
                       long nPort, long nQueue,
                       long nMinThread, long nMaxThread):
                       TCServerThreadPool(sHostName, sService,
                                          nPort, nQueue,
                                          nMinThread, nMaxThread)
{
}

//==========================================================================
// 函数 : TCDataCollectionServer::~TCDataCollectionServer()
// 用途 : 析构函数
// 原型 : ~TCDataCollectionServer()
// 参数 : 无
// 返回 : 无
// 说明 :
//==========================================================================
TCDataCollectionServer::~TCDataCollectionServer()
{
    Clear();
}

//==========================================================================
// 函数 : TCDataCollectionServer::GetNewThreadSever
// 用途 : 
// 原型 : TCThreadServerSock * GetNewThreadSever(int nSock)
// 参数 : nSock -- 在监听的套接字
// 返回 : TCThreadServerSock指针
// 说明 :
//==========================================================================
TCThreadServerSock * TCDataCollectionServer::GetNewThreadSever(int nSock)
{
    TCThreadServerSock *pServerSock;
    try
    {
        pServerSock = new TCDataCollectionTask(nSock);
    }
    catch(...)
    {
        throw TCException("TCDataCollectionServer::GetNewThreadSever() Error:"
                          " can not create Server thread");
    }
    return pServerSock;
}

//==========================================================================
// 函数 : TCDataCollectionServer::DestroyAThreadServer
// 用途 : 清除服务线程
// 原型 : void DestroyAThreadServer(TCThreadServerSock *pServerSock)
// 参数 : pServerSock -- 服务线程指针
// 返回 : 无
// 说明 :
//==========================================================================
void TCDataCollectionServer::DestroyAThreadServer(TCThreadServerSock *pServerSock)
{
    delete (TCDataCollectionTask *)pServerSock;
}

