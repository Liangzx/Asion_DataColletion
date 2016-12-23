/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 瞭望设备协议处理类
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



//: 数据库连接
extern otl_connect m_dbConnect;


extern TCDataCollectionLog  *glgMls;

extern map<str_Lac_Cell_Node, str_Space_Pos_Unit> g_msCellInfo_PosUnit;

//==========================================================================
// 函数 : TCDataCollectionLWHandler::TCDataCollectionLWHandler
// 用途 : 构造函数
// 原型 : TCDataCollectionLWHandler()
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionLWHandler::TCDataCollectionLWHandler() {
	redis_context = NULL;
}

//==========================================================================
// 函数 : TCDataCollectionLWHandler::~TCDataCollectionLWHandler()
// 用途 : 析构函数
// 原型 : ~TCDataCollectionLWHandler()
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionLWHandler::~TCDataCollectionLWHandler()
{
	if (redis_context != NULL)
	{
		redisFree(redis_context);
	}
}

//==========================================================================
// 函数 : TCDataCollectionLWHandler::Main_Handler
// 用途 : 主要实现具体的业务交互逻辑
// 原型 : void Main_Handler()
// 参数 :
// 返回 :
// 说明 : 主要实现具体的业务交互逻辑
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
		// redis 密码验证
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
		//: 远程发送的地址
		m_sTacker_Send_IPAddress = cusSocket.GetRemoteAddress();
		m_nTimeOut = 60;

		printf("接收到 Tracker 数据请求：IP=[%s], Port=[%d], Timeout=[%d]\n", (char*)cusSocket.GetRemoteAddress(), cusSocket.GetRemotePort(), m_nTimeOut);

		//: 长连接方式
		bool sos_active = false;
		TCString now_b = "";
		while (1) {
			m_sRecvTime = TCTime::Now();
			Init();
			TCString sLogMsg = "Time:Now=[" + TCTime::Now() + "],Recv Port=[" + IntToStr(cusSocket.GetRemotePort()) + "] Data";
			glgMls->AddDataCollectionRunLog(sLogMsg);

			RecvRequest(cusSocket);
			DealRequest(cusSocket);
			// TODO:检查是否有下发的警告消息
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
			if (reply_lre->type == REDIS_REPLY_ARRAY)
			{
				for (size_t i = 0; i < reply_lre->elements; i++)
				{
					printf("list: %s\n", reply_lre->element[i]->str);
					str_reply = reply_lre->element[i]->str;
					tc_reply = str_reply;
					tc_str_list.CommaText(tc_reply, ',');
					sos_type = tc_str_list[0];
					TCString res_cmd = "ONS2A";
					res_cmd += m_sDev_ID;
					res_cmd += "B1";
					res_cmd += "60";
					res_cmd += "END";
					if (sos_type == "SOSA" && !sos_active)
					{
						SendRespPkg(cusSocket, res_cmd, res_cmd);
						now_b = TCTime::Now();
						LOG_WRITE("群发警告");
						sos_active = true;
						break;
					}
					else if (sos_type == "SOSS" && (tc_str_list[1] == m_sDev_ID) && !sos_active){
						LOG_WRITE("单发警告");
						redisReply * reply_lrm = (redisReply *)redisCommand(redis_context, "LREM WuDongDe:BusiAlarmData:FromBusinessMgr 0 %s", str_reply);
						freeReplyObject(reply_lrm);
						SendRespPkg(cusSocket, res_cmd, res_cmd);
						now_b = TCTime::Now();
						sos_active = true;
					}
					else {
						;
					}
//					LOG_WRITE("ssos_type: %s: %s", (char *)sos_type, (char *)m_sDev_ID)
				}
			}	
			freeReplyObject(reply_lre);
			// TODO: 下发警告取消
			if (!now_b.IsEmpty() && TCTime::RelativeTime(now_b, 60) <= TCTime::Now() && sos_active)
			{
				LOG_WRITE("下发警告取消");
				TCString res_cmd = "ONS2A";
				res_cmd += m_sDev_ID;
				res_cmd += "B060END";
				SendRespPkg(cusSocket, res_cmd, res_cmd);
				
				redisReply * reply_lrm = (redisReply *)redisCommand(redis_context, "LREM WuDongDe:BusiAlarmData:FromBusinessMgr 0 SOSA,ALL");
				freeReplyObject(reply_lrm);
				sos_active = false;
			}	
		}
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return true;
	}
	catch (TCException &e) {
		TCString sLogMsg = TCString("At TCDataCollectionLWHandler::Main_Handler 处理请求时出错:") + e.GetExceptionMessage();
		glgMls->AddDataCollectionRunLog(sLogMsg);
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return false;
	}
	catch (...) {
		glgMls->AddDataCollectionRunLog("At TCDataCollectionLWHandler::Main_Handler Unknow, 关闭Socket连接");
		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
		return false;
	}
}

