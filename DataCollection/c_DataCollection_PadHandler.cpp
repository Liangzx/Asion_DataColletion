/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : pad设备协议处理类
File Name   : c_DataCollection_PadHandler.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#pragma hdrstop

#include "c_DataCollection_PadHandler.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_task.h"
#include <iostream>


#pragma package(smart_init)

extern TCDataCollectionHandler gsDataCollectionHandler;

extern TCDataCollectionConfig *gDataCollectionConfig;


//: 数据库连接
extern otl_connect m_dbConnect;


extern TCDataCollectionLog  *glgMls; 

extern map<str_Lac_Cell_Node, str_Space_Pos_Unit> g_msCellInfo_PosUnit;

//==========================================================================
// 函数 : TCDataCollectionPadHandler::TCDataCollectionPadHandler
// 用途 : 构造函数
// 原型 : TCDataCollectionPadHandler()
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionPadHandler::TCDataCollectionPadHandler(){
	redis_context = NULL;
}

//==========================================================================
// 函数 : TCDataCollectionPadHandler::~TCDataCollectionPadHandler()
// 用途 : 析构函数
// 原型 : ~TCDataCollectionPadHandler()
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionPadHandler::~TCDataCollectionPadHandler()
{
	if (redis_context != NULL)
	{
		redisFree(redis_context);
	}
}

//==========================================================================
// 函数 : TCDataCollectionPadHandler::Main_Handler
// 用途 : 主要实现具体的业务交互逻辑
// 原型 : void Main_Handler()
// 参数 :
// 返回 :
// 说明 : 主要实现具体的业务交互逻辑
//==========================================================================
bool TCDataCollectionPadHandler::Main_Handler(TCCustomUniSocket  &cusSocket){
	try{
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
		m_sTacker_Send_IPAddress=cusSocket.GetRemoteAddress();
		m_nTimeOut=60;
		
		printf("接收到 Tracker 数据请求：IP=[%s], Port=[%d], Timeout=[%d]\n", (char*)cusSocket.GetRemoteAddress(), cusSocket.GetRemotePort(), m_nTimeOut);
		
		//TCString now_b = "";
		//: 长连接方式
		while(1){ 
			m_sRecvTime=TCTime::Now();
			Init();
			TCString sLogMsg="Time:Now=["+TCTime::Now()+"],Recv Port=["+IntToStr(cusSocket.GetRemotePort())+"] Data";
            glgMls->AddDataCollectionRunLog(sLogMsg);
 			RecvRequest(cusSocket);
 			DealRequest(cusSocket);	
			//// TODO:检查是否有下发的警告消息
			
			//redisReply * reply = (redisReply *)redisCommand(redis_context, "LRANGE WuDongDe:BusiAlarmData:FromBusinessMgr 0 -1");
			//char * str_reply = NULL;
			//TCString tc_reply;
			//TCStringList tc_str_list;
			//TCString unique_id_dev;
			//TCString sos_type;
			//bool sos_active = false;
			//if (reply->type == REDIS_REPLY_ARRAY)
			//{
			//	for (size_t i = 0; i < reply->elements; i++)
			//	{
			//		printf("list: %s\n", reply->element[i]->str);
			//		str_reply = reply->element[i]->str;
			//		tc_reply = str_reply;
			//		tc_str_list.CommaText(tc_reply, ',');
			//		if (tc_str_list[1] == m_sDev_ID) {
			//			redisReply * reply_lrm = (redisReply *)redisCommand(redis_context, "LREM WuDongDe:BusiAlarmData:FromBusinessMgr 0 %s", str_reply);
			//			freeReplyObject(reply_lrm);
			//		}
			//		else if (sos_type == "SOSS" && (tc_str_list[1] == m_sDev_ID) && !sos_active) {
			//			LOG_WRITE("单发警告");
			//			redisReply * reply_lrm = (redisReply *)redisCommand(redis_context, "LREM WuDongDe:BusiAlarmData:FromBusinessMgr 0 %s", str_reply);
			//			freeReplyObject(reply_lrm);
			//			SendRespPkg(cusSocket, res_cmd, res_cmd);
			//			now_b = TCTime::Now();
			//			sos_active = true;
			//		}
			//		else {
			//			;
			//		}
			//	}
			//}
			//freeReplyObject(reply);
			//// TODO: 下发警告取消
			//SendRespPkg(cusSocket, "", "");
 		}
 		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
 		return true;
  }catch(TCException &e){
    TCString sLogMsg = TCString("At TCDataCollectionPadHandler::Main_Handler 处理请求时出错:") + e.GetExceptionMessage();
    glgMls->AddDataCollectionRunLog(sLogMsg);
    cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
    return false;
  }catch(...){
    glgMls->AddDataCollectionRunLog("At TCDataCollectionPadHandler::Main_Handler Unknow, 关闭Socket连接");
    cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
    return false;
 	}
}

