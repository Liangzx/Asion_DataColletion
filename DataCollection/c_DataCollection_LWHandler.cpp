/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : �t���豸Э�鴦����
File Name   : c_DataCollection_LWHandler.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#pragma hdrstop

#include "c_DataCollection_LWHandler.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_task.h"
#include "hiredis.h"

#pragma package(smart_init)

extern TCDataCollectionHandler gsDataCollectionHandler;

extern TCDataCollectionConfig *gDataCollectionConfig;



// ���ݿ�����
extern otl_connect m_dbConnect;


extern TCDataCollectionLog  *glgMls;

extern map<str_Lac_Cell_Node, str_Space_Pos_Unit> g_msCellInfo_PosUnit;

std::string hex_to_string(const char * buf, int len) {
	char ch[2] = { 0 };
	std::string str;
	std::string tmp;
	for (size_t i = 0; i < len; ++i) {
		memset(ch, 0, sizeof(ch));
		sprintf(ch, "%02X", buf[i]);
		tmp = ch;
		str += tmp.substr(tmp.size() - 2, 2);
	}

	return str;
}

std::string hex_char(const char * buf, int len) {
	char ch[2] = { 0 };
	std::string str;
	std::string tmp;
	for (size_t i = 0; i < len; ++i) {
		if ((buf[i] > 0x09) && !isalnum(buf[i])) {
			std::cout << "[wrong char]" << std::endl;
			return std::string(15, '0');
		}
		memset(ch, 0, sizeof(ch));
		if (buf[i] <= 0x09) {
			sprintf(ch, "%x", buf[i]);
		}
		else
		{
			sprintf(ch, "%c", buf[i]);
		}
		str += toupper(ch[0]);
	}

	return str;
}

//==========================================================================
// ���� : TCDataCollectionLWHandler::TCDataCollectionLWHandler
// ��; : ���캯��
// ԭ�� : TCDataCollectionLWHandler()
// ���� :
// ���� :
// ˵�� :
//==========================================================================
TCDataCollectionLWHandler::TCDataCollectionLWHandler() {
	redis_context = NULL;
}

//==========================================================================
// ���� : TCDataCollectionLWHandler::~TCDataCollectionLWHandler()
// ��; : ��������
// ԭ�� : ~TCDataCollectionLWHandler()
// ���� :
// ���� :
// ˵�� :
//==========================================================================
TCDataCollectionLWHandler::~TCDataCollectionLWHandler()
{
	if (redis_context != NULL)
	{
		redisFree(redis_context);
	}
}

