/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 根据协议派发给响应的协议处理类
File Name   : c_DataCollection_task.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
****************************************************************************/

#ifndef C_DataCollection_TASK_H
#define C_DataCollection_TASK_H
//---------------------------------------------------------------------------
#include "cmpublic.h"
#include "cmpublic_net.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_Public.h"
#include "c_DataCollection_LWHandler.h"
#include "c_DataCollection_PadHandler.h"
#include "c_DataCollection_JTT808Handler.h"
#include "c_DataCollection_OBDHandler.h"
#include "c_DataCollection_Handle.h"
#include "c_DataCollection_log.h"

#include <map>
#include <vector>

using namespace std;

#define PKG_LENGTH 7

class TCDataCollectionTask : public TCThreadServerSock
{
private:
    long  m_nTimeOut;
    long  m_nProcessTime;
    
    long m_nCmd_Index;
    
    bool nLogined_Status;
    
    TCString m_sDev_ID;
    TCString m_sCust_ID;
    TCString m_sImei;
    TCString m_sImsi;

		//: 记录处理后，客户端未响应的操作序号以及报文内容；
		map<TCString, TCString> m_msOperID_PkgContent;
    
    TCString  m_sRecvTime;
    TCString  m_sSendTime;
    
    //: 终端上报IP地址；
    TCString sTacker_Send_IPAddress;
	
		//: 报文头的前缀 CWT
		TCString m_sPkg_Head_Flag;
		//: 报文头的内容的长度
		long m_nContent_Len;

		//: 报文内容域的整个内容
		TCString m_sPkg_Content;

		//: 报文内容域解析后的请求命令字
		TCString m_sReq_Command;
		
		//: 报文一次解析 （按照 & 分割解析） 后的字段列表
		TCStringList m_lsContentFieldList;
		
		//: 报文内容处理后的响应命令字
		TCString m_sRespCommand;
		
		//: 报文内容处理后的响应报文内容
		TCString m_sRespPkgContent;   
    
private:
    void Pkg_Handler();

    void Init();

private:
	   	

public:
    TCDataCollectionTask(int nSock);
    TCDataCollectionTask();
    ~TCDataCollectionTask();
    void ClientExecute();
};

#endif