//==========================================================================
// 函数 : TCDataCollectionPadHandler::DealRequest
// 用途 : 处理请求
// 原型 : void DealRequest()
// 参数 : 无
// 返回 : 无
// 说明 :
//==========================================================================
void TCDataCollectionPadHandler::DealRequest(TCCustomUniSocket  &cusSocket){
	try{
		//: 按照命令字进行处理；
#ifdef __TEST__
   	printf("============receive Tracker request==============\n");
   	printf("Command:=%s, Content=[%s], Time=[%s]\n", (char*)m_sReq_Command, (char*)m_sPkg_Content, (char*)TCTime::Now());
#endif
	//======执行服务调用==========================
    m_sRespCommand="";
    m_sRespPkgContent="";
		if (m_sReq_Command=="66"){
    	DoCommand_PadLocInfo(cusSocket);
		} else if (m_sReq_Command == "88") {
			DoCommand_DeviceStatus(cusSocket);
		} else {
    	//: 未知命令
    	TCString sLogMsg = TCString ("未知命令,m_sReq_Command: [") + m_sReq_Command+"]";
      glgMls->AddDataCollectionRunLog(sLogMsg);
      return;
    }
  }catch(TCException &e){
  	try{
      TCString sLogMsg = TCString ("系统错误,At DealRequest: ") + e.GetExceptionMessage();
      glgMls->AddDataCollectionRunLog(sLogMsg);
    }catch(TCException &e1){
      TCString sLogMsg = TCString("系统错误: when Catch a Error and DealRequest: ") + e1.GetExceptionMessage();
      glgMls->AddDataCollectionRunLog(sLogMsg);
    }catch(...){
      glgMls->AddDataCollectionRunLog("系统发生未知异常");
    }
  }
}


