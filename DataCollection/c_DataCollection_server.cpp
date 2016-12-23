/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : �������̳߳س���
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
// ���� : TCDataCollectionServer::TCDataCollectionServer
// ��; : ���캯��
// ԭ�� : TCDataCollectionServer(TCString sHostName, TCString sService,
//                       long nPort, long nQueue,
//                       long nMinThread, long nMaxThread)
// ���� : sHostName  -- ������
//        sService   -- ������
//        nPort      -- �˿�
//        nQueue     -- Listen���д�С
//        nMinThread -- ��С�߳���
//        nMaxThread -- ����߳���
// ���� :
// ˵�� :
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
// ���� : TCDataCollectionServer::~TCDataCollectionServer()
// ��; : ��������
// ԭ�� : ~TCDataCollectionServer()
// ���� : ��
// ���� : ��
// ˵�� :
//==========================================================================
TCDataCollectionServer::~TCDataCollectionServer()
{
    Clear();
}

//==========================================================================
// ���� : TCDataCollectionServer::GetNewThreadSever
// ��; : 
// ԭ�� : TCThreadServerSock * GetNewThreadSever(int nSock)
// ���� : nSock -- �ڼ������׽���
// ���� : TCThreadServerSockָ��
// ˵�� :
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
// ���� : TCDataCollectionServer::DestroyAThreadServer
// ��; : ��������߳�
// ԭ�� : void DestroyAThreadServer(TCThreadServerSock *pServerSock)
// ���� : pServerSock -- �����߳�ָ��
// ���� : ��
// ˵�� :
//==========================================================================
void TCDataCollectionServer::DestroyAThreadServer(TCThreadServerSock *pServerSock)
{
    delete (TCDataCollectionTask *)pServerSock;
}

