/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : ���涨λ�豸Э�鴦����
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
    
    //: JT/T808-2011�����׼�Ĺؼ�����
    unsigned short m_nMsgId;	//: ��Ϣ����
    unsigned short m_nMsgAttr;	//: ��Ϣ����
    int m_nMsgBodyLen;	//: ��Ϣ�峤��
    TCString m_sMsgName;	//: ��Ϣ����
    TCString m_sMsisdn;	//: �ֻ�����
		TCString m_device_id; // �豸���� �ֻ�Ϊimei
		TCString m_no_imsi;			// �ֻ�imsi
    unsigned short m_nMsgSerialNum;	//: ��Ϣ���
    TCString m_sRecvMsgBody;	//: ���յ�����Ϣ�壻
    
    //: ��Ӧ����
    unsigned short m_nRespMsgSerialNum;	//: ���;
    unsigned short m_nRespMsgId;	//: ��Ϣ����
    TCString m_sRespMsgBody;	//: ��Ӧ��Ϣ�壻
    
    
        
    TCString  m_sRecvTime;
    TCString  m_sSendTime;
    
    //: �ն��ϱ�IP��ַ��
    TCString m_sTacker_Send_IPAddress;
	
		//: ����ͷ��ǰ׺ CWT
		TCString m_sPkg_Head_Flag;
		//: ����ͷ�����ݵĳ���
		long m_nContent_Len;

		//: �������������������
		TCString m_sPkg_Content;

		//: �ָ��ַ�
		char m_cDelimter;
		
		//: ��������������������������
		TCString m_sReq_Command;
		
		//: ����һ�ν��� ������ & �ָ������ ����ֶ��б�
		TCStringList m_lsContentFieldList;
		
		//: �������ݴ�������Ӧ������
		TCString m_sRespCommand;
		
		//: �������ݴ�������Ӧ��������
		TCString m_sRespPkgContent;
		
    static TCCriticalSection m_csLogLock;
    
    
private:
	//: ���泧�ҵ����ݴ���ʽ����������, ��������������
  void RecvRequest(TCCustomUniSocket  &cusSocket);
  
    void DealRequest(TCCustomUniSocket  &cusSocket);
    void SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespCommand, TCString sRespContent);

    void Init();

		bool RecordTrackerLocInfo(TCString phone_no, str_Space_Pos_Unit sPosNode, str_Lac_Cell_Node sLBSNode, int nRscp, int nRxLev,
			double nspeed=0.0, double high = 0.0, int direcion = 0);
		
private:
		//: �ն�ע�ᴦ��
		void DoCommand_ClientRegister(TCCustomUniSocket  &cusSocket);
   
		//: �ն˼�Ȩ
		void DoCommand_ClientAuth(TCCustomUniSocket  &cusSocket);
    		
		//: λ����Ϣ�㱨
    void DoCommand_LocInfo(TCCustomUniSocket  &cusSocket);
 		
 		//: ��������͸��
 		void DoCommand_DataTrans(TCCustomUniSocket  &cusSocket);
 		 		
 		//: ͨ��Ӧ��
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