//==========================================================================
// 函数 : TCDataCollectionPadHandler::RecvRequest(TCCustomUniSocket  &cusSocket)
// 用途 :	pad的数据处理方式，接收数据, 按照无阻塞接收
// 原型 : void RecvRequest()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionPadHandler::RecvRequest(TCCustomUniSocket  &cusSocket){
	try{
		const int nMaxPkg_Length=1024*1000;
		char sbuff[nMaxPkg_Length];	
		memset(sbuff, 0, sizeof(sbuff));
		m_vsPkgList.clear();
		m_sReq_Command="";
		m_sPkg_Content="";
				
		int nTimeOut= 600 * 1000;
		
		//: 开始按照 select 来进行判断
		if(!cusSocket.WaitForData(nTimeOut)){
			//: 超时120秒了，可以直接关闭连接；
			TCString sLog="Pad Tracker[" + m_sTacker_Send_IPAddress + "]已经超时120秒未发送数据，断开连接";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}
		
		int nGetPkgLen=0;
		nGetPkgLen=cusSocket.ReceiveBuf(sbuff, nMaxPkg_Length);

		if(nGetPkgLen==0){
			//: 超时120秒了，可以直接关闭连接；
			TCString sLog="Pad Tracker[" + m_sTacker_Send_IPAddress + "]已经断开连接";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}
			
		TCString sPkg_Content(sbuff, nGetPkgLen);
		
    if(Length(sPkg_Content)<4){
    	//: 异常，请检查数据
    	TCString sLogMsg;
			sLogMsg = "";
			sLogMsg += TCString("ERROR: 请求报文长度异常=[")+IntToStr(Length(sPkg_Content))+"]" ;
	  	glgMls->AddDataCollectionRunLog(sLogMsg);
    	printf("Error Packet Format: %s\n", (char*)sLogMsg);
    	//cusSocket.Close();
			throw TCException(sLogMsg);
    	//return ;
    }
    
    m_sReq_Command=Mid(sPkg_Content, 3, 2);
    m_cDelimter=',';
    m_sPkg_Content=Mid(sPkg_Content, 5, Length(sPkg_Content)-4);

    //: 需要判断后续是否有多条记录，如果是，就放到一个vector中保存;
    if(AT(m_sPkg_Content, "AT")>0) {
			//: 后面还有多条记录
			TCString cmd_str = "AT" + m_sReq_Command;
			m_sPkg_Content = cmd_str + m_sPkg_Content;
			m_sPkg_Content=ReplaceAllSubStr(m_sPkg_Content, cmd_str, ";");
			m_sPkg_Content = Mid(m_sPkg_Content, 2, Length(m_sPkg_Content));
			TCStringList lsRcdPkgList;
      lsRcdPkgList.Clear();
      lsRcdPkgList.CommaText(m_sPkg_Content, ';');
		for(int nPkgSeq=0; nPkgSeq<lsRcdPkgList.GetCount(); nPkgSeq++){
		m_vsPkgList.push_back(lsRcdPkgList[nPkgSeq]);
		//
		std::cout << (char *)lsRcdPkgList[nPkgSeq] << std::endl;
	}
    }else{
	m_vsPkgList.push_back(m_sPkg_Content);
    }
    
    //======== 5. 记录日志 =============
    TCString sLogMsg;
		sLogMsg = "";
		sLogMsg += TCString("请求命令码:(") + m_sReq_Command, +"), 记录数=("+IntToStr(m_vsPkgList.size());
		sLogMsg += TCString(") 内容：（ ") + m_sPkg_Content;
		sLogMsg += TCString(") 请求包结束\n");
    glgMls->AddDataCollectionRunLog(sLogMsg);
  }catch (TCException &e){
    cusSocket.Close();
    throw e;
  }catch (...){
    cusSocket.Close();
    throw TCException("TCDataCollectionPadHandler::RecvRequest() Error:"
                        "Unknown Exception.");
  }
}

//==========================================================================
// 函数 : TCDataCollectionPadHandler::SendRespPkg
// 用途 : 发送应答包
// 原型 : void SendRespPkg(TCString sRespCommand, TCString sRespContent)
// 参数 : sRespCommand---返回命令代码   sRespContent ---- 返回内容(前三个字节就是命令码)
// 返回 : 无
// 说明 :
//==========================================================================
void TCDataCollectionPadHandler::SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespCommand, TCString sRespContent){
	//: 平台无应答
	
	int nError=0;
	
//	if(StrToInt(Mid(TCTime::Now(), 11, 2))%10==0){
		//: 每次10分钟下发一次告警;
//		if(m_sCurAlarm_NotifyTimer=="" || m_sCurAlarm_NotifyTimer==Mid(TCTime::Now(), 1, 10)){
//			nError=1;
//			m_sCurAlarm_NotifyTimer=Mid(TCTime::Now(), 1, 10);
//		}
//	}
	
	TCString sTmpRespContent="AT0";
	sTmpRespContent+=IntToStr(nError);
	sTmpRespContent+=",TA";
	
  cusSocket.SendBuf((char *)sTmpRespContent, Length(sTmpRespContent));

	m_sSendTime=TCTime::Now();

#ifdef __TEST__		
	printf("RecvTime=[%s], SendTime=[%s], sTmpRespContent=[%s]\n", (char*)m_sRecvTime, (char*)m_sSendTime, (char*)sTmpRespContent);
#endif

}


