/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 兆益定位设备协议处理类
File Name   : c_DataCollection_JTT808Handler.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#ifndef C_DataCollection_JTT808HANDLER_H
#define C_DataCollection_JTT808HANDLER_H
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

typedef struct ImsiMore {
	TCString imsi_;
	TCString imei_;
}ImsiMore;

class TCDataCollectionJTT808Handler
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
    
    //: JT/T808-2011部颁标准的关键数据
    unsigned short m_nMsgId;	//: 消息类型
    unsigned short m_nMsgAttr;	//: 消息属性
    int m_nMsgBodyLen;	//: 消息体长度
    TCString m_sMsgName;	//: 消息名称
    TCString m_sMsisdn;	//: 手机号码
		TCString m_device_id; // 设备串号 手机为imei
		TCString m_no_imsi;			// 手机imsi
    unsigned short m_nMsgSerialNum;	//: 消息序号
    TCString m_sRecvMsgBody;	//: 接收到的消息体；
    
    //: 响应数据
    unsigned short m_nRespMsgSerialNum;	//: 序号;
    unsigned short m_nRespMsgId;	//: 消息类型
    TCString m_sRespMsgBody;	//: 响应消息体；
    
    
        
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
    
    
private:
	//: 兆益厂家的数据处理方式，接收数据, 按照无阻塞接收
  void RecvRequest(TCCustomUniSocket  &cusSocket);
  
    void DealRequest(TCCustomUniSocket  &cusSocket);
    void SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespCommand, TCString sRespContent);

    void Init();

		bool RecordTrackerLocInfo(TCString phone_no, str_Space_Pos_Unit sPosNode, str_Lac_Cell_Node sLBSNode, int nRscp, int nRxLev,
			double nspeed=0.0, double high = 0.0, int direcion = 0);
		
private:
		//: 终端注册处理
		void DoCommand_ClientRegister(TCCustomUniSocket  &cusSocket);
   
		//: 终端鉴权
		void DoCommand_ClientAuth(TCCustomUniSocket  &cusSocket);
    		
		//: 位置信息汇报
    void DoCommand_LocInfo(TCCustomUniSocket  &cusSocket);
 		
 		//: 数据上行透传
 		void DoCommand_DataTrans(TCCustomUniSocket  &cusSocket);
 		 		
 		//: 通用应答
 		void ServerGeneralAck(TCCustomUniSocket  &cusSocket);
private:
	redisContext * redis_context;
	TCString vender;
public:
    TCDataCollectionJTT808Handler();
    ~TCDataCollectionJTT808Handler();
   	bool Main_Handler(TCCustomUniSocket  &cusSocket);
};

#endif

