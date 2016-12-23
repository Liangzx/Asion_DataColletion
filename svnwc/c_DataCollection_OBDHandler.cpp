/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : OBD��λ�豸Э�鴦����
File Name   : c_DataCollection_OBDHandler.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#pragma hdrstop

#include <algorithm>
#include <ctime>
#include "c_DataCollection_OBDHandler.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_task.h"

#pragma package(smart_init)

extern TCDataCollectionHandler gsDataCollectionHandler;

extern TCDataCollectionConfig *gDataCollectionConfig;


// ���ݿ�����
extern otl_connect m_dbConnect;

// ���ݿ�����
extern otl_connect m_dbConnect;

extern TCDataCollectionLog  *glgMls;

extern map<str_Lac_Cell_Node, str_Space_Pos_Unit> g_msCellInfo_PosUnit;

bool is_big_endian()
{
	unsigned short test = 0x1234;
	if (*((char *)&test) == 0x12) {
		return true;
	}
	else {
		return false;
	}
}

TCString TCDataCollectionOBDHandler::unpack_pkg(TCString sPkg_Content)
{
	// TODO:�Ա��Ľ��н��ת�崦��
	/*
	ת���������:0XE7->0xE6+0X02
				0XE6->0XE6+0X01
	*/
	unsigned char xe6 = 0xE6;
	unsigned char xe7 = 0xE7;
	unsigned char x01 = 0x01;
	unsigned char x02 = 0x02;
	TCString sMessage = "";
	int nMegLen = Length(sPkg_Content);
	for (int i = 1; i <= nMegLen; i++) {
		if ((unsigned char)sPkg_Content[i] == xe6) {
			if ((i + 1) <= nMegLen) {
				if ((unsigned char)sPkg_Content[i + 1] == x01) {
					sMessage += xe6;
					i++;
				}
				else if ((unsigned char)sPkg_Content[i + 1] == x02) {
					sMessage += xe7;
					i++;
				}
				else {
					;
				}
			}
		}
		else {
			sMessage += sPkg_Content[i];
		}
	}
	return sMessage;
}

TCString TCDataCollectionOBDHandler::pack_pkg(TCString sPkg_Content)
{
	// TODO:�Ա��ĵ����ݽ��д��ת�崦��������Ϣ��ʶ

	TCString re_pkg;
	re_pkg = "";
	unsigned char xe6 = 0xE6;
	unsigned char xe7 = 0xE7;


	TCString pack_xe6 = TCString(0xE6) + TCString(0x01);
	TCString pack_xe7 = TCString(0xE6) + TCString(0x02);

	for (int i = 1; i <= sPkg_Content.GetLength(); ++i) {
		if ((unsigned char)sPkg_Content[i] == 0xE6) {
			re_pkg += pack_xe6;
		}else if ((unsigned char)sPkg_Content[i] == xe7) {
			re_pkg += pack_xe7;
		} else {
			re_pkg += sPkg_Content[i];
		}
	}
	return re_pkg;
}

//==========================================================================
// ���� : TCDataCollectionOBDHandler::TCDataCollectionOBDHandler
// ��; : ���캯��
// ԭ�� : TCDataCollectionOBDHandler()
// ���� :
// ���� :
// ˵�� :
//==========================================================================
TCDataCollectionOBDHandler::TCDataCollectionOBDHandler() {
	redis_context = NULL;
	m_device_id = "";
}

//==========================================================================
// ���� : TCDataCollectionOBDHandler::~TCDataCollectionOBDHandler()
// ��; : ��������
// ԭ�� : ~TCDataCollectionOBDHandler()
// ���� :
// ���� :
// ˵�� :
//==========================================================================
TCDataCollectionOBDHandler::~TCDataCollectionOBDHandler()
{
	if (redis_context != NULL)
	{
		redisFree(redis_context);
	}
}