//==========================================================================
// 函数 : void TCDataCollectionPadHandler::DoCommand_PadLocInfo
// 用途 : 平板上报位置信息
// 原型 : void DoCommand_PadLocInfo()
// 参数 : 无
// 返回 : 无
// 说明 :  
//==========================================================================
void TCDataCollectionPadHandler::DoCommand_PadLocInfo(TCCustomUniSocket  &cusSocket){
	field_counts = 15;
	for(int nPkgSeq=0; nPkgSeq<m_vsPkgList.size(); nPkgSeq++){
		//if(nPkgSeq>1){
		//	m_sPkg_Content=Mid(m_vsPkgList[nPkgSeq], 3, Length(m_vsPkgList[nPkgSeq])-2);
		//}else
		//	m_sPkg_Content=m_vsPkgList[nPkgSeq];
		m_sPkg_Content = m_vsPkgList[nPkgSeq];
		//: 分解用户数据
		TCStringList lsTrackerPkgList;
		lsTrackerPkgList.Clear();
		lsTrackerPkgList.CommaText(m_sPkg_Content, m_cDelimter);
		
		if(lsTrackerPkgList.GetCount() < field_counts){
			//: 至少15个数据域
			TCString sLogMsg;
			sLogMsg = "";
			sLogMsg += TCString("请求命令码:(") + m_sReq_Command;
			sLogMsg += TCString(") 报文异常，域数量=[") + IntToStr(lsTrackerPkgList.GetCount())+"]";
			glgMls->AddDataCollectionRunLog(sLogMsg);
			printf("报文异常TCDataCollectionPadHandler::DoCommand_PadLocInfo: %s\n", (char *)sLogMsg);
			//throw TCException(sLogMsg);
			continue;
		}
		
		//: 设备串号
		TCString sImei=lsTrackerPkgList[0];
		
		//: LAC，ASCII字节码，10进制
		int nLAC=StrToInt(lsTrackerPkgList[1]);
		
		//: 小区识别 RNCID+CI，ASCII字节码，10进制 对应的十进制数据，转换为16进制后，后16BITS为CI号，28BIT到17BIT为RNCID
		//char sBuff[32];
		//memset(sBuff, 0, sizeof(sBuff));
		//sprintf(sBuff, "%010x", StrToInt(lsTrackerPkgList[2]));
	
		//: 根据最后4个字符形成的字符串用16进转码形成10进制数字;
		//char sResult[32];
		//memset(sResult, 0, sizeof(sResult));
		//sprintf(sResult, "0x%s", (char*)Right(TCString(sBuff), 4));
		
		/*int nCellID=strtol((char*)sResult, NULL, 10);*/
		int nCellID = StrToInt(lsTrackerPkgList[2]);
		//std::cout << "lsTrackerPkgList[2]: " << (char *)lsTrackerPkgList[2] << std::endl;
		str_Lac_Cell_Node sLBSNode;
		sLBSNode.nLac=nLAC;
		sLBSNode.nCellID=nCellID;
		
		
		//: 信号强度，ASCII字节码
		int nRxLev=(StrToInt(lsTrackerPkgList[3]));

		//: 经度， ASCII字节码
		double dLongitude=StrToFloat(lsTrackerPkgList[4]);
		
		//: 纬度， ASCII字节码，
		double dLatitude=StrToFloat(lsTrackerPkgList[5]);

		//: 蓝牙 x
		double dBLE_X=StrToFloat(lsTrackerPkgList[6]);
		
		//: 蓝牙 y
		double dBLE_Y=StrToFloat(lsTrackerPkgList[7]);

		//: 蓝牙z
		double dBLE_Z = StrToFloat(lsTrackerPkgList[13]);
		
		//: MAC 地址
		TCString sBLE_MAC=lsTrackerPkgList[8];

		// 手机imsi
		TCString imsi = lsTrackerPkgList[11];

		// 手机号码
		TCString phone_no = lsTrackerPkgList[12];
		// 海拔高度
		double elevation = 0;
		
		//: 位置信息
		str_Space_Pos_Unit sPosNode;
		sPosNode.clear();
		sPosNode.dLongitude=dLongitude;
		sPosNode.dLatitude=dLatitude;
				
		printf("sImei=[%s], nLAC-CI=[%d-%d],nRxLev=[%d], GPS=[%.6f, %.6f], BLE_MAC=[%s],\
						dBLE_Site=[%.3f-%.3f-%.3f], phone_no %s\n",\
						(char*)sImei, nLAC, nCellID, nRxLev, dLongitude, dLatitude, \
						(char*)sBLE_MAC, dBLE_X, dBLE_Y, dBLE_Z, (char *)phone_no);
		
		TCString sLog="DoCommand_PadLocInfo: sImei=["+sImei+"],\
		nLAC-CI=["+IntToStr(nLAC)+"-"+IntToStr(nCellID)	+"],RSSI=["+IntToStr(nRxLev)+"],\
		GPS=["+FloatToStr(sPosNode.dLongitude,6)+","+FloatToStr(sPosNode.dLatitude,6) +"],\
	  BLEMAC=["+sBLE_MAC+"], X,Y=["+FloatToStr(dBLE_X,3)+","+FloatToStr(dBLE_Y,3) + +FloatToStr(dBLE_Z, 3) +"]" + ","\
		+ "phone_no:" + phone_no + "," + "imsi:" + imsi + "\n";
		glgMls->AddDataCollectionRunLog(sLog);
						
		RecordPadLocInfo(sImei, sPosNode, sLBSNode, nRxLev, dBLE_X, dBLE_Y, dBLE_Z, sBLE_MAC, imsi, phone_no, elevation);
	}
	return ;	
}