//==========================================================================
// ���� : TCDataCollectionLWHandler::Main_Handler
// ��; : ��Ҫʵ�־����ҵ�񽻻��߼�
// ԭ�� : void Main_Handler()
// ���� :
// ���� :
// ˵�� : ��Ҫʵ�־����ҵ�񽻻��߼�
//==========================================================================
bool TCDataCollectionLWHandler::Main_Handler(TCCustomUniSocket  &cusSocket) {
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
		redisReply * reply_auth = (redisReply *)redisCommand(redis_context, "AUTH %s",
			(char *)gDataCollectionConfig->GetRedisAuth());
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

		printf("���յ� Tracker ��������IP=[%s], Port=[%d], Timeout=[%d]\n",
			(char*)cusSocket.GetRemoteAddress(), cusSocket.GetRemotePort(), m_nTimeOut);

		// �����ӷ�ʽ
		bool sos_active = false;
		TCString now_b = "";
		while (1) {
			m_sRecvTime = TCTime::Now();
			Init();
			TCString sLogMsg = "Time:Now=[" + TCTime::Now() + "],Recv Port=[" + IntToStr(cusSocket.GetRemotePort()) + "] Data";
			glgMls->AddDataCollectionRunLog(sLogMsg);

			RecvRequest(cusSocket);
			pkg_resp(cusSocket);
			DealRequest(cusSocket);
			// TODO:����Ƿ����·��ľ�����Ϣ
			char * str_reply = NULL;
			TCString tc_reply;
			TCStringList tc_str_list;
			TCString sos_type;			
			redisReply * reply_lre = NULL;
			reply_lre = (redisReply *)redisCommand(redis_context, "LRANGE WuDongDe:BusiAlarmData:FromBusinessMgr 0 -1");
			if (reply_lre->type == REDIS_REPLY_NIL || reply_lre->type == REDIS_REPLY_ERROR)
			{
				freeReplyObject(reply_lre);
				continue;
			}
			// AT+AQMSOSCON=4672815,1,10,END
			TCString sos_flag = "1";
			TCString sos_interval = "30";
			TCString sos_send_buf = "AT+AQMSOSCON=000000,1,180,END";
			int snd_cnt = 3;

			if (reply_lre->type == REDIS_REPLY_ARRAY)
			{

				
				for (size_t i = 0; i < reply_lre->elements; i++)
				{
					printf("list: %s\n", reply_lre->element[i]->str);
					str_reply = reply_lre->element[i]->str;
					tc_reply = str_reply;
					tc_str_list.CommaText(tc_reply, ',');
					sos_type = tc_str_list[0];
					//
					LOG_WRITE("m_imsi:%s,alert:%s", (char *)m_sImsi, (char *)tc_str_list[1]);
					if (sos_type == "SOSA"/* && !sos_active*/)
					{
						SendRespPkg(cusSocket, sos_send_buf);
						now_b = TCTime::Now();
						LOG_WRITE("Ⱥ������[%s]", (char *)m_sImsi);
						/*sos_active = true;*/
						break;
					} else if (sos_type == "SOSS" && (tc_str_list[1] == m_sImsi)/* && !sos_active*/){
						LOG_WRITE("��������[%s]", (char *)tc_str_list[1]);
						redisReply * reply_lrm = (redisReply *)redisCommand(redis_context, "LREM WuDongDe:BusiAlarmData:FromBusinessMgr 0 %s", str_reply);
						freeReplyObject(reply_lrm);
						for (size_t i = 0; i < snd_cnt; i++)
						{
							SendRespPkg(cusSocket, sos_send_buf);
						}
						
						//now_b = TCTime::Now();
						/*sos_active = true;*/
					}
					else {
						;
					}
					LOG_WRITE("ssos_type: %s: %s", (char *)sos_type, (char *)m_sImsi)
				}
			}	
			freeReplyObject(reply_lre);
			redisReply * reply_lrm = (redisReply *)redisCommand(redis_context, "LREM WuDongDe:BusiAlarmData:FromBusinessMgr 0 SOSA,ALL");
			freeReplyObject(reply_lrm);
			//// TODO: �·�����ȡ��--60
			//if (!now_b.IsEmpty() && TCTime::RelativeTime(now_b, 60) <= TCTime::Now() && sos_active)
			//{
			//	sos_flag = "0";
			//	LOG_WRITE("�·�����ȡ��");
			//	SendRespPkg(cusSocket, sos_send_buf);
			//	
			//	redisReply * reply_lrm = (redisReply *)redisCommand(redis_context, "LREM WuDongDe:BusiAlarmData:FromBusinessMgr 0 SOSA,ALL");
			//	freeReplyObject(reply_lrm);
			//	sos_active = false;
			//}	
		}
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return true;
	}
	catch (TCException &e) {
		TCString sLogMsg = TCString("At TCDataCollectionLWHandler::Main_Handler ��������ʱ����:") + e.GetExceptionMessage();
		glgMls->AddDataCollectionRunLog(sLogMsg);
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return false;
	}
	catch (...) {
		glgMls->AddDataCollectionRunLog("At TCDataCollectionLWHandler::Main_Handler Unknow, �ر�Socket����");
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return false;
	}
}