//==========================================================================
// 函数 : TCDataCollectionLWHandler::DealRequest
// 用途 : 处理请求
// 原型 : void DealRequest()
// 参数 : 无
// 返回 : 无
// 说明 :
//==========================================================================
void TCDataCollectionLWHandler::DealRequest(TCCustomUniSocket  &cusSocket) {
	try {
		//: 按照命令字进行处理；
#ifdef __TEST__
		printf("============receive Tracker request==============\n");
		printf("Command:=%s, Content=[%s], Time=[%s]\n", (char*)m_sReq_Command, (char*)m_sPkg_Content, (char*)TCTime::Now());
#endif

		//======执行服务调用==========================
		m_sRespPkgContent = "";

		if (m_sReq_Command == "ONS3") {
			//: GPS LBS定位服务
			DoCommand_GPSLBSLocInfo(cusSocket);
		}
		else {
			;
		}

	}
	catch (TCException &e) {
		try {
			TCString sLogMsg = TCString("系统错误,At DealRequest: ") + e.GetExceptionMessage();
			glgMls->AddDataCollectionRunLog(sLogMsg);
		}
		catch (TCException &e1) {
			TCString sLogMsg = TCString("系统错误: when Catch a Error and SendRespPkg: ") + e1.GetExceptionMessage();
			glgMls->AddDataCollectionRunLog(sLogMsg);
		}
		catch (...) {
			glgMls->AddDataCollectionRunLog("系统发生未知异常");
		}
	}
}


//==========================================================================
// 函数 : TCDataCollectionLWHandler::RecvRequest(TCCustomUniSocket  &cusSocket)
// 用途 : 瞭望厂家的数据处理方式，接收数据, 按照无阻塞接收
// 原型 : void RecvRequest()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionLWHandler::RecvRequest(TCCustomUniSocket  &cusSocket) {
	try {
		const int nMaxPkg_Length = 1024 * 1000;
		char sbuff[nMaxPkg_Length];
		memset(sbuff, 0, sizeof(sbuff));

		m_sReq_Command = "";
		m_sPkg_Content = "";

		int nTimeOut = 600 * 1000;

		//: 开始按照 select 来进行判断
		if (!cusSocket.WaitForData(nTimeOut)) {
			//: 超时120秒了，可以直接关闭连接；
			TCString sLog = "LW Tracker[" + m_sTacker_Send_IPAddress + "]已经超时120秒未发送数据，断开连接";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}

		int nGetPkgLen = 0;
		nGetPkgLen = cusSocket.ReceiveBuf(sbuff, nMaxPkg_Length);
		m_sBit_Pkt = "";
		m_sBit_Pkt = TCString(sbuff, nGetPkgLen);

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
			//: 超时120秒了，可以直接关闭连接；
			TCString sLog = "LW Tracker[" + m_sTacker_Send_IPAddress + "]已经断开连接";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}

		TCString sPkg_Content(sbuff, nGetPkgLen);

		TCString sLogMsg = "Time:Now=[" + TCTime::Now() + "],Recv Port=[" + IntToStr(cusSocket.GetRemotePort()) + "] Data";
		glgMls->AddDataCollectionRunLog(sLogMsg);

		if (Length(sPkg_Content)<4) {
			//: 异常，请检查数据
			TCString sLogMsg;
			sLogMsg = "";
			sLogMsg += TCString("ERROR: 请求报文长度异常=[") + IntToStr(Length(sPkg_Content)) + "]";
			glgMls->AddDataCollectionRunLog(sLogMsg);
			printf("Error Packet Format: %s\n", (char*)sLogMsg);
			cusSocket.Close();
			throw TCException(sLogMsg);
			//return;
		}

		m_sReq_Command = Mid(sPkg_Content, 1, 4);
		m_cDelimter = ',';
		m_sPkg_Content = Mid(sPkg_Content, 5, Length(sPkg_Content) - 4);

		SendRespPkg(cusSocket, m_sRespCommand, m_sRespCommand);

		//======== 5. 记录日志 =============
		sLogMsg = "";
		sLogMsg += TCString("Req_Command:(") + m_sReq_Command;
		sLogMsg += TCString(") \t Pkg_Len(") + IntToStr(Length(m_sPkg_Content));
		sLogMsg += TCString(")\n");
		glgMls->AddDataCollectionRunLog(sLogMsg);
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
// 函数 : TCDataCollectionLWHandler::SendRespPkg
// 用途 : 发送应答包
// 原型 : void SendRespPkg(TCString sRespCommand, TCString sRespContent)
// 参数 : sRespCommand---返回命令代码   sRespContent ---- 返回内容(前三个字节就是命令码)
// 返回 : 无
// 说明 :
//==========================================================================
void TCDataCollectionLWHandler::SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespCommand, TCString sRespContent) {
	//: 平台无应答
	sRespContent = m_sRespCommand;

	cusSocket.SendBuf((char *)sRespContent, Length(sRespContent));

	m_sSendTime = TCTime::Now();

#ifdef __TEST__		
	printf("RecvTime=[%s], SendTime=[%s]\n", (char*)m_sRecvTime, (char*)m_sSendTime);
#endif

}

