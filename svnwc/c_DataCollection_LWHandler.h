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

	TCString m_sDev_ID;
	TCString m_sImei;
	TCString m_sImsi;
	TCString m_sRow_Pkt;
	unsigned short m_nMsgId;
	//
	vector<TCString> m_vsPkgList;
	TCString unrecv_;
	//: ��¼����󣬿ͻ���δ��Ӧ�Ĳ�������Լ��������ݣ�
	map<TCString, TCString> m_msOperID_PkgContent;

	TCString  m_sRecvTime;
	TCString  m_sSendTime;

	//: �ն��ϱ�IP��ַ��
	TCString m_sTacker_Send_IPAddress;

	//: �������������������
	TCString m_sPkg_Content;

	//: ��������������������������
	TCString m_sReq_Command;

	//: �������ݴ�������Ӧ������
	TCString m_sRespCommand;

	//: �������ݴ�������Ӧ��������
	TCString m_sRespPkgContent;

	static TCCriticalSection m_csLogLock;


private:
	//: �t�����ҵ����ݴ���ʽ����������, ��������������
	void RecvRequest(TCCustomUniSocket  &cusSocket);

	void DealRequest(TCCustomUniSocket  &cusSocket);
	void SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespContent);

	void Init();
	bool LocationRecordToRedis(RecordInfo record_info);
	double gps_lac_lon_deal(TCString lac_lon);
	void parse_blue_list(TCString const & blue_str, std::vector<std::string> & blue_list);
	unsigned char get_check_code();
	void pkg_resp(TCCustomUniSocket  &cusSocket);

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