//==========================================================================
// ���� : TCDataCollectionOBDHandler::Main_Handler
// ��; : ��Ҫʵ�־����ҵ�񽻻��߼�
// ԭ�� : void Main_Handler()
// ���� :
// ���� :
// ˵�� : ��Ҫʵ�־����ҵ�񽻻��߼�
//==========================================================================
bool TCDataCollectionOBDHandler::Main_Handler(TCCustomUniSocket  &cusSocket) {
	try {
		vender = gDataCollectionConfig->GetTrackerVendor();
		// connect redis server
		const char * host = gDataCollectionConfig->GetRedisServerAddress();
		int port = gDataCollectionConfig->GetRedisServerPort();
		struct timeval time_out = { 1, 500000 };
		redis_context = redisConnectWithTimeout(host, port, time_out);
		if (redis_context == NULL || redis_context->err)
		{
			if (redis_context)
			{
				printf("connection error: %s\n", redis_context->errstr);
				redisFree(redis_context);
			}
			else
			{
				printf("connection error: can't allocate redis redis_context.\n");
			}
			exit(-1);
		}
		// redis ������֤
		redisReply * reply_auth = (redisReply *)redisCommand(redis_context, "AUTH %s", (char *)gDataCollectionConfig->GetRedisAuth());
		if (reply_auth->type == REDIS_REPLY_ERROR) {
			printf("auth error: %s\n.", reply_auth->str);
			freeReplyObject(reply_auth);
			redisFree(redis_context);
			reply_auth = NULL;
			redis_context = NULL;
			exit(-1);
		}
		freeReplyObject(reply_auth);
		reply_auth = NULL;

		std::cout << "redis connect success..." << std::endl;
		// Զ�̷��͵ĵ�ַ
		m_sTacker_Send_IPAddress = cusSocket.GetRemoteAddress();
		m_nTimeOut = 60;
		printf("���յ� Tracker ��������IP=[%s], Port=[%d], Timeout=[%d]\n", (char*)cusSocket.GetRemoteAddress(), cusSocket.GetRemotePort(), m_nTimeOut);

		// �����ӷ�ʽ
		while (1) {
			m_sRecvTime = TCTime::Now();
			Init();
			TCString sLogMsg = "Time:Now=[" + TCTime::Now() + "],Recv Port=[" + IntToStr(cusSocket.GetRemotePort()) + "] Data";
			glgMls->AddDataCollectionRunLog(sLogMsg);
			RecvRequest(cusSocket);
			DealRequest(cusSocket);
		}
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return true;
	}
	catch (TCException &e) {
		TCString sLogMsg = TCString("At TCDataCollectionOBDHandler::Main_Handler ��������ʱ����:") + e.GetExceptionMessage();
		glgMls->AddDataCollectionRunLog(sLogMsg);
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return false;
	}
	catch (...) {
		glgMls->AddDataCollectionRunLog("At TCDataCollectionOBDHandler::Main_Handler Unknow, �ر�Socket����");
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return false;
	}
}

//==========================================================================
// ���� : TCDataCollectionOBDHandler::DealRequest
// ��; : ��������
// ԭ�� : void DealRequest()
// ���� : ��
// ���� : ��
// ˵�� :
//==========================================================================
void TCDataCollectionOBDHandler::DealRequest(TCCustomUniSocket  &cusSocket) {
	try {
		// ���������ֽ��д���
#ifdef __TEST__
		printf("============receive Tracker request==============\n");
		printf("m_nMsgId:=%d, m_nMsgAttr=[%d], m_nMsgBodyLen=[%d], m_sMsisdn=[%s], Time=[%s]\n", \
			m_nMsgId, m_nMsgAttr, m_nMsgBodyLen, (char*)m_sMsisdn, (char*)TCTime::Now());
#endif

		TCString sLog = "";
		//======ִ�з������==========================		
		switch (m_nMsgId) {
		case 0x0102:
			// �ն˵�½	
			sLog = "�ն˵�½";
			glgMls->AddDataCollectionRunLog(sLog);
			DoCommand_Login_Resp(cusSocket);
			break;
		case 0x0002:
			// �ն������ϱ�	
			sLog = "�ն������ϱ�";
			glgMls->AddDataCollectionRunLog(sLog);
			// ��Ϣ����
			msg_arr_[0] = 0x00;
			msg_arr_[1] = 0x02;
			DoCommand_GeneralResp(cusSocket, msg_arr_);
			break;
		case 0x0202:
			// �ն��ϴ�GPS��λ����
			sLog = "�ն��ϴ�GPS��λ����";
			glgMls->AddDataCollectionRunLog(sLog);
			DoCommand_ParseGpsInfo(cusSocket);
			// ��Ϣ����
			msg_arr_[0] = 0x00;
			msg_arr_[1] = 0x05;
			DoCommand_GeneralResp(cusSocket, msg_arr_);
			break;
		case 0x0207:
			// �ն��ϴ���վ����
			sLog = "�ն��ϴ���վ����--����Ҫ��������Ӧ";
			glgMls->AddDataCollectionRunLog(sLog);
			DoCommand_ParseLbsInfo(cusSocket);
			// ��Ϣ����
			msg_arr_[0] = 0x00;
			msg_arr_[1] = 0x05;
			DoCommand_GeneralResp(cusSocket, msg_arr_);
			break;
		case 0x0218:
			// �ն��ϴ���վ����(4G)
			sLog = "�ն��ϴ���վ����(4G)";
			glgMls->AddDataCollectionRunLog(sLog);
			DoCommand_ParseLbsInfo_4G(cusSocket);
			msg_arr_[0] = 0x00;
			msg_arr_[1] = 0x05;
			DoCommand_GeneralResp(cusSocket, msg_arr_);
			break;
		default:
			// ������Ϣͨ��Ӧ��
			sLog = "OBD������Ϣ";
			//��Ϣ����
			msg_arr_[0] = 0x00;
			msg_arr_[1] = 0x05;
			glgMls->AddDataCollectionRunLog(sLog);
			DoCommand_GeneralResp(cusSocket, msg_arr_);
		}
	}
	catch (TCException &e) {
		try {
			TCString sLogMsg = TCString("ϵͳ����,At DealRequest: ") + e.GetExceptionMessage();
			glgMls->AddDataCollectionRunLog(sLogMsg);
		}
		catch (TCException &e1) {
			TCString sLogMsg = TCString("ϵͳ����: when Catch a Error and SendRespPkg: ") + e1.GetExceptionMessage();
			glgMls->AddDataCollectionRunLog(sLogMsg);
		}
		catch (...) {
			glgMls->AddDataCollectionRunLog("ϵͳ����δ֪�쳣");
		}
	}
}

