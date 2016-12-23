/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : pad设备协议处理类
File Name   : c_DataCollection_PadHandler.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#ifndef C_DataCollection_PADHANDLER_H
#define C_DataCollection_PADHANDLER_H
//---------------------------------------------------------------------------
#include "cmpublic.h"
#include "cmpublic_net.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_Public.h"
#include "c_DataCollection_Handle.h"
#include "c_DataCollection_log.h"
#include "hiredis.h"

#include <map>
#include <vector>

using namespace std;

#define PKG_LENGTH 7

class TCDataCollectionPadHandler
{
private:
    long  m_nTimeOut;
    long  m_nProcessTime;
    
    long m_nCmd_Index;
    
    bool nLogined_Status;
    
    TCString m_sCurAlarm_NotifyTimer;
    
    TCString m_sDev_ID;
    TCString m_sCust_ID;
    TCString m_sImei;
    TCString m_sImsi;

	vector<TCString> m_vsPkgList;

		//: 记录处理后，客户端未响应的操作序号以及报文内容；
		map<TCString, TCString> m_msOperID_PkgContent;
    
    TCString  m_sRecvTime;
    TCString  m_sSendTime;
    
    //: 终端上报IP地址；
    TCString m_sTacker_Send_IPAddress;
	
		//: 报文头的前缀 CWT
		TCString m_sPkg_Head_Flag;
		//: 报文头的内容的长度
		long m_nContent_Len;

		//: 报文内容域的整个内容
		TCString m_sPkg_Content;

		//: 分割字符
		char m_cDelimter;
		
		//: 报文内容域解析后的请求命令字
		TCString m_sReq_Command;
		
		//: 报文一次解析 （按照 & 分割解析） 后的字段列表
		TCStringList m_lsContentFieldList;
		
		//: 报文内容处理后的响应命令字
		TCString m_sRespCommand;
		
		//: 报文内容处理后的响应报文内容
		TCString m_sRespPkgContent;
		
    static TCCriticalSection m_csLogLock;
		int field_counts;

private:
	//: pad的数据处理方式，接收数据, 按照无阻塞接收
  void RecvRequest(TCCustomUniSocket  &cusSocket);
  
    void DealRequest(TCCustomUniSocket  &cusSocket);
    void SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespCommand, TCString sRespContent);

    void Init();

		////: 将穿戴式设备的测量信息登记到数据库中
		bool RecordTrackerLocInfo(TCString sImei, str_Space_Pos_Unit sPosNode, str_Lac_Cell_Node sLBSNode, int nRscp, int nRxLev, int nStatus, double nhigh, int power_percent);

		//: 将BLE的测量信息登记到数据库中
		bool RecordPadLocInfo(TCString sImei, str_Space_Pos_Unit sPosNode, str_Lac_Cell_Node sLBSNode, int nRxLev,\
			double dBLE_X, double dBLE_Y, double dBLE_Z, TCString sBLE_MAC,TCString imsi, TCString phone_no, double elevation);
		
private:
    
    //: 平板上报位置信息
    void DoCommand_PadLocInfo(TCCustomUniSocket  &cusSocket);
		//: 上报设备状态信息
		void DoCommand_DeviceStatus(TCCustomUniSocket  &cusSocket);
public:
		redisContext * redis_context;
		TCString vender;
public:
    TCDataCollectionPadHandler();
    ~TCDataCollectionPadHandler();
   	bool Main_Handler(TCCustomUniSocket  &cusSocket);
};

#endif