void TCDataCollectionPadHandler::DoCommand_DeviceStatus(TCCustomUniSocket & cusSocket)
{
	field_counts = 4;
	for (int nPkgSeq = 0; nPkgSeq < m_vsPkgList.size(); nPkgSeq++) {
		m_sPkg_Content = m_vsPkgList[nPkgSeq];
		//: 分解用户数据
		TCStringList lsTrackerPkgList;
		lsTrackerPkgList.Clear();
		lsTrackerPkgList.CommaText(m_sPkg_Content, m_cDelimter);

		if (lsTrackerPkgList.GetCount() < field_counts) {
			TCString sLogMsg;
			sLogMsg = "";
			sLogMsg += TCString("请求命令码:(") + m_sReq_Command;
			sLogMsg += TCString(") 报文异常，域数量=[") + IntToStr(lsTrackerPkgList.GetCount()) + "]";
			glgMls->AddDataCollectionRunLog(sLogMsg);
			printf("报文异常TCDataCollectionPadHandler::DoCommand_PadLocInfo: %s\n", (char *)sLogMsg);
			//throw TCException(sLogMsg);
			continue;
		}
		TCString blue_seq = lsTrackerPkgList[1];
		TCString blue_seq_power = lsTrackerPkgList[2];
		TCStringList blue_seq_list;
		TCStringList blue_seq_power_list;
		blue_seq_list.Clear();
		blue_seq_power_list.Clear();
		if (blue_seq.IsEmpty()) {
			continue;
		}
		blue_seq_list.CommaText(blue_seq, '|');
		if (!blue_seq_power.IsEmpty()) {
			blue_seq_power_list.CommaText(blue_seq_power, '|');
		}	
		if ((blue_seq_list.GetCount() != blue_seq_power_list.GetCount()) && !blue_seq_power.IsEmpty()) {
			printf("蓝牙个数序列与对应的电量序列不匹配.\n");
			continue;
		}
		TCString bt_time_stamp = TCTime::Now();
		redisReply * ry_blue = NULL;
		TCString blue_type = "4";
		for (int seq = 0; seq < blue_seq_list.GetCount(); seq++) {
			// 蓝牙最新一条记录的数据
			if (blue_seq_list[seq].IsEmpty()) {
				continue;
			}
			TCString blue_record;
			blue_record += bt_time_stamp;
			blue_record += ",";
			blue_record += "";
			blue_record += ",";
			blue_record += (blue_seq_power.IsEmpty() ? "-1" :  blue_seq_power_list[seq]);
			blue_record += ",";
			blue_record += "";
			blue_record += ",";
			blue_record += "";
			blue_record += ",";
			blue_record += "";
			blue_record += ",";
			blue_record += blue_type;

			ry_blue = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:PostermLastDataHash %s %s",
				(char *)AllTrim(blue_seq_list[seq]), (char *)blue_record);
			freeReplyObject(ry_blue);
			ry_blue = NULL;
		}
	}
	return;
}