//==========================================================================
// ���� : TCDataCollectionOBDHandler::RecvRequest(TCCustomUniSocket  &cusSocket)
// ��; : JT/T808-2011�����׼�����ݴ���ʽ����������, ��������������
// ԭ�� : void RecvRequest()
// ���� : ��
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionOBDHandler::RecvRequest(TCCustomUniSocket  &cusSocket) {
	try {
		const int nMaxPkg_Length = 1024 * 4;
		char sbuff[nMaxPkg_Length];
		memset(sbuff, 0, sizeof(sbuff));

		m_sReq_Command = "";
		m_sPkg_Content = "";

		int nTimeOut = 600 * 1000;

		// ��ʼ���� select �������ж�
		if (!cusSocket.WaitForData(nTimeOut)) {
			// ��ʱ120���ˣ�����ֱ�ӹر����ӣ�
			TCString sLog = "OBD Tracker[" + m_sTacker_Send_IPAddress + "]�Ѿ���ʱ120��δ�������ݣ��Ͽ�����";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}

		int nGetPkgLen = 0;
		nGetPkgLen = cusSocket.ReceiveBuf(sbuff, nMaxPkg_Length);
		// ԭʼ����
		show_pkg(sbuff, nGetPkgLen);
		if (nGetPkgLen == 0) {
			// ��ʱ120���ˣ�����ֱ�ӹر����ӣ�
			TCString sLog = "OBD Tracker[" + m_sTacker_Send_IPAddress + "]�Ѿ��Ͽ�����";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}
		TCString sPkg_Content(sbuff, nGetPkgLen);

		// ��һ���ֽ�0xBB�����һ���ֽڶ���0xEE
		// TODO:
		if (Length(sPkg_Content)<2 || (unsigned char)sPkg_Content[1] != 0xE7 || (unsigned char)sPkg_Content[Length(sPkg_Content)] != 0xE7) {
			// �쳣����������
			TCString sLogMsg;
			sLogMsg = "";
			sLogMsg += TCString("ERROR: �����ĳ����쳣=[") + IntToStr(Length(sPkg_Content)) + "]";
			glgMls->AddDataCollectionRunLog(sLogMsg);
			printf("Error Packet Format: %s\n", (char*)sLogMsg);
			cusSocket.Close();
			throw TCException(sLogMsg);
		}		

		//ԭʼ����		
		sPkg_Content = Mid(sPkg_Content, 2, Length(sPkg_Content) - 1);		

		//ת�屨��--���
		m_sPkg_Content = unpack_pkg(sPkg_Content);
		// ��Ϣ����
		m_nMsgAttr = AscStrToInt(Mid(m_sPkg_Content, 3, 2));
		std::cout << "��Ϣ���ԣ�" << m_nMsgAttr << std::endl;
		//��ϢID
		m_nMsgId = AscStrToInt(Mid(m_sPkg_Content, 1, 2));
		// ��Ϣ�峤��
		m_nMsgBodyLen = (~(63 << 9))&(m_nMsgAttr);
		std::cout << "��Ϣ�峤�ȣ�" << m_nMsgBodyLen << std::endl;
		//��Ϣ��ˮ��
		m_nMsgSerialNum = AscStrToInt(Mid(m_sPkg_Content, 12, 2));
		// �豸id
		m_device_id = Mid(m_sPkg_Content, 5, 7);

		//// У��ͺ���֤
		//if ((int)len_chk != Length(m_sPkg_Content)) {
		//	LOG_WRITE("У��ͺ���֤ʧ��");
		//}
	}
	catch (TCException &e) {
		cusSocket.Close();
		throw e;
	}
	catch (...) {
		cusSocket.Close();
		throw TCException("TCDataCollectionOBDHandler::RecvRequest() Error:"
			"Unknown Exception.");
	}
}

