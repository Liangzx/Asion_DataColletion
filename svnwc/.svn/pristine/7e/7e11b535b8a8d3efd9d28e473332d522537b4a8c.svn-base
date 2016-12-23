/*****************************************************************************
CAPTION
           Project Name: 乌东德定位系统--数据采集模块
           Description : 根据协议派发给相应的协议处理类
           File Name   : c_DataCollection_task.cpp
           Requires    : LinuxAS3 , Oracle 9i
           Create Date : 2016-05-04
******************************************************************************/

#pragma hdrstop

#include "c_DataCollection_task.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_log.h"

#pragma package(smart_init)

extern TCDataCollectionConfig *gDataCollectionConfig;
extern TCDataCollectionLog  *glgMls; 

//==========================================================================
// 函数 : TCDataCollectionTask::TCDataCollectionTask
// 用途 : 构造函数
// 原型 : TCDataCollectionTask(int nSock)
// 参数 : 已连接的套接字
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionTask::TCDataCollectionTask(int nSock) : TCThreadServerSock(nSock){
}

//==========================================================================
// 函数 : TCDataCollectionTask::TCDataCollectionTask
// 用途 : 构造函数
// 原型 : TCDataCollectionTask()
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionTask::TCDataCollectionTask(){
   
}

//==========================================================================
// 函数 : TCDataCollectionTask::~TCDataCollectionTask()
// 用途 : 析构函数
// 原型 : ~TCDataCollectionTask()
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionTask::~TCDataCollectionTask()
{
}

//==========================================================================
// 函数 : TCDataCollectionTask::ClientExecute
// 用途 : 主要实现具体的业务逻辑，由客户端重载
// 原型 : void ClientExecute()
// 参数 :
// 返回 :
// 说明 : 应用程序需继承此类并重载此函数
//==========================================================================
void TCDataCollectionTask::ClientExecute(){
	try{
		//: 远程发送的地址
		sTacker_Send_IPAddress=m_cusSocket.GetRemoteAddress();
		//: 登录状态为未登录;
		nLogined_Status=false;
		m_nTimeOut=60;
		
		printf("Recv Tracker Requst: IP=[%s], Port=[%d], ThreadID=[%s]， Timeout=[%d]\n", (char*)m_cusSocket.GetRemoteAddress(), m_cusSocket.GetRemotePort(), (char*)IntToStr(GetThreadID()),  m_nTimeOut);
		 
		m_sRecvTime=TCTime::Now();
		Init();
 		Pkg_Handler();
 		m_cusSocket.Close();
 		
 		printf("ThreadID=[%s] Handle End\n", (char*)IntToStr(GetThreadID()));
		
  }catch(TCException &e){
    TCString sLogMsg = TCString("At Clientexec 处理请求时出错:") + e.GetExceptionMessage();
    glgMls->AddDataCollectionRunLog(sLogMsg);
    m_cusSocket.Close();
  }catch(...){
    glgMls->AddDataCollectionRunLog("At Clientexec Unknow, 关闭Socket连接");
    m_cusSocket.Close();
 	}
}

//==========================================================================
// 函数 : TCDataCollectionTask::Pkg_Handler()
// 用途 : 接收请求
// 原型 : void Pkg_Handler()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionTask::Pkg_Handler(){
	try{
		
		if(gDataCollectionConfig->GetTrackerVendor()=="PAD"){
			//: pad可穿戴中断设备的协议栈
			TCDataCollectionPadHandler sHandlerObj;
			//: 接收数据报文
			sHandlerObj.Main_Handler(m_cusSocket);
			return;
		}else if(gDataCollectionConfig->GetTrackerVendor()=="LW"){
			//: 瞭望可穿戴中断设备的协议栈
			TCDataCollectionLWHandler sHandlerObj;
			//: 接收数据报文
			sHandlerObj.Main_Handler(m_cusSocket);
			return;
		}else if(gDataCollectionConfig->GetTrackerVendor()=="JTT808"){
			//: 车载设备的协议栈
			TCDataCollectionJTT808Handler sHandlerObj;
			//: 接收数据报文
			sHandlerObj.Main_Handler(m_cusSocket);
			return;
		}		
  }catch (TCException &e){
    m_cusSocket.Close();
    throw e;
  }catch (...){
    m_cusSocket.Close();
    throw TCException("TCDataCollectionTask::Pkg_Handler() Error:"
                        "Unknown Exception.");
  }
}
//==========================================================================
// 函数 : void TCDataCollectionTask::Init
// 用途 : 初始化
// 原型 : void Init()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionTask::Init(){
	m_sPkg_Head_Flag="";
	m_sPkg_Content="";
	m_sReq_Command="";
	m_sRespCommand="";
	m_sRespPkgContent="";
	return;
}