//==========================================================================
// 函数 : void TCDataCollectionPadHandler::Init
// 用途 : 初始化
// 原型 : void Init()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionPadHandler::Init(){
	m_sPkg_Content="";
	m_sReq_Command="";
	m_sRespCommand="";
	m_sRespPkgContent="";
	return;
}


//==========================================================================
// 函数 : TCDataCollectionPadHandler::RecordTrackerLocInfo
// 用途 : 用户位置信息登记
// 原型 : RecordTrackerLocInfo()
// 参数 : 无
// 返回 : 无
// 说明 : 用户位置信息登记
//==========================================================================
bool TCDataCollectionPadHandler::RecordTrackerLocInfo(TCString sImei, str_Space_Pos_Unit sPosNode, str_Lac_Cell_Node sLBSNode, int nRscp, int nRxLev, int nStatus, double nhigh, int power){
	try{  
		RecordInfo record_info;
		record_info.clear();
		record_info.vender = vender;
		record_info.unique_id_ = sImei;
		record_info.longitude_ = FloatToStr(sPosNode.dLongitude,6);
		record_info.latitude_ = FloatToStr(sPosNode.dLatitude,6);
		record_info.lac_ = sLBSNode.nLac;
		record_info.cellid_ = sLBSNode.nCellID;
    
    str_Space_Pos_Unit sCurLBSPosInfo;
    sCurLBSPosInfo.clear();
    
    map<str_Lac_Cell_Node, str_Space_Pos_Unit>::iterator ITR_AdjLacCell_PosUnit;
		ITR_AdjLacCell_PosUnit=g_msCellInfo_PosUnit.find(sLBSNode);
		if(ITR_AdjLacCell_PosUnit!=g_msCellInfo_PosUnit.end()){
			//: 当前的小区信息;
			sCurLBSPosInfo=ITR_AdjLacCell_PosUnit->second;
		}
		record_info.cell_x = FloatToStr(sCurLBSPosInfo.dLongitude,6);
		record_info.cell_y = FloatToStr(sCurLBSPosInfo.dLatitude,6);
    
		record_info.rscp_ = nRscp;
		record_info.rxlev_ = nRxLev;
		record_info.useralarmstatus = nStatus;
		// 海拔电量
		record_info.electricity_ = power;
		record_info.elevation_ = nhigh;
		record_info.time_stamp = TCTime::Now();

		std::string str_csv = record_info.putcsv();
    
    TCString sLog="RecordTrackerLocInfo Record Cur IMEI=["+sImei +"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="RecordTrackerLocInfo SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="RecordTrackerLocInfo 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="RecordTrackerLocInfo 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}

//==========================================================================
// 函数 : TCDataCollectionPadHandler::RecordPadLocInfo
// 用途 : 登记PAD的位置信息
// 原型 : RecordPadLocInfo()
// 参数 : 无
// 返回 : 无
// 说明 : 登记PAD的位置信息
//==========================================================================
bool TCDataCollectionPadHandler::RecordPadLocInfo(TCString sImei, str_Space_Pos_Unit sPosNode,\
	str_Lac_Cell_Node sLBSNode, int nRxLev, double dBLE_X, double dBLE_Y, double dBLE_Z,\
	TCString sBLE_MAC,TCString imsi, TCString phone_no, double elevation){
	try{
		// TODO: 查询终端配置队列区分phone和pad
		redisReply * ry_pad = NULL;

		RecordInfo record_info;
		record_info.clear();
		record_info.vender = vender;
		record_info.unique_id_ = sImei;
		record_info.longitude_ = FloatToStr(sPosNode.dLongitude, 6);
		record_info.latitude_ = FloatToStr(sPosNode.dLatitude, 6);
		record_info.cellid_ = sLBSNode.nCellID;
		record_info.lac_ = sLBSNode.nLac;
		record_info.imsi = imsi;
		record_info.phone_no = phone_no;
		record_info.elevation_ = elevation;
		       
    str_Space_Pos_Unit sCurLBSPosInfo;
    sCurLBSPosInfo.clear();
    
    map<str_Lac_Cell_Node, str_Space_Pos_Unit>::iterator ITR_AdjLacCell_PosUnit;
		ITR_AdjLacCell_PosUnit=g_msCellInfo_PosUnit.find(sLBSNode);
		if(ITR_AdjLacCell_PosUnit!=g_msCellInfo_PosUnit.end()){
			//: 当前的小区信息;
			sCurLBSPosInfo=ITR_AdjLacCell_PosUnit->second;
		}
		record_info.cell_x = FloatToStr(sCurLBSPosInfo.dLongitude,6);
		record_info.cell_y = FloatToStr(sCurLBSPosInfo.dLatitude,6);
		//
		record_info.rscp_ =  nRxLev;
		//record_info.rxlev_ = nRxLev;
		record_info.ble_x = FloatToStr(dBLE_X,3);
		record_info.ble_y = FloatToStr(dBLE_Y,3);
		record_info.ble_z = FloatToStr(dBLE_Z, 3);
		record_info.blemac_ = sBLE_MAC;
		record_info.time_stamp = TCTime::Now();


		// 根据imsi去LOC_POSTERM_INFO获取队列类型
		ry_pad = (redisReply *)redisCommand(redis_context, "HGET LOC_POSTERM_INFO %s", (char *)record_info.imsi);
		if (ry_pad->type == REDIS_REPLY_NIL || ry_pad->type == REDIS_REPLY_ERROR) {
			LOG_WRITE("LOC_POSTERM_INFO中无对应的imsi:[%s]", (char *)record_info.imsi);
			freeReplyObject(ry_pad);
			ry_pad = NULL;
			return false;
		}

		TCString term_type = "";
		if(ry_pad->type == REDIS_REPLY_STRING) {
			TCString str = ry_pad->str;
			TCStringList str_list;
			str_list.CommaText(str, ',');
			if (str_list.GetCount() < 1) {
				freeReplyObject(ry_pad);
				return false;
			}	else {
				term_type = str_list[1];
			}			 
		}
		freeReplyObject(ry_pad);
		ry_pad = NULL;
		
		term_type = term_type.Right(1);
		record_info.vender = term_type;

		// 输出csv
		std::string str_csv = record_info.putcsv();
#ifdef DEBUG_OUT
		TCString sLog = "RecordPadLocInfo Record Cur IMEI=[" + sImei + "]";
		glgMls->AddDataCollectionRunLog(sLog);
		fprintf(stdout, "IMSI:%s, POSTERM_TYPE:%s\n", (char *)record_info.imsi, (char *)term_type.Right(1));
#endif
		// TODO:放入定位队列
		ry_pad = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromDataCollection %s", (char *)str_csv.c_str());
		if (ry_pad->type == REDIS_REPLY_ERROR)
		{
#ifdef DEBUG_OUT
			std::cout << "record_info--csv: " << str_csv << "failed" << std::endl;
#endif
		}
		freeReplyObject(ry_pad);
		ry_pad = NULL;
		// LBS 计算队列
		ry_pad = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromLBSCollection %s", (char *)str_csv.c_str());
		if (ry_pad->type == REDIS_REPLY_ERROR)
		{
			perror("error:LPUSH WuDongDe:PositionData:FromLBSCollection");
		}
		freeReplyObject(ry_pad);
		ry_pad = NULL;

		// 供原始记录入库使用
		ry_pad = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:SrcRecordForOracle %s", (char *)str_csv.c_str());
		freeReplyObject(ry_pad);
		ry_pad = NULL;

		// 终端最新一条记录的数据
		// 1 车载 2 穿戴 3 app 4 蓝牙
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
		str_latest_data += "";
		str_latest_data += ",";
		str_latest_data += term_type;

		ry_pad = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:PostermLastDataHash %s %s",
			(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
		freeReplyObject(ry_pad);
		ry_pad = NULL;

		// --20161026
		// 新增指纹采集队列--imsi+timestamp[20161026142021--yyyymmddhhmmss],device_type
		str_latest_data = (char *)record_info.time_stamp;
		str_latest_data += ",";
		str_latest_data += term_type;

		// 终端状态确认
		ry_pad = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:TermStatusCheckHash %s %s",
			(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
		freeReplyObject(ry_pad);
		ry_pad = NULL;

		return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="RecordPadLocInfo SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="RecordPadLocInfo 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="RecordPadLocInfo 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}