char TCDataCollectionOBDHandler::chk_x_or(TCString chk_buf)
{
	std::string buf = (char *)chk_buf;
	char res = buf[0];
	for (size_t i = 1; i < chk_buf.GetLength(); i++)
	{
		res ^= (char)buf[i];
	}
	return res;
}

//==========================================================================
// ���� : void TCDataCollectionOBDHandler::DoCommand_ClientAuth
// ��; : �ն˼�Ȩ
// ԭ�� : void DoCommand_ClientAuth()
// ���� : ��
// ���� : ��
// ˵�� :  
//==========================================================================
void TCDataCollectionOBDHandler::DoCommand_Login_Resp(TCCustomUniSocket  &cusSocket) {

	TCString msg_body = Mid(m_sPkg_Content, 14, m_nMsgBodyLen);
	
	TCString pkg_imei = Mid(msg_body, 52, 15);
	TCString pkg_imsi = Mid(msg_body, 67, 15);

	m_no_imsi = pkg_imsi;
	m_no_imei = pkg_imei;
	char ch_buf[4] = { 0x81, 0x02, 0x00, 0x02 };
	//
	TCString send_buf(ch_buf, 4);
	// TODO:6 bit gmt time
	//
	time_t tt = time(NULL);
	tm *t = gmtime(&tt);
	// ƽ̨��ǰʱ��
	char ch_t[6];
	ch_t[5] = (char)t->tm_sec;
	ch_t[4] = (char)t->tm_min;
	ch_t[3] = (char)t->tm_hour;
	ch_t[2] = (char)t->tm_mday;
	ch_t[1] = (char)t->tm_mon;
	ch_t[0] = (char)t->tm_year;
	// ����ID
	send_buf += m_device_id;
	send_buf += ch_t;
	TCString car_type = Mid(msg_body, 102, 2);
	send_buf += car_type;
	// ����
	short car_volume = 2000;
	TCString car_vol((char *)&car_volume, 2);
	send_buf += car_vol;
	// �Ƿ�����
	char if_update = 0x50;
	send_buf += if_update;
	// У��
	char chk = chk_x_or(send_buf);
	send_buf += chk;
	send_buf = pack_pkg(send_buf);
	send_buf = TCString(0xE7) + send_buf + TCString(0xE7);

	cusSocket.SendBuf(send_buf, send_buf.GetLength());
}

void TCDataCollectionOBDHandler::DoCommand_GeneralResp(TCCustomUniSocket & cusSocket,
	char msg_arr[2])
{
	TCString msgs_arr_s(msg_arr, 2);
	TCString msg_num =TCString((char *)&m_nMsgSerialNum, 2);

	// ��С���ж�
	if (is_big_endian()) {
		std::string msg_num_s = (char *)msg_num;
		std::reverse(msg_num_s.begin(), msg_num_s.end());
		msg_num = (char *)msg_num_s.c_str();
	}

	TCString send_buf = "";
	send_buf += TCString(0x80) + TCString(0x01);
	send_buf += msgs_arr_s;
	send_buf += m_device_id;
	send_buf += msg_num;
	send_buf += msg_num;

	char fun_id[2] = { 0x80, 0x01 };
	send_buf += fun_id;
	char res = 0x00;
	send_buf += res;
	char chk = chk_x_or(send_buf);
	send_buf += chk;
	send_buf = pack_pkg(send_buf);
	send_buf = TCString(0XE7)+ send_buf + TCString(0XE7);

	cusSocket.SendBuf(send_buf, send_buf.GetLength());
}