//==========================================================================
// ���� : TCDataCollectionLWHandler::DealRequest
// ��; : ��������
// ԭ�� : void DealRequest()
// ���� : ��
// ���� : ��
// ˵�� :
//==========================================================================
void TCDataCollectionLWHandler::DealRequest(TCCustomUniSocket  &cusSocket) {
	try {
		// ���������ֽ��д���
#ifdef __TEST__
		printf("============receive Tracker request==============\n");
		printf("Command:=%s, Content=[%s], Time=[%s]\n", (char*)m_sReq_Command, (char*)m_sPkg_Content, (char*)TCTime::Now());
#endif

		for (size_t i = 0; i < m_vsPkgList.size(); i++)
		{
			m_sPkg_Content = m_vsPkgList[i];
			unsigned char chk = (unsigned char)m_sPkg_Content[144];
			if (chk != get_check_code()) {
				LOG_WRITE("pkg check failed!!!");
				continue;
			}
			//======ִ�з������==========================
			m_nMsgId = AscStrToInt(Mid(m_sPkg_Content, 1, 2));
			if (m_nMsgId == 0xAA76) {
				// GPS LBS��λ����
				DoCommand_GPSLBSLocInfo(cusSocket);
			}
			else {
				;
			}
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
// ���� : TCDataCollectionLWHandler::RecvRequest(TCCustomUniSocket  &cusSocket)
// ��; : �t�����ҵ����ݴ���ʽ����������, ��������������
// ԭ�� : void RecvRequest()
// ���� : ��
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionLWHandler::RecvRequest(TCCustomUniSocket  &cusSocket) {
	try {
		const int nMaxPkg_Length = 1024 * 1000;
		short const pkg_len = 144;
		char sbuff[nMaxPkg_Length];
		memset(sbuff, 0, sizeof(sbuff));
		

		m_sReq_Command = "";
		m_sPkg_Content = "";

		int nTimeOut = 600 * 1000;

		// ��ʼ���� select �������ж�
		if (!cusSocket.WaitForData(nTimeOut)) {
			// ��ʱ120���ˣ�����ֱ�ӹر����ӣ�
			TCString sLog = "LW Tracker[" + m_sTacker_Send_IPAddress + "]�Ѿ���ʱ120��δ�������ݣ��Ͽ�����";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}

		int nGetPkgLen = 0;
		nGetPkgLen = cusSocket.ReceiveBuf(sbuff, nMaxPkg_Length);

		m_sRow_Pkt = "";
		m_sRow_Pkt += "Recv Raw Packet Length=[" + IntToStr(nGetPkgLen) + "],RawPacket HexStr=[\n";

		//		printf("\nSrc Code: Length=[%d]\n", nGetPkgLen);
		for (int nSeq = 0; nSeq<nGetPkgLen; nSeq++) {
			//			printf("[%02x]:", sbuff[nSeq]);
			char sBuff[32];
			memset(sBuff, 0, sizeof(sBuff));
			sprintf(sBuff, "%02x:", sbuff[nSeq]);
			m_sRow_Pkt += Right(TCString(sBuff), 3);
		}
		m_sRow_Pkt += "]\n";
		printf("%s\n", (char*)m_sRow_Pkt);
		glgMls->AddDataCollectionRunLog(m_sRow_Pkt);
		//		printf("\n\n");


		if (nGetPkgLen == 0) {
			// ��ʱ120���ˣ�����ֱ�ӹر����ӣ�
			TCString sLog = "LW Tracker[" + m_sTacker_Send_IPAddress + "]�Ѿ��Ͽ�����";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}

		TCString sPkg_Content(sbuff, nGetPkgLen);

		TCString sLogMsg = "Time:Now=[" + TCTime::Now() + "],Recv Port=[" + IntToStr(cusSocket.GetRemotePort()) + "] Data";
		glgMls->AddDataCollectionRunLog(sLogMsg);

		if (Length(sPkg_Content) < pkg_len || (unsigned char)sPkg_Content[1] != 0xAA) {
			// �쳣����������
			TCString sLogMsg;
			sLogMsg = "";
			sLogMsg += TCString("ERROR: �����ĳ����쳣=[") + IntToStr(Length(sPkg_Content)) + "]";
			glgMls->AddDataCollectionRunLog(sLogMsg);
			printf("Error Packet Format: %s\n", (char*)sLogMsg);
			cusSocket.Close();
			throw TCException(sLogMsg);
			//return;
		}
		
		// m_vsPkgList
		m_vsPkgList.clear();
		TCString pkg_tmp = "";

		for (size_t i = 0; i < nGetPkgLen / pkg_len; i++)
		{
			pkg_tmp = Mid(sPkg_Content, pkg_len * i + 1, pkg_len);
			m_vsPkgList.push_back(pkg_tmp);
		}

	}
	catch (TCException &e) {
		cusSocket.Close();
		throw e;
	}
	catch (...) {
		cusSocket.Close();
		throw TCException("TCDataCollectionLWHandler::RecvRequest() Error:"
			"Unknown Exception.");
	}
}

//==========================================================================
// ���� : TCDataCollectionLWHandler::SendRespPkg
// ��; : ����Ӧ���
// ԭ�� : void SendRespPkg(TCString sRespCommand, TCString sRespContent)
// ���� : sRespCommand---�����������   sRespContent ---- ��������(ǰ�����ֽھ���������)
// ���� : ��
// ˵�� :
//==========================================================================
void TCDataCollectionLWHandler::SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespContent) {
	TCString snd_buf = sRespContent;
	m_sSendTime = TCTime::Now();
	LOG_WRITE("send time[%s],send content:%s", (char *)m_sSendTime, (char *)snd_buf);
	cusSocket.SendBuf((char *)snd_buf, snd_buf.GetLength());
	

#ifdef __TEST__		
	printf("RecvTime=[%s], SendTime=[%s]\n", (char*)m_sRecvTime, (char*)m_sSendTime);
#endif

}

//==========================================================================
// ���� : void TCDataCollectionLWHandler::DoCommand_GPSLocInfo
// ��; : �t���Ŀɴ���ʽ�ն��ϱ�GPS��LBSλ����Ϣ
// ԭ�� : void DoCommand_GPSLocInfo()
// ���� : ��
// ���� : ��
// ˵�� :  
//==========================================================================
void TCDataCollectionLWHandler::DoCommand_GPSLBSLocInfo(TCCustomUniSocket  &cusSocket) {
	// �ֽ��û�����
	//
	__ENTERFUNCTION;
	std::string pkg_str = hex_to_string(m_sPkg_Content, m_sPkg_Content.GetLength());
	LOG_WRITE("[source]:%s", pkg_str.c_str());
	m_sDev_ID = (char *)hex_to_string(Mid(m_sPkg_Content, 3, 5), 3).c_str();
	// ʱ���ʽ
	TCString sTempTimer = Mid(m_sPkg_Content, 8, 6);
	TCString sTimeStamp = "";
	// year
	TCString time_tmp = IntToStr(sTempTimer[1]);
	if (Length(time_tmp) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += time_tmp;

	// month
	time_tmp = IntToStr(sTempTimer[2]);
	if (Length(time_tmp) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += time_tmp;

	// day
	time_tmp = IntToStr(sTempTimer[3]);
	if (Length(time_tmp) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += time_tmp;

	// hour
	time_tmp = IntToStr(sTempTimer[4]);
	if (Length(time_tmp) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += time_tmp;

	// minite
	time_tmp = IntToStr(sTempTimer[5]);
	if (Length(time_tmp) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += time_tmp;

	// second
	time_tmp = IntToStr(sTempTimer[6]);
	if (Length(time_tmp) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += time_tmp;

	printf("sTimeStamp=[%s]\n", (char*)sTimeStamp);
	TCString sBJ_Time = "";

	if (!TCTime::IsValidDatetime(sTimeStamp)) {
		TCString sLog = "FATAL ERROR: INVALID TIMER=[" + sTimeStamp + "]\n";
		glgMls->AddDataCollectionRunLog(sLog);
		sBJ_Time = TCTime::Now();
	}
	else {
		// ת���ɱ���ʱ��
		sBJ_Time = TCTime::RelativeTime(sTimeStamp, 8 * 3600);
		printf("sBJ_Time=[%s]\n", (char*)sBJ_Time);
	}

	

	RecordInfo info;
	info.clear();

	// �߶���Ϣ		
	char sHigh_Level[32] = { 0 };
	sprintf(sHigh_Level, "%02d%02d.%02d", m_sPkg_Content[26], m_sPkg_Content[27], m_sPkg_Content[28]);
	info.elevation_ = StrToFloat(TCString(sHigh_Level));

	// ��λ״̬��
	int nLoc_Type = (int)m_sPkg_Content[29];
	if (nLoc_Type == 0)	{
		info.longitude_ = 0.0000;
		info.latitude_ = 0.0000;
	} else {
		// ��γ�ȴ���
		info.latitude_ = FloatToStr(gps_lac_lon_deal(Mid(m_sPkg_Content, 14, 6)), 6);
		info.longitude_ = FloatToStr(gps_lac_lon_deal(Mid(m_sPkg_Content, 20, 6)), 6);
	}

	// SOS״̬
	info.sos_flag_ = (int)m_sPkg_Content[142];

	// С��CELLID��Ϣ
	info.cellid_ = AscStrToInt(Mid(m_sPkg_Content, 36, 4));
	info.lac_ = AscStrToInt(Mid(m_sPkg_Content, 40, 3));
	// RSCP
	info.rscp_ = (int)m_sPkg_Content[43];
	// RXLEV
	info.rxlev_ = (int)m_sPkg_Content[44];
	// ��ص�ѹ���Է�Ϊ��λ��ASCII�ֽ���
	info.electricity_ = ((int)(m_sPkg_Content[143]));
	// �ֻ���
	info.phone_no = (char *)hex_to_string(Mid(m_sPkg_Content, 45, 8), 8).substr(5, 13).c_str();
	// �ֻ�imsi
	int imsi_len = 15;
	std::string imsi_tmp = hex_char(Mid(m_sPkg_Content, 53, imsi_len), imsi_len);
	/*info.imsi = (char *)imsi_tmp.substr(imsi_tmp.find_first_not_of('0', 0), imsi_tmp.length()).c_str();*/
	
	info.imsi = (char *)imsi_tmp.c_str();
	m_sImsi = info.imsi;
	//
	info.time_stamp = TCTime::Now();

	// ������
	unsigned char  gyro_ch = (unsigned char)m_sPkg_Content[31];
	short gyro_x = m_sPkg_Content[30]==0x00 ? (short)gyro_ch
		: 0 - (short)gyro_ch;
	gyro_ch = (unsigned char)m_sPkg_Content[33];
	short gyro_y = m_sPkg_Content[32] == 0x00 ? (short)gyro_ch
		: 0 - (short)gyro_ch;
	gyro_ch = (unsigned char)m_sPkg_Content[35];
	short gyro_z = m_sPkg_Content[34] == 0x00 ? (short)gyro_ch
		: 0 - (short)gyro_ch;

	// ����
	TCString blue_str = "";
	blue_str = Mid(m_sPkg_Content, 73, 64);
	std::vector<std::string> blue_list;
	parse_blue_list(blue_str, blue_list);
	for (size_t i = 0; i < 6; i++)
	{
		info.mac_rssi[i] = (char *)blue_list[i].c_str();
	}
	LOG_WRITE("[parse]latitude:%s,longitude:%s,cellid:%d,lac:%d,imsi:%s,phone_no:%s,rxlev:%f,electricity:%d,elevation:%f,gyro[x:%d,y:%d,z:%d],sos_flag:%d",
		(char *)info.latitude_, (char *)info.longitude_, info.cellid_, info.lac_, (char *)info.imsi,
		(char *)info.phone_no, info.rxlev_, info.electricity_, info.elevation_, gyro_x, gyro_y, gyro_z, info.sos_flag_);
	LocationRecordToRedis(info);
	__LEAVEFUNCTION;
}

//==========================================================================
// ���� : void TCDataCollectionLWHandler::Init
// ��; : ��ʼ��
// ԭ�� : void Init()
// ���� : ��
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionLWHandler::Init() {
	m_sPkg_Content = "";
	m_sReq_Command = "";
	m_sRespCommand = "ONSNETUNOK";
	m_sRespPkgContent = "";
	return;
}

bool TCDataCollectionLWHandler::LocationRecordToRedis(RecordInfo record_info)
{
	std::string str_csv = record_info.putcsv();

	redisReply * ry_lw = NULL;
	//
	// ����imsiȥLOC_POSTERM_INFO��ȡ��������
	ry_lw = (redisReply *)redisCommand(redis_context, "HGET LOC_POSTERM_INFO %s", (char *)record_info.imsi);
	if (ry_lw->type == REDIS_REPLY_NIL || ry_lw->type == REDIS_REPLY_ERROR) {
		LOG_WRITE("LOC_POSTERM_INFO���޶�Ӧ��imsi:[%s]", (char *)record_info.imsi);
		freeReplyObject(ry_lw);
		ry_lw = NULL;
		return false;
	}

	TCString term_type = "";
	if (ry_lw->type == REDIS_REPLY_STRING) {
		TCString str = ry_lw->str;
		TCStringList str_list;
		str_list.CommaText(str, ',');
		if (str_list.GetCount() < 1) {
			freeReplyObject(ry_lw);
			return false;
		}
		else {
			term_type = str_list[1];
		}
	}
	freeReplyObject(ry_lw);
	ry_lw = NULL;

	term_type = AllTrim(term_type);
	record_info.vender = term_type;
	//
	if (record_info.sos_flag_ == 1)
	{
		// TODO: ���뵽������Ϣ����
		ry_lw = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:SOSAlarmData:FromDataCollection %s", (char *)str_csv.c_str());
		if (ry_lw->type == REDIS_REPLY_ERROR)
		{
			LOG_WRITE("TCDataCollectionLWHandler::RecordLWTrackerLocInfo::REDIS_REPLY_ERROR");
		}
		freeReplyObject(ry_lw);
	}
	// TODO: ���뵽��λ��Ϣ����-WuDongDe:PositionData:FromDataCollection
	ry_lw = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromDataCollection %s", (char *)str_csv.c_str());
	if (ry_lw->type == REDIS_REPLY_ERROR)
	{
		perror("error:LPUSH WuDongDe:PositionData:FromDataCollection");
	}
	freeReplyObject(ry_lw);
	// LBS �������
	ry_lw = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromLBSCollection %s", (char *)str_csv.c_str());
	if (ry_lw->type == REDIS_REPLY_ERROR)
	{
		perror("error:LPUSH WuDongDe:PositionData:FromLBSCollection");
	}
	freeReplyObject(ry_lw);
	// ��ԭʼ��¼�������
	ry_lw = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:SrcRecordForOracle %s", (char *)str_csv.c_str());
	if (ry_lw->type == REDIS_REPLY_ERROR)
	{
		perror("error:LPUSH WuDongDe:SrcRecordForOracle");
	}
	freeReplyObject(ry_lw);
	ry_lw = NULL;

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
	str_latest_data += IntToStr(record_info.sos_flag_);
	str_latest_data += ",";
	str_latest_data += "";
	str_latest_data += ",";
	str_latest_data += term_type;

	ry_lw = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:PostermLastDataHash %s %s",
		(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
	freeReplyObject(ry_lw);
	ry_lw = NULL;
	//std::cout << "TCDataCollectionLWHandler::RecordLWTrackerLocInfo::return" << std::endl;

	// --20161026
	// // �ն�״̬ȷ��
	str_latest_data = (char *)record_info.time_stamp;
	str_latest_data += ",";
	str_latest_data += term_type;

	ry_lw = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:TermStatusCheckHash %s %s",
		(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
	freeReplyObject(ry_lw);
	ry_lw = NULL;

	return true;
}

double TCDataCollectionLWHandler::gps_lac_lon_deal(TCString lac_lon)
{
	assert(lac_lon.GetLength() == 6);
	int degree = (int)lac_lon[2];
	int fen_integer = (int)lac_lon[3];
	double fen_fraction = AscStrToInt(Mid(lac_lon, 4, 3))/1000000.0;
	double fen = fen_integer + fen_fraction;
	double degree_fraction = fen / 60;

	return (degree*1.0 + degree_fraction);
}

void TCDataCollectionLWHandler::parse_blue_list(TCString const & blue_str,
	std::vector<std::string> & blue_list)
{
	blue_list.clear();
	std::string str_tmp((char *)blue_str, blue_str.GetLength());
	std::string bt;

	int const blue_cnt = blue_str.GetLength() / 8;
	for (size_t i = 0; i < blue_cnt; i++)	{
		bt = str_tmp.substr(8 * i, 6);
		bt = hex_to_string(bt.c_str(), bt.size());
		LOG_WRITE("mac:%d[%s]", i, bt.c_str());
		bt += "|0";
		blue_list.push_back(bt);
	}
}

unsigned char TCDataCollectionLWHandler::get_check_code()
{
	TCString chk_str = Mid(m_sPkg_Content, 8, 136);
	unsigned char res = (unsigned char)chk_str[1];
	for (int i = 2; i <= chk_str.GetLength(); i++) {
		res += (unsigned char)chk_str[i];
	}
	return (res & 0xff);
}

void TCDataCollectionLWHandler::pkg_resp(TCCustomUniSocket & cusSocket)
{
	TCString resp = "BD+DATAOK";

	cusSocket.SendBuf((char *)resp, resp.GetLength());
}



