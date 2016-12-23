/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : �t���豸Э�鴦����
File Name   : c_DataCollection_LWHandler.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#ifndef C_DataCollection_LWHandler_H
#define C_DataCollection_LWHandler_H
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

class TCDataCollectionLWHandler
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
	TCString m_sBit_Pkt;
	TCString m_sRow_Pkt;

	//: ��¼����󣬿ͻ���δ��Ӧ�Ĳ�������Լ��������ݣ�
	map<TCString, TCString> m_msOperID_PkgContent;

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
	//: �t�����ҵ����ݴ���ʽ����������, ��������������
	void RecvRequest(TCCustomUniSocket  &cusSocket);

	void DealRequest(TCCustomUniSocket  &cusSocket);
	void SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespCommand, TCString sRespContent);

	void Init();

	bool RecordLWTrackerLocInfo(TCString sImei, str_Space_Pos_Unit sPosNode, str_Lac_Cell_Node sLBSNode, int nRscp, int nRxLev, int nSOS_Status, int power_percent, TCString imsi, TCString phone_no, double elevation);

private:
	redisContext * redis_context;

	//: �ɴ���ʽ�ն��ϱ�GPS��LBSλ����Ϣ
	void DoCommand_GPSLBSLocInfo(TCCustomUniSocket  &cusSocket);
public:
	TCString vender;
	TCDataCollectionLWHandler();
	~TCDataCollectionLWHandler();
	bool Main_Handler(TCCustomUniSocket  &cusSocket);
};

#endif