void TCDataCollectionOBDHandler::DoCommand_ParseGpsInfo(TCCustomUniSocket & cusSocket)
{
	TCString msg_body = Mid(m_sPkg_Content, 14, m_nMsgBodyLen);
	TCString gps_infos = Mid(msg_body, 4, m_nMsgBodyLen -3);
	TCString trip_mark = Mid(msg_body, 1, 2);
	gps_infos_.clear();
	unsigned short gps_pkg_num = AscStrToInt(Mid(msg_body, 3, 1));
	GpsInfo gps;
	for (size_t i = 0; i < gps_pkg_num; i++)
	{
		gps.altitude_ = AscStrToInt(Mid(gps_infos, (int)(gps_pkg_num * i + 15), 4));
		gps.bat_vol_ = AscStrToInt(Mid(gps_infos, (int)(gps_pkg_num * i + 26), 2));
		gps.direction_ = AscStrToInt(Mid(gps_infos, (int)(gps_pkg_num * i + 22), 2));
		gps.latitude_ = AscStrToInt(Mid(gps_infos, (int)(gps_pkg_num * i + 7), 4));
		gps.longitude_ = AscStrToInt(Mid(gps_infos, (int)(gps_pkg_num * i + 11), 4));
		gps.speed_ = AscStrToInt(Mid(gps_infos, (int)(gps_pkg_num * i + 28), 1));

		gps_infos_.push_back(gps);
	}
	RecordInfo record;
	record.clear();

	for (size_t i = 0; i < gps_infos_.size(); i++)
	{
		record.electricity_ = (int)gps_infos_[i].bat_vol_;
		record.elevation_ = (int)gps_infos_[i].altitude_;
		record.electricity_ = (int)gps_infos_[i].bat_vol_;
		record.latitude_ = FloatToStr(gps.latitude_ / 1000000.0, 6);
		record.longitude_ = FloatToStr(gps.longitude_ / 1000000.0, 6);
		record.speed = (int)gps.speed_;
		//
		record.time_stamp = TCTime::Now();
		record.imsi = m_no_imsi;
		record.unique_id_ = m_no_imei;
		record.vender = vender;

		LocationRecordToRedis(record);
	}

}

void TCDataCollectionOBDHandler::DoCommand_ParseLbsInfo(TCCustomUniSocket & cusSocket)
{
	TCString msg_body = Mid(m_sPkg_Content, 14, m_nMsgBodyLen);
	//
	RecordInfo record;
	record.clear();

	record.lac_ = (int)AscStrToInt(Mid(msg_body, 14, 2));
	record.cellid_ = (int)AscStrToInt(Mid(msg_body, 16, 2));
	record.rscp_ = (int)AscStrToInt(Mid(msg_body, 19, 1));
	record.rxlev_ = record.rscp_;
	//
	record.time_stamp = TCTime::Now();
	record.vender = vender;
	record.imsi = m_no_imsi;
	record.unique_id_ = m_no_imei;
	//
	LocationRecordToRedis(record);
}

void TCDataCollectionOBDHandler::DoCommand_ParseLbsInfo_4G(TCCustomUniSocket & cusSocket)
{
	TCString msg_body = Mid(m_sPkg_Content, 14, m_nMsgBodyLen);
	int sign_type = AscStrToInt(Mid(msg_body, 10, 1));
	TCString lbs_info = Mid(msg_body, 12, m_nMsgBodyLen - 11);

	int lac_tac_idx;
	int ci_idx;
	int rxlev_idx;

	if (sign_type==3) {
		// 4G
		lac_tac_idx = 8;
		ci_idx = 2;
		rxlev_idx = 11;
	} else if (sign_type==2) {
		// 3G
		lac_tac_idx = 2;
		ci_idx = 3;
		rxlev_idx = 7;
	} else if (sign_type==1) {
		// 2G
		lac_tac_idx = 2;
		ci_idx = 3;
		rxlev_idx = 7;
	} else {
		return;
	}

	TCStringList lbs_lst;
	lbs_lst.CommaText(lbs_info, ',');

	unsigned long lac = AscStrToInt(lbs_lst[lac_tac_idx]);
	unsigned long ci = AscStrToInt(lbs_lst[ci_idx]);
	unsigned long rxlev = AscStrToInt(lbs_lst[rxlev_idx]);

	RecordInfo record;
	record.clear();
	record.imsi = m_no_imsi;
	record.unique_id_ = m_no_imei;
	record.time_stamp = TCTime::Now();
	record.rscp_ = rxlev;
	record.rxlev_ = rxlev;
	record.vender = vender;
	
	//
	LocationRecordToRedis(record);
}