//==========================================================================
// 函数 : void TCDataCollectionLWHandler::DoCommand_GPSLocInfo
// 用途 : 瞭望的可穿戴式终端上报GPS及LBS位置信息
// 原型 : void DoCommand_GPSLocInfo()
// 参数 : 无
// 返回 : 无
// 说明 :  
//==========================================================================
void TCDataCollectionLWHandler::DoCommand_GPSLBSLocInfo(TCCustomUniSocket  &cusSocket) {
	//: 分解用户数据

	//: 设备串号
	//TCString sImei = IntToStr(AscStrToInt(Mid(m_sPkg_Content, 1, 4)));
	//m_sDev_ID = sImei;

	//printf("sImei=[%s]\n", (char*)sImei);

	//: 时间格式
	TCString sTempTimer = Mid(m_sPkg_Content, 5, 7);
	TCString sTimeStamp = "";
	//: Year_H
	TCString sYear_H = IntToStr(sTempTimer[1]);
	if (Length(sYear_H) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += sYear_H;

	//: sYear_T
	TCString sYear_T = IntToStr(sTempTimer[2]);
	if (Length(sYear_T) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += sYear_T;

	//: Month
	TCString sMonth = IntToStr(sTempTimer[3]);
	if (Length(sMonth) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += sMonth;

	//: Day
	TCString sDay = IntToStr(sTempTimer[4]);
	if (Length(sDay) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += sDay;

	//: Hour
	TCString sHour = IntToStr(sTempTimer[5]);
	if (Length(sHour) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += sHour;

	//: Minute
	TCString sMinute = IntToStr(sTempTimer[6]);
	if (Length(sMinute) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += sMinute;

	//: Second
	TCString sSecond = IntToStr(sTempTimer[7]);
	if (Length(sSecond) == 1) {
		sTimeStamp += "0";
	}
	sTimeStamp += sSecond;

	printf("sTimeStamp=[%s]\n", (char*)sTimeStamp);
	TCString sBJ_Time = "";

	if (!TCTime::IsValidDatetime(sTimeStamp)) {
		TCString sLog = "FATAL ERROR: INVALID TIMER=[" + sTimeStamp + "]\n";
		glgMls->AddDataCollectionRunLog(sLog);
		sBJ_Time = TCTime::Now();
	}
	else {
		//: 转换成北京时间
		sBJ_Time = TCTime::RelativeTime(sTimeStamp, 8 * 3600);
		printf("sBJ_Time=[%s]\n", (char*)sBJ_Time);
	}

	//: 纬度
	int nLatitude_Degree = m_sPkg_Content[12];
	int nLatitude_Minute = m_sPkg_Content[13];

	char sTemp_Dot[16];
	memset(sTemp_Dot, 0, sizeof(sTemp_Dot));
	sprintf(sTemp_Dot, "%02x", m_sPkg_Content[14]);
	TCString sLatitude_Dot = "." + Right(TCString(sTemp_Dot), 1);

	memset(sTemp_Dot, 0, sizeof(sTemp_Dot));
	sprintf(sTemp_Dot, "%02d", m_sPkg_Content[15]);

	sLatitude_Dot += TCString(sTemp_Dot);
	memset(sTemp_Dot, 0, sizeof(sTemp_Dot));
	sprintf(sTemp_Dot, "%02d", m_sPkg_Content[16]);

	sLatitude_Dot += TCString(sTemp_Dot);

	TCString sLatitude_Minute = IntToStr(nLatitude_Minute) + sLatitude_Dot;

	double dLatitude_Minute = StrToFloat(sLatitude_Minute);
	printf("dLatitude_Minute=[%f], nLatitude_Minute=[%d], StrToFloat(sLatitude_Minute)=[%.6f]\n", dLatitude_Minute, nLatitude_Minute, StrToFloat(sLatitude_Minute));

	double dLatitude = nLatitude_Degree*1.0 + dLatitude_Minute / 60;

	//: 经度处理
	int nLongtitude_High2Byte = m_sPkg_Content[18];

	//: 度分
	int nLong_Degr_Min = m_sPkg_Content[19];

	//: 经度的准确度
	TCString sLongtitude_Degree = IntToStr(nLongtitude_High2Byte) + Mid(IntToStr(nLong_Degr_Min), 1, 1);

	//: 分的后面2位
	char sBuff[3];
	memset(sBuff, 0, sizeof(sBuff));
	sprintf(sBuff, "%02x", m_sPkg_Content[20]);

	//: 度
	TCString sLongtitude_Minute = Mid(IntToStr(nLong_Degr_Min), 2, 1) + Mid(TCString(sBuff), 1, 1);

	memset(sTemp_Dot, 0, sizeof(sTemp_Dot));
	sprintf(sTemp_Dot, "%02d", m_sPkg_Content[21]);

	TCString sLongitude_Dot = "." + TCString(sTemp_Dot);

	memset(sTemp_Dot, 0, sizeof(sTemp_Dot));
	sprintf(sTemp_Dot, "%02d", m_sPkg_Content[22]);

	sLongitude_Dot += TCString(sTemp_Dot);

	memset(sTemp_Dot, 0, sizeof(sTemp_Dot));
	sprintf(sTemp_Dot, "%02d", m_sPkg_Content[23]);

	sLongitude_Dot += TCString(sTemp_Dot);

	sLongtitude_Minute = sLongtitude_Minute + sLongitude_Dot;

	double dLong_Minute = StrToFloat(sLongtitude_Minute) / 60;

	double dLongitude = StrToInt(sLongtitude_Degree)*1.0 + dLong_Minute;

	//: 位置信息
	str_Space_Pos_Unit sPosNode;
	sPosNode.clear();
	sPosNode.dLongitude = dLongitude;
	sPosNode.dLatitude = dLatitude;


	//: 高度信息		
	char sHigh_Level[32];
	memset(sHigh_Level, 0, sizeof(sHigh_Level));
	sprintf(sHigh_Level, "%02d%02d.%02d", m_sPkg_Content[25], m_sPkg_Content[26], m_sPkg_Content[27]);

	double dHigh = StrToFloat(TCString(sHigh_Level));

	//: 预留位： 3位

	//: 定位状态：
	int nLoc_Type = (int)m_sPkg_Content[31];
	if (nLoc_Type == 0)
	{
		sPosNode.dLongitude = 0.0000;
		sPosNode.dLatitude = 0.0000;
	}

	//: SOS状态：
	int nSOS_Status = (int)m_sPkg_Content[32];


	//: 小区CELLID信息；对应的十进制数据，转换为16进制后，前面16个BIT为RNCID， 后16BITS为CI号
	int nCellID = 0;
	memcpy(&nCellID, (char*)Mid(m_sPkg_Content, 35, 2), 2);

	//: LAC, 3字节保留
	int nLAC = 0;
	char sLacBuff[4];
	memset(sLacBuff, 0, sizeof(sLacBuff));
	sLacBuff[1] = m_sPkg_Content[37];
	sLacBuff[2] = m_sPkg_Content[38];
	sLacBuff[3] = m_sPkg_Content[39];

	memcpy(&nLAC, (char*)sLacBuff, 4);
	//		printf("nLAC=[%d]\n", nLAC);
	nLAC = ntohl(nLAC);
	//		printf("ntohl nLAC=[%d]\n", nLAC);

	//: RSCP
	int nRscp = (int)m_sPkg_Content[40];

	//: RXLEV
	int nRxLev = (int)m_sPkg_Content[41];

	//: 电池电压，以伏为单位，ASCII字节码
	int nPower_Percent = ((int)(m_sPkg_Content[42]));

	// 设备imei
	int intTmp = 0;
	//TCString imei = Mid(m_sPkg_Content, 43, 15);
	TCString imei;
	for (int i = 0; i < 15; i++)
	{
		if (m_sPkg_Content[43] == '0')
		{
			imei = "";
			break;
		}
		intTmp = m_sPkg_Content[43 + i];
		imei += IntToStr(intTmp);
	}
	printf("imei:%s\n", (char *)imei);

	// 手机imsi
	//TCString imsi = (char*)Mid(m_sPkg_Content, 58, 15);
	TCString imsi;
	for (int i = 0; i < 15; i++)
	{
		if (m_sPkg_Content[58] == '0')
		{
			imsi = "";
			break;
		}
		intTmp = m_sPkg_Content[58 + i];
		imsi += IntToStr(intTmp);
	}
	printf("imsi:%s\n", (char *)imsi);
	// 将imsi当成唯一id
	m_sDev_ID = imsi;
	// 手机号
	//TCString phone_no = (char*)Mid(m_sPkg_Content, 73, 11);
	TCString phone_no;
	for (int i = 0; i < 11; i++)
	{
		if (m_sPkg_Content[58] == '0')
		{
			phone_no = "";
			break;
		}
		intTmp = m_sPkg_Content[73 + i];
		phone_no += IntToStr(intTmp);
	}
	printf("phone_no:%s\n", (char *)phone_no);
	//: 位置信息
	str_Lac_Cell_Node sLBSNode;
	sLBSNode.clear();
	sLBSNode.nLac = nLAC;
	sLBSNode.nCellID = nCellID;


	TCString sLog = "DoCommand_GPSLocInfo: Imei=[" + imei \
		+ "],TimeStamp=[" + sBJ_Time + "], nLAC-CI=[" + IntToStr(sLBSNode.nLac) \
		+ "-" + IntToStr(sLBSNode.nCellID) + "],nRscp=[" + IntToStr(nRscp) + "],nRxLev=[" \
		+ IntToStr(nRxLev) + "], GPS=[" + FloatToStr(sPosNode.dLongitude, 6) + ","\
		+ FloatToStr(sPosNode.dLatitude, 6) + "], dHigh=[" + FloatToStr(dHigh, 3) \
		+ "], nPower_Percent=[" + IntToStr(nPower_Percent) + "%], nLoc_Type=[" + IntToStr(nLoc_Type) \
		+ "], nSOS_Status=[" + IntToStr(nSOS_Status) + "],[" + "phone_no:" + phone_no + "]" \
		+ "IMSI:" + imsi + "\n";
	glgMls->AddDataCollectionRunLog(sLog);
	double elevation = dHigh;
	RecordLWTrackerLocInfo(imei, sPosNode, sLBSNode, nRscp, nRxLev, nSOS_Status, nPower_Percent, imsi, phone_no, elevation);

	return;
}

//==========================================================================
// 函数 : void TCDataCollectionLWHandler::Init
// 用途 : 初始化
// 原型 : void Init()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionLWHandler::Init() {
	m_sPkg_Content = "";
	m_sReq_Command = "";
	m_sRespCommand = "ONSNETUNOK";
	m_sRespPkgContent = "";
	return;
}


//==========================================================================
// 函数 : TCDataCollectionLWHandler::RecordLWTrackerLocInfo
// 用途 : 用户位置信息登记
// 原型 : RecordLWTrackerLocInfo()
// 参数 : 无
// 返回 : 无
// 说明 : 用户位置信息登记
//==========================================================================
bool TCDataCollectionLWHandler::RecordLWTrackerLocInfo(TCString sImei, str_Space_Pos_Unit sPosNode, str_Lac_Cell_Node sLBSNode, int nRscp, int nRxLev, int nSOS_Status, int power_percent, TCString imsi, TCString phone_no, double elevation) {
	try
	{
		//std::cout << "TCDataCollectionLWHandler::RecordLWTrackerLocInfo::begin" << std::endl;
		RecordInfo record_info;
		record_info.clear();
		record_info.unique_id_ = sImei;
		record_info.vender = vender;
		record_info.longitude_ = FloatToStr(sPosNode.dLongitude, 6);
		record_info.latitude_ = FloatToStr(sPosNode.dLatitude, 6);
		record_info.lac_ = sLBSNode.nLac;
		record_info.cellid_ = sLBSNode.nCellID;
		record_info.sos_flag_ = nSOS_Status;
		record_info.imsi = imsi;
		record_info.phone_no = phone_no;
		record_info.elevation_ = elevation;

		str_Space_Pos_Unit sCurLBSPosInfo;
		sCurLBSPosInfo.clear();

		map<str_Lac_Cell_Node, str_Space_Pos_Unit>::iterator ITR_AdjLacCell_PosUnit;
		ITR_AdjLacCell_PosUnit = g_msCellInfo_PosUnit.find(sLBSNode);
		if (ITR_AdjLacCell_PosUnit != g_msCellInfo_PosUnit.end()) {
			//: 当前的小区信息;
			sCurLBSPosInfo = ITR_AdjLacCell_PosUnit->second;
		}
		record_info.cell_x = FloatToStr(sCurLBSPosInfo.dLongitude, 6);
		record_info.cell_y = FloatToStr(sCurLBSPosInfo.dLatitude, 6);

		record_info.rscp_ = nRscp;
		record_info.rxlev_ = nRxLev;
		record_info.imsi = imsi;
		record_info.time_stamp = TCTime::Now();
		std::string str_csv = record_info.putcsv();

		redisReply * ry_lw = NULL;
		if (nSOS_Status == 1)
		{
			// TODO: 放入到警告消息队列
			ry_lw = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:SOSAlarmData:FromDataCollection %s", (char *)str_csv.c_str());
			if (ry_lw->type == REDIS_REPLY_ERROR)
			{
				LOG_WRITE("TCDataCollectionLWHandler::RecordLWTrackerLocInfo::REDIS_REPLY_ERROR");
			}
			freeReplyObject(ry_lw);
		}
		// TODO: 放入到定位消息队列-WuDongDe:PositionData:FromDataCollection
		ry_lw = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromDataCollection %s", (char *)str_csv.c_str());
		if (ry_lw->type == REDIS_REPLY_ERROR)
		{
			perror("error:LPUSH WuDongDe:PositionData:FromDataCollection");
		}
		freeReplyObject(ry_lw);
		// LBS 计算队列
		ry_lw = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromLBSCollection %s", (char *)str_csv.c_str());
		if (ry_lw->type == REDIS_REPLY_ERROR)
		{
			perror("error:LPUSH WuDongDe:PositionData:FromLBSCollection");
		}
		freeReplyObject(ry_lw);
		// 供原始记录队列入库
		ry_lw = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:SrcRecordForOracle %s", (char *)str_csv.c_str());
		if (ry_lw->type == REDIS_REPLY_ERROR)
		{
			perror("error:LPUSH WuDongDe:SrcRecordForOracle");
		}
		freeReplyObject(ry_lw);
		ry_lw = NULL;

		// 终端最新一条记录的数据
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
		str_latest_data += "2";
		
		ry_lw = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:PostermLastDataHash %s %s",
			(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
		freeReplyObject(ry_lw);
		ry_lw = NULL;
		//std::cout << "TCDataCollectionLWHandler::RecordLWTrackerLocInfo::return" << std::endl;
		return true;
	}
	catch (otl_exception& p) { // intercept OTL exceptions	
		cerr << p.msg << endl; // print out error message
		cerr << p.stm_text << endl; // print out SQL that caused the error
		cerr << p.var_info << endl; // print out the variable that caused the error
		TCString sLog = "RecordLWTrackerLocInfo SQL Error: Msg=[" + TCString((char*)p.msg) + "] SQL=[" + TCString((char*)p.stm_text) + "]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
	catch (TCException &Excep) {
		TCString sLog = "RecordLWTrackerLocInfo 异常情况：" + Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
	catch (...) {
		TCString sLog = "RecordLWTrackerLocInfo 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}


