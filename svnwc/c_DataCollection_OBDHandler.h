/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : OBD��λ�豸Э�鴦����
File Name   : c_DataCollection_OBDHandler.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#ifndef C_DataCollection_OBDHANDLER_H
#define C_DataCollection_OBDHANDLER_H
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

typedef struct GpsInfo {
	unsigned long speed_; //�ٶ�
	unsigned long direction_; // ����
	unsigned long bat_vol_; // ��ѹ
	unsigned long  latitude_;// γ��
	unsigned long  longitude_;// ����
	unsigned long  altitude_;//����

}GpsInfo;

class TCDataCollectionOBDHandler
{
private:
	long  m_nTimeOut;
	long  m_nProcessTime;
	// д��redis�Ľṹ
	RecordInfo record_info_;

	// JT/T808-2011�����׼�Ĺؼ�����
	unsigned short m_nMsgId;	// ��Ϣ����
	unsigned short m_nMsgAttr;	// ��Ϣ����
	int m_nMsgBodyLen;			// ��Ϣ�峤��
	TCString m_sMsgName;		// ��Ϣ����
	TCString m_sMsisdn;			// �ֻ�����
	// obd ��imei
	TCString m_device_id;		// �ն�id
	TCString m_no_imsi;			// �ֻ�imsi
	TCString m_no_imei;			// �ֻ�imsi
	unsigned short m_nMsgSerialNum;	// ��Ϣ���
	TCString m_sRecvMsgBody;	// ���յ�����Ϣ��

	unsigned short m_nRespMsgSerialNum;	// ���;
	unsigned short m_nRespMsgId;	// ��Ϣ����
	TCString m_sRespMsgBody;	// ��Ӧ��Ϣ�壻


	TCString  m_sRecvTime;
	TCString  m_sSendTime;

	// �ն��ϱ�IP��ַ��
	TCString m_sTacker_Send_IPAddress;

	// ����ͷ��ǰ׺ CWT
	TCString m_sPkg_Head_Flag;
	// ����ͷ�����ݵĳ���
	long m_nContent_Len;

	// �������������������--OBD������ͷβ--Ϊ������
	TCString m_sPkg_Content;

	// �ָ��ַ�
	char m_cDelimter;

	// ��������������������������
	TCString m_sReq_Command;

	// ����һ�ν��� ������ & �ָ������ ����ֶ��б�
	TCStringList m_lsContentFieldList;

	// �������ݴ�������Ӧ������
	TCString m_sRespCommand;

	// �������ݴ�������Ӧ��������
	TCString m_sRespPkgContent;

	static TCCriticalSection m_csLogLock;


private:
	// OBD�����ݴ���ʽ����������, ��������������
	void RecvRequest(TCCustomUniSocket  &cusSocket);
	void show_pkg(const char * sbuff, long len) {
		TCString ms_sRow_Pkt = "";
		char sBuff[32] = { 0 };
		for (int nSeq = 0; nSeq<len; nSeq++) {
			memset(sBuff, 0, sizeof(sBuff));
			sprintf(sBuff, "%02x:", sbuff[nSeq]);
			ms_sRow_Pkt += Right(TCString(sBuff), 3);
		}
		printf("OBD:received original packet: %s\n", (char *)ms_sRow_Pkt);
	}

	void DealRequest(TCCustomUniSocket  &cusSocket);

	void Init();

	bool LocationRecordToRedis(RecordInfo record_info);

private:
	char chk_x_or(TCString chk_buf);
	// �ն˵�½
	void DoCommand_Login_Resp(TCCustomUniSocket  &cusSocket);

	// ƽ̨ͨ��Ӧ��
	void DoCommand_GeneralResp(TCCustomUniSocket  &cusSocket, char msg_arr[2]);

	// �ն��ϴ�GPS��λ����
	void DoCommand_ParseGpsInfo(TCCustomUniSocket  &cusSocket);

	// �ն��ϴ���վ����
	void DoCommand_ParseLbsInfo(TCCustomUniSocket  &cusSocket);
	void DoCommand_ParseLbsInfo_4G(TCCustomUniSocket  &cusSocket);
	// ����ת��--�ⱨ��
	TCString unpack_pkg(TCString sPkg_Content);
	// ����ת��--�������
	TCString pack_pkg(TCString sPkg_Content);


private:
	std::vector<GpsInfo> gps_infos_;
	redisContext * redis_context;
	TCString vender;
	char msg_arr_[2];
public:
	TCDataCollectionOBDHandler();
	~TCDataCollectionOBDHandler();
	bool Main_Handler(TCCustomUniSocket  &cusSocket);
};

#endif