//==========================================================================
// ���� : void TCDataCollectionOBDHandler::Init
// ��; : ��ʼ��
// ԭ�� : void Init()
// ���� : ��
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionOBDHandler::Init() {
	m_sPkg_Content = "";
	m_sReq_Command = "";
	m_sRespCommand = "";
	m_sRespPkgContent = "";
	return;
}

bool TCDataCollectionOBDHandler::LocationRecordToRedis(RecordInfo record_info_par)
{
	__ENTERFUNCTION;
	RecordInfo record_info = record_info_par;
	
	redisReply * ry_obd = NULL;

	// ����imsiȥLOC_POSTERM_INFO��ȡ��������
	ry_obd = (redisReply *)redisCommand(redis_context, "HGET LOC_POSTERM_INFO %s", (char *)record_info.imsi);
	if (ry_obd->type == REDIS_REPLY_NIL || ry_obd->type == REDIS_REPLY_ERROR) {
		LOG_WRITE("LOC_POSTERM_INFO���޶�Ӧ��imsi:[%s]", (char *)record_info.imsi);
		freeReplyObject(ry_obd);
		ry_obd = NULL;
		return false;
	}

	TCString term_type = "";
	if (ry_obd->type == REDIS_REPLY_STRING) {
		TCString str = ry_obd->str;
		TCStringList str_list;
		str_list.CommaText(str, ',');
		if (str_list.GetCount() < 1) {
			freeReplyObject(ry_obd);
			return false;
		}
		else {
			term_type = str_list[1];
		}
	}
	freeReplyObject(ry_obd);
	ry_obd = NULL;

	term_type = AllTrim(term_type);
	record_info.vender = term_type;
	// 
	std::string str_csv = record_info.putcsv();
	ry_obd = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromDataCollection %s", (char *)str_csv.c_str());
	if (ry_obd->type == REDIS_REPLY_ERROR)
	{
		std::cout << "record_info--csv: " << str_csv << "failed" << std::endl;
	}
	freeReplyObject(ry_obd);
	ry_obd = NULL;
	// 
	// LBS �������
	ry_obd = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromLBSCollection %s", (char *)str_csv.c_str());
	if (ry_obd->type == REDIS_REPLY_ERROR)
	{
		perror("error:LPUSH WuDongDe:PositionData:FromLBSCollection");
	}
	freeReplyObject(ry_obd);
	ry_obd = NULL;

	// ��ԭʼ��¼���ʹ��
	ry_obd = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:SrcRecordForOracle %s", (char *)str_csv.c_str());
	freeReplyObject(ry_obd);
	ry_obd = NULL;

	// �ն�����һ����¼������
	std::string str_latest_data;
	str_latest_data += (char *)record_info.time_stamp;
	str_latest_data += ",";
	str_latest_data += "";
	str_latest_data += ",";
	str_latest_data += IntToStr(record_info.electricity_);
	str_latest_data += ",";
	str_latest_data += "";
	str_latest_data += ",";
	str_latest_data += "";
	str_latest_data += ",";
	str_latest_data += FloatToStr(record_info.speed);
	str_latest_data += ",";
	str_latest_data += term_type;

	ry_obd = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:PostermLastDataHash %s %s",
		(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
	freeReplyObject(ry_obd);
	ry_obd = NULL;
	//
	// ����ָ�Ʋɼ�����--imsi+timestamp[20161026142021--yyyymmddhhmmss],device_type
	str_latest_data = (char *)record_info.time_stamp;
	str_latest_data += ",";
	str_latest_data += term_type;

	// �ն�״̬ȷ��
	ry_obd = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:TermStatusCheckHash %s %s",
		(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
	freeReplyObject(ry_obd);
	ry_obd = NULL;
	return true;

	__LEAVEFUNCTION
	return false;
}




