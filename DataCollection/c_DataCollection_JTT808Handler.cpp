/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 兆益定位设备协议处理类
File Name   : c_DataCollection_JTT808Handler.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#pragma hdrstop

#include "c_DataCollection_JTT808Handler.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_task.h"

#pragma package(smart_init)

extern TCDataCollectionHandler gsDataCollectionHandler;

extern TCDataCollectionConfig *gDataCollectionConfig;


//: 数据库连接
extern otl_connect m_dbConnect;

//: 数据库连接
extern otl_connect m_dbConnect;


extern TCDataCollectionLog  *glgMls; 

extern map<str_Lac_Cell_Node, str_Space_Pos_Unit> g_msCellInfo_PosUnit;

//==========================================================================
// 函数 : TCDataCollectionJTT808Handler::TCDataCollectionJTT808Handler
// 用途 : 构造函数
// 原型 : TCDataCollectionJTT808Handler()
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionJTT808Handler::TCDataCollectionJTT808Handler(){
	redis_context = NULL;
}

//==========================================================================
// 函数 : TCDataCollectionJTT808Handler::~TCDataCollectionJTT808Handler()
// 用途 : 析构函数
// 原型 : ~TCDataCollectionJTT808Handler()
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionJTT808Handler::~TCDataCollectionJTT808Handler()
{
	if (redis_context != NULL)
	{
		redisFree(redis_context);
	}
}

//==========================================================================
// 函数 : TCDataCollectionJTT808Handler::Main_Handler
// 用途 : 主要实现具体的业务交互逻辑
// 原型 : void Main_Handler()
// 参数 :
// 返回 :
// 说明 : 主要实现具体的业务交互逻辑
//==========================================================================
bool TCDataCollectionJTT808Handler::Main_Handler(TCCustomUniSocket  &cusSocket){
	try{
		//TODO:载入imsi等对应关系
		//LoadImsiMore();
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
		
		//printf("接收到 Tracker 数据请求：IP=[%s], Port=[%d], Timeout=[%d]\n", (char*)cusSocket.GetRemoteAddress(), cusSocket.GetRemotePort(), m_nTimeOut);
		
		//: 长连接方式
		// TODO:改用短链接方式20161025--由服务器关闭连接
		while(1){ 
			m_sRecvTime=TCTime::Now();
			Init();
			TCString sLogMsg="Time:Now=["+TCTime::Now()+"],Recv Port=["+IntToStr(cusSocket.GetRemotePort())+"] Data";
                        glgMls->AddDataCollectionRunLog(sLogMsg);
 			RecvRequest(cusSocket);
 			DealRequest(cusSocket);	
 		}
 		cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
 		return true;
  }catch(TCException &e){
    TCString sLogMsg = TCString("At TCDataCollectionJTT808Handler::Main_Handler 处理请求时出错:") + e.GetExceptionMessage();
    glgMls->AddDataCollectionRunLog(sLogMsg);
    cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
    return false;
  }catch(...){
    glgMls->AddDataCollectionRunLog("At TCDataCollectionJTT808Handler::Main_Handler Unknow, 关闭Socket连接");
    cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
    return false;
 	}
}

//==========================================================================
// 函数 : TCDataCollectionJTT808Handler::DealRequest
// 用途 : 处理请求
// 原型 : void DealRequest()
// 参数 : 无
// 返回 : 无
// 说明 :
//==========================================================================
void TCDataCollectionJTT808Handler::DealRequest(TCCustomUniSocket  &cusSocket){
	try{
		//: 按照命令字进行处理；
#ifdef __TEST__
   	printf("============receive Tracker request==============\n");
   	printf("m_nMsgId:=%d, m_nMsgAttr=[%d], m_nMsgBodyLen=[%d], m_sMsisdn=[%s], Time=[%s]\n", \
   	m_nMsgId, m_nMsgAttr, m_nMsgBodyLen, (char*)m_sMsisdn, (char*)TCTime::Now());
#endif
		
		TCString sLog="";

		//======执行服务调用==========================		
		switch(m_nMsgId){
			case 0x0100:   
				//: 终端注册
				sLog = "Recv["+m_sMsisdn+"]的[Client Register]";
				glgMls->AddDataCollectionRunLog(sLog);
				//m_sRecvMsgBody = Mid(m_sPkg_Content,13,m_nMsgBodyLen);
				// TODO:new
				m_sRecvMsgBody = Mid(m_sPkg_Content, 29, m_nMsgBodyLen);
				DoCommand_ClientRegister(cusSocket);
				break;
			case 0x0102:   
				//: 终端鉴权	
				sLog = "Recv["+m_sMsisdn+"]的[Client Auth]";
				glgMls->AddDataCollectionRunLog(sLog);
				//m_sRecvMsgBody = Mid(m_sPkg_Content,13,m_nMsgBodyLen);
				// TODO:--new
				m_sRecvMsgBody = Mid(m_sPkg_Content, 29, m_nMsgBodyLen);
				DoCommand_ClientAuth(cusSocket);
				break;
			case 0x0002:   
	  		//:终端心跳
				sLog = "Recv["+m_sMsisdn+"]的[Heart Break]";
				glgMls->AddDataCollectionRunLog(sLog);
				ServerGeneralAck(cusSocket);
				break;
			case 0x0200:  
	  		// 位置信息汇报
				sLog = "Recv["+m_sMsisdn+"]的[Location Report]";
				glgMls->AddDataCollectionRunLog(sLog);
				// m_sRecvMsgBody = Mid(m_sPkg_Content,13,m_nMsgBodyLen);
				// TODO:--new按新协议进行解析--新增imei/imsi
				m_sRecvMsgBody = Mid(m_sPkg_Content, 29, m_nMsgBodyLen);
				DoCommand_LocInfo(cusSocket);
				break;
			case 0x0900:  
	  		//: 数据上行透传 (BLE部分，暂未处理；)
				sLog = "Recv["+m_sMsisdn+"]的[Data Transparent]";
				glgMls->AddDataCollectionRunLog(sLog);
				//m_sRecvMsgBody = Mid(m_sPkg_Content,13,m_nMsgBodyLen);
				// TODO:--new
				m_sRecvMsgBody = Mid(m_sPkg_Content, 29, m_nMsgBodyLen);
				DoCommand_DataTrans(cusSocket);
	    	break;
	    default:
				//:其它信息发平台通用应答
				sLog = "Recv["+m_sMsisdn+"]的[General Msg]";
				glgMls->AddDataCollectionRunLog(sLog);
				ServerGeneralAck(cusSocket);
		}
  }catch(TCException &e){
  	try{
      TCString sLogMsg = TCString ("系统错误,At DealRequest: ") + e.GetExceptionMessage();
      glgMls->AddDataCollectionRunLog(sLogMsg);
    }catch(TCException &e1){
      TCString sLogMsg = TCString("系统错误: when Catch a Error and SendRespPkg: ") + e1.GetExceptionMessage();
      glgMls->AddDataCollectionRunLog(sLogMsg);
    }catch(...){
      glgMls->AddDataCollectionRunLog("系统发生未知异常");
    }
  }
}


//==========================================================================
// 函数 : TCDataCollectionJTT808Handler::RecvRequest(TCCustomUniSocket  &cusSocket)
// 用途 : JT/T808-2011部颁标准的数据处理方式，接收数据, 按照无阻塞接收
// 原型 : void RecvRequest()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionJTT808Handler::RecvRequest(TCCustomUniSocket  &cusSocket){
	try{
		const int nMaxPkg_Length=1024*1000;
		char sbuff[nMaxPkg_Length];
		memset(sbuff, 0, sizeof(sbuff));

		m_sReq_Command="";
		m_sPkg_Content="";
				
		int nTimeOut=600 * 1000;
		
		//: 开始按照 select 来进行判断
		if(!cusSocket.WaitForData(nTimeOut)){
			//: 超时120秒了，可以直接关闭连接；
			TCString sLog="JTT808 Tracker[" + m_sTacker_Send_IPAddress + "]已经超时120秒未发送数据，断开连接";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}
		
		int nGetPkgLen=0;
		nGetPkgLen=cusSocket.ReceiveBuf(sbuff, nMaxPkg_Length);

		if(nGetPkgLen==0){
			//: 超时120秒了，可以直接关闭连接；
			TCString sLog="JTT808-2011 Tracker[" + m_sTacker_Send_IPAddress + "]已经断开连接";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}
			
		TCString sPkg_Content(sbuff, nGetPkgLen);
		printf("sPkg_Content[1]: %2x sPkg_Content[Length(sPkg_Content)]:%2x\n", sPkg_Content[1], sPkg_Content[Length(sPkg_Content)]);
		//LOG_WRITE("Pkg_content:%s\n", (char *)sPkg_Content);
		
		//: 第一个字节和最后一个字节都是0x7e
		// TODO:
    if(Length(sPkg_Content)<2 || sPkg_Content[1]!=0x7e ||  sPkg_Content[Length(sPkg_Content)]!=0x7e){
    	//: 异常，请检查数据
    	TCString sLogMsg;
			sLogMsg = "";
			sLogMsg += TCString("ERROR: 请求报文长度异常=[")+IntToStr(Length(sPkg_Content))+"]" ;
	  	glgMls->AddDataCollectionRunLog(sLogMsg);
    	printf("Error Packet Format: %s\n", (char*)sLogMsg);
    	cusSocket.Close();
			throw TCException(sLogMsg);
    	//return ;
    }
		//原始报文
		TCString ms_sRow_Pkt = "";
		char sBuff[32] = { 0 };
		for (int nSeq = 0; nSeq<sPkg_Content.GetLength(); nSeq++) {
			memset(sBuff, 0, sizeof(sBuff));
			sprintf(sBuff, "%02x:", sbuff[nSeq]);
			ms_sRow_Pkt += Right(TCString(sBuff), 3);
		}
		printf("JTT808:received original packet: %s\n", (char *)ms_sRow_Pkt);
    sPkg_Content=Mid(sPkg_Content, 2, Length(sPkg_Content)-2);
    
    //:对消息进行reverse 逆转义
		unsigned char cx7e = 0x7e;
		unsigned char cx7d = 0x7d;
		unsigned char cx01 = 0x01;
		unsigned char cx02 = 0x02;
		
		TCString sx7eTransf = TCString(cx7d) + TCString(cx02);
		TCString sx7dTransf = TCString(cx7d) + TCString(cx01);
		
		TCString sMessage = "";
		int nMegLen = Length(sPkg_Content);
		for(int i=1;i<=nMegLen;i++){
			if(sPkg_Content[i] == cx7d){
				if((i+1)<= nMegLen){
					if(sPkg_Content[i+1] == cx02){
						sMessage += cx7e;
						i++;
					}else if(sPkg_Content[i+1] == cx01){
						sMessage += cx7d;
						i++;
					}else{

					}
				}				
			}else{
				sMessage += sPkg_Content[i];
			}
		}

		glgMls->AddDataCollectionRunLog(ms_sRow_Pkt);
    //:消息ID
		m_nMsgId = AscStrToInt(Mid(sMessage,1,2));
		std::cout << "消息ID" << m_nMsgId << std::endl;

		//:消息属性
		m_nMsgAttr = AscStrToInt(Mid(sMessage,3,2));
		std::cout << "消息属性：" << m_nMsgAttr << std::endl;

		//:消息体长度
		m_nMsgBodyLen = (~(63<<9))&(m_nMsgAttr);
		std::cout << "消息体长度：" << m_nMsgBodyLen << std::endl;

		//:手机号，解析前是bcd码--old
		m_sMsisdn = "";
		//m_sMsisdn = IntToStr(AscStrToInt(Mid(sMessage, 5, 1)) >> 4)
		//		+ IntToStr(AscStrToInt(Mid(sMessage,5,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,6,1))>>4)+ IntToStr(AscStrToInt(Mid(sMessage,6,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,7,1))>>4)+ IntToStr(AscStrToInt(Mid(sMessage,7,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,8,1))>>4) + IntToStr(AscStrToInt(Mid(sMessage,8,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,9,1))>>4) + IntToStr(AscStrToInt(Mid(sMessage,9,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,10,1))>>4) + IntToStr(AscStrToInt(Mid(sMessage,10,1))&(~(15<<4)));
		
		// TODO:协议调整--device id
		// 设备imei
		m_device_id = "";
		m_device_id = IntToStr(AscStrToInt(Mid(sMessage, 5, 1))&(~(15 << 4)))\
				+ IntToStr(AscStrToInt(Mid(sMessage, 6, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 6, 1))&(~(15 << 4)))\
				+ IntToStr(AscStrToInt(Mid(sMessage, 7, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 7, 1))&(~(15 << 4)))\
				+ IntToStr(AscStrToInt(Mid(sMessage, 8, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 8, 1))&(~(15 << 4)))\
				+ IntToStr(AscStrToInt(Mid(sMessage, 9, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 9, 1))&(~(15 << 4)))\
				+ IntToStr(AscStrToInt(Mid(sMessage, 10, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 10, 1))&(~(15 << 4)))\
				+ IntToStr(AscStrToInt(Mid(sMessage, 11, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 11, 1))&(~(15 << 4)))\
				+ IntToStr(AscStrToInt(Mid(sMessage, 12, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 12, 1))&(~(15 << 4)));
		// imsi
		m_no_imsi = "";
		m_no_imsi = IntToStr(AscStrToInt(Mid(sMessage, 13, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 14, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 14, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 15, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 15, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 16, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 16, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 17, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 17, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 18, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 18, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 19, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 19, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 20, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 20, 1))&(~(15 << 4)));
		
		// 手机号--new
		m_sMsisdn = IntToStr(AscStrToInt(Mid(sMessage, 21, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 22, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 22, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 23, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 23, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 24, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 24, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 25, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 25, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 26, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 26, 1))&(~(15 << 4)));
		//:流水号
		//m_nMsgSerialNum = AscStrToInt(Mid(sMessage,11,2));

		//TODO:
		m_nMsgSerialNum = AscStrToInt(Mid(sMessage, 27, 2));

		m_sPkg_Content=sMessage;
		
		TCString pkg_time = "";
    //======== 5. 记录日志 =============
    TCString sLogMsg;
		sLogMsg = "";
		sLogMsg += TCString("MsgID:(") + IntToStr(m_nMsgId);
		sLogMsg += TCString(") Attr:") + IntToStr(m_nMsgAttr);
		sLogMsg += TCString(") MsgLen:") + IntToStr(m_nMsgBodyLen);
		sLogMsg += TCString(") MSISDN:") + m_sMsisdn;
		sLogMsg += TCString(") MsgSerialNum:") + IntToStr(m_nMsgSerialNum);
		sLogMsg += TCString(") END\n");
    glgMls->AddDataCollectionRunLog(sLogMsg);
  }catch (TCException &e){
    cusSocket.Close();
    throw e;
  }catch (...){
    cusSocket.Close();
    throw TCException("TCDataCollectionJTT808Handler::RecvRequest() Error:"
                        "Unknown Exception.");
  }
}

//==========================================================================
// 函数 : TCDataCollectionJTT808Handler::SendRespPkg
// 用途 : 发送应答包
// 原型 : void SendRespPkg(TCString sRespCommand, TCString sRespContent)
// 参数 : sRespCommand---返回命令代码   sRespContent ---- 返回内容(前三个字节就是命令码)
// 返回 : 无
// 说明 :
//==========================================================================
void TCDataCollectionJTT808Handler::SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespCommand, TCString sRespContent){
	TCString sMsgHeader = "";
	
	int nMsgId = htons(m_nRespMsgId);
	char *pMsgId = (char*)&nMsgId;
	TCString sMsgId = TCString(pMsgId,2);
	sMsgHeader += sMsgId;

	//:除了包长度，其它属性为0
	unsigned short nMsgAttr = Length(m_sRespMsgBody);
	nMsgAttr = htons(nMsgAttr);
	char *pMsgAttr = (char*)&nMsgAttr;
	TCString sMsgAttr = TCString(pMsgAttr,2);
	sMsgHeader += sMsgAttr;

	//:手机号暂时为0
	char acMsisdn[6];
	memset(acMsisdn,0,sizeof(acMsisdn));
	TCString sMsisdn = TCString(acMsisdn,6);
	sMsgHeader += sMsisdn;

	unsigned short nMsgSerialNum = ++m_nRespMsgSerialNum;
	nMsgSerialNum = htons(nMsgSerialNum);
	char *pMsgSerialNum = (char*)&nMsgSerialNum;
	TCString spMsgSerialNum = TCString(pMsgSerialNum,2);
	sMsgHeader += spMsgSerialNum;

	TCString sMsg = sMsgHeader + m_sRespMsgBody;
	unsigned char cCheckCode = sMsg[1];
	int nMsgLen = Length(sMsg);
	for(int i=2;i<= nMsgLen;i++){
		cCheckCode = cCheckCode ^ (unsigned char)(sMsg[i]);
	}

	sMsg += TCString(cCheckCode);

	TCString sMsgBeforTransf =  sMsg;
	//:进行转义处理
		
	//:对消息进行转义
	unsigned char cx7e = 0x7e;
	unsigned char cx7d = 0x7d;
	unsigned char cx01 = 0x01;
	unsigned char cx02 = 0x02;
	TCString sx7eTransf = TCString(cx7d) + TCString(cx02);
	TCString sx7dTransf = TCString(cx7d) + TCString(cx01);
	TCString sMsgAfterTransf = "";
	int nMegLen = Length(sMsgBeforTransf);
	for(int i=1;i<=nMegLen;i++){
		if(sMsgBeforTransf[i] == cx7e){
			sMsgAfterTransf += sx7eTransf;
		}else if(sMsgBeforTransf[i] == cx7d){
			sMsgAfterTransf += sx7dTransf;
		}else{
			sMsgAfterTransf += sMsgBeforTransf[i];
		}
	}

	//:增加消息标志
	TCString sSendMsg = "";
	sSendMsg = TCString(cx7e)+sMsgAfterTransf+TCString(cx7e);
  cusSocket.SendBuf((char *)sSendMsg, Length(sSendMsg));

	printf("SendMsg:");
	for (int i = 1; i <= Length(sSendMsg); i++) {
		printf("0x%x", sSendMsg[i]);
	}
	printf("\n");
	m_sSendTime=TCTime::Now();
	//LOG_WRITE("RecvTime=[%s], SendTime=[%s], SendMsg[%s]", (char*)m_sRecvTime, (char*)m_sSendTime,
	//	(char *)sSendMsg);
#ifdef __TEST__		
	printf("RecvTime=[%s], SendTime=[%s]\n", (char*)m_sRecvTime, (char*)m_sSendTime);
#endif

}

//==========================================================================
// 函数 : void TCDataCollectionJTT808Handler::DoCommand_LocInfo
// 用途 : 可穿戴式终端上报位置信息
// 原型 : void DoCommand_LocInfo()
// 参数 : 无
// 返回 : 无
// 说明 :  
//==========================================================================
void TCDataCollectionJTT808Handler::DoCommand_LocInfo(TCCustomUniSocket  &cusSocket){
		TCString sLog ="";
		int nLen = Length(m_sRecvMsgBody);
		sLog="Client Location Report msg body: Length=["+IntToStr(Length(m_sRecvMsgBody))+"]\n";
		for(int i=1;i<=nLen;i++)
		{
			char sBuff[8];
			memset(sBuff, 0, sizeof(sBuff));
			sprintf(sBuff, "%02x:", m_sRecvMsgBody[i]);
			TCString sTempValue=TCString(sBuff);
			sLog+="["+Right(sTempValue, 2)+"]:";
		}
		sLog+="\n";

		printf("m_sRecvMsgBody:%s\n", (char*)sLog);
		TCString sslog = "RecvMsgBody:" + sLog;		
		glgMls->AddDataCollectionRunLog(sslog);
		
		unsigned int nStaus = AscStrToInt(Mid(m_sRecvMsgBody,5,4));

		int nLocationStaus = (nStaus>>1)&1;

		TCString sLatitudeDB = "";
		TCString sLongitudeDB = "";
		int nFlag = 0;
		
		if(nLocationStaus == NO_LOCATION){
			nFlag = 1;
			sLog = "JTT808-2011 Terminal:["+m_sMsisdn+"] Location Except";
			glgMls->AddDataCollectionRunLog(sLog);
		}else{
			nFlag = 0;
			sLog = "JTT808-2011 Terminal:["+m_sMsisdn+"] Location Correcte";
			glgMls->AddDataCollectionRunLog(sLog);

			//:纬度
			TCString sLatitude = Mid(m_sRecvMsgBody,9,4);
			std::string str_t = (char *)sLatitude;
			//printf("//:纬度:%s\n", str_t.c_str());
			//for (size_t i = 0; i <4; i++)
			//{
			//	printf("%02x:", str_t[i]);
			//}
			//printf("\n");
			unsigned int nLatitude = AscStrToInt(sLatitude);
			//printf("//:纬度int:%d\n", nLatitude);
			float fLatitude = nLatitude/1000000.0;
			sLatitudeDB = FloatToStr(fLatitude,6);

			//:经度
			TCString sLongitude = Mid(m_sRecvMsgBody,13,4);
			unsigned int nLongitude = AscStrToInt(sLongitude);
			float fLongitude = nLongitude/1000000.0;
			sLongitudeDB = FloatToStr(fLongitude,6);			
		}

		//:添加车辆速度的数据的入库
		TCString sSpeed = Mid(m_sRecvMsgBody,19,2);
		unsigned int nSpeed = AscStrToInt(sSpeed);
		//printf("nSpeed=%u\n",nSpeed);
		float fSpeed = nSpeed/10.0;
		TCString sSpeedDB = FloatToStr(fSpeed,1);
		// 车载新增: 陀螺仪X倾斜角,陀螺仪Y倾斜角,陀螺仪Z倾斜角,加速度X值,加速度X值,加速度Z值--有符号
		/*
		加速(g) = 带符号的原始数据 * 8 / 32768
		陀螺仪(dps) = 带符号的原始数据 * 125 / 32768
		*/
		TCString tmp_pkg = Mid(m_sRecvMsgBody, 23, 2);
		short gyro_x = (short)AscStrToInt(tmp_pkg);
		double gyro_x_d = gyro_x * 125.0 / 32768;
		tmp_pkg = Mid(m_sRecvMsgBody, 25, 2);
		short gyro_y = (short)AscStrToInt(tmp_pkg);
		double gyro_y_d = gyro_y * 125.0 / 32768;
		tmp_pkg = Mid(m_sRecvMsgBody, 27, 2);
		short gyro_z = (short)AscStrToInt(tmp_pkg);
		double gyro_z_d = gyro_z * 125.0 / 32768;

		//printf("gyro:[%hd,%hd,%hd]\n", gyro_x, gyro_y, gyro_z);
		//printf("gyro:[%f,%f,%f]\n", gyro_x_d, gyro_y_d, gyro_z_d);

		tmp_pkg = Mid(m_sRecvMsgBody, 29, 2);
		short accelerate_x = (short)AscStrToInt(tmp_pkg);
		double accelerate_x_d = accelerate_x * 8.0 / 32768;
		tmp_pkg = Mid(m_sRecvMsgBody, 31, 2);
		short accelerate_y = (short)AscStrToInt(tmp_pkg);
		double accelerate_y_d = accelerate_y * 8.0 / 32768;
		tmp_pkg = Mid(m_sRecvMsgBody, 33, 2);
		short accelerate_z = (short)AscStrToInt(tmp_pkg);
		double accelerate_z_d = accelerate_z * 8.0 / 32768;
		
		//printf("accelerate:[%hd,%hd,%hd]\n", accelerate_x, accelerate_y, accelerate_z);
		//printf("accelerate_d:[%f,%f,%f]\n", accelerate_x_d, accelerate_y_d, accelerate_z_d);
		// 加速度=带符号的原始数据*8/32768
		// 陀螺仪=带符号的原始数据*125/32768

		str_Space_Pos_Unit sPosNode;
		sPosNode.clear();
		sPosNode.dLongitude=StrToFloat(sLongitudeDB);
		sPosNode.dLatitude=StrToFloat(sLatitudeDB);
		
		//: 信息透传基站和小区信息;
		TCString sAdditonalLocInfo_Str=Right(m_sRecvMsgBody, 8);
				
		sLog = "Additional Data Lac-Cell Msg";
		glgMls->AddDataCollectionRunLog(sLog);

		int nCurLen=sAdditonalLocInfo_Str[2];	//: 长度域: 应该等于6;
		
		TCString sLac_Field=Mid(sAdditonalLocInfo_Str, 3, 2);
		std::string lac_str = (char *)sLac_Field;
		//for (size_t i = 0; i < 2; i++)
		//{
		//	printf("%02x:", lac_str[i]);
		//}
		int nLAC=AscStrToInt(sLac_Field);
		
		TCString sCellID_Field=Mid(sAdditonalLocInfo_Str, 5, 2);
		int nCellID=AscStrToInt(sCellID_Field);
		
		int nRscp= AscStrToInt(sAdditonalLocInfo_Str[7]);	//: RSCP接收信号码功率
		int nRxLev= AscStrToInt(sAdditonalLocInfo_Str[8]);	//: RSSI接收信号强度
			
		ServerGeneralAck(cusSocket);
		
		str_Lac_Cell_Node sLBSNode;
		sLBSNode.clear();
		sLBSNode.nLac=nLAC;
		sLBSNode.nCellID=nCellID;
		
		//double dHigh=0.00;
		// TODO:
		TCString sHigh = Mid(m_sRecvMsgBody, 17, 2);
		double dHigh = (double)AscStrToInt(sHigh);

		double dPower=0.00;	
		TCString sHigh = Mid(m_sRecvMsgBody, 17, 2);
		int direction = AscStrToInt(Mid(m_sRecvMsgBody, 21, 1));	//: 方向
		
		//printf("sMsisdn=[%s], nLAC-CI=[%d-%d],nRscp=[%d],nRxLev=[%d], GPS=[%.6f, %.6f], dHigh=[%.2f], dPower=[%.3f]\n", \
		(char*)m_sMsisdn, sLBSNode.nLac, sLBSNode.nCellID, nRscp, nRxLev, sPosNode.dLongitude, sPosNode.dLatitude, dHigh, dPower);
		
		sLog = "DoCommand_LocInfo: sMsisdn=[" + m_sMsisdn + "], nLAC-CI=[" + IntToStr(nLAC) + "-" + IntToStr(nCellID)
			+ "],nRscp=[" + IntToStr(nRscp) + "],nRxLev=[" + IntToStr(nRxLev) + "], GPS=["
			+ FloatToStr(sPosNode.dLongitude, 6) + "," + FloatToStr(sPosNode.dLatitude, 6)
			+ "], dHigh=[" + FloatToStr(dHigh, 3) + "], dPower=[" + FloatToStr(dPower, 3)
			+ "],speed=[" + FloatToStr(fSpeed) + "],IMSI=[" + m_no_imsi
			+ "],IMEI=[" + m_device_id + "]"
			+ "[gyro(x,y,z)]=[" + IntToStr(gyro_x) + "," + IntToStr(gyro_y) + "," + IntToStr(gyro_z) + "]"
			+ "[accelerate(x,y,z)=[" + IntToStr(accelerate_x) + "," 
			+ IntToStr(accelerate_y) + "," + IntToStr(accelerate_z) + "]\n";
		glgMls->AddDataCollectionRunLog(sLog);
				
		RecordTrackerLocInfo(m_sMsisdn, sPosNode, sLBSNode, nRscp, nRxLev, fSpeed, dHigh, direction);
				
		return ;	
}



//==========================================================================
// 函数 : void TCDataCollectionJTT808Handler::DoCommand_DataTrans
// 用途 : 数据上行透传
// 原型 : void DoCommand_DataTrans()
// 参数 : 无
// 返回 : 无
// 说明 :  
//==========================================================================
void TCDataCollectionJTT808Handler::DoCommand_DataTrans(TCCustomUniSocket  &cusSocket){
	try{	
		printf("\n iBeaconReport msg body: \n");
		int nLen = Length(m_sRecvMsgBody);
		for(int i=1;i<=nLen;i++){
			printf("[%02x],",(unsigned char)m_sRecvMsgBody[i]);
		}
		printf("\n");
		if(Length(m_sRecvMsgBody)<8){
			//:不完整的包，丢弃处理
			ServerGeneralAck(cusSocket);	
			return ;
		}
			
		unsigned char cTag = (unsigned char)m_sRecvMsgBody[1];
		if(cTag != 0xf1){
			//:不是透传包的ibeacon,丢弃处理
			ServerGeneralAck(cusSocket);	
			return ;
		}
		
		TCString sLog = "Data Transparent Msg";
		glgMls->AddDataCollectionRunLog(sLog);

		int nCurLen=m_sRecvMsgBody[2];	//: 长度域: 应该等于6;
		
		TCString sLac_Field=Mid(m_sRecvMsgBody, 3, 2);
		int nLAC=AscStrToInt(sLac_Field);
		
		TCString sCellID_Field=Mid(m_sRecvMsgBody, 5, 2);
		int nCellID=AscStrToInt(sCellID_Field);
		
		int nRscp=m_sRecvMsgBody[7];	//: RSCP接收信号码功率
		int nRxLev=m_sRecvMsgBody[8];	//: RSSI接收信号强度
		
		ServerGeneralAck(cusSocket);

		str_Space_Pos_Unit sPosNode;
		sPosNode.clear();
		
		str_Lac_Cell_Node sLBSNode;
		sLBSNode.clear();
		sLBSNode.nLac=nLAC;
		sLBSNode.nCellID=nCellID;
		
		double dHigh=0.00;
		double dPower=0.00;
		
		printf("sMsisdn=[%s], nLAC-CI=[%d-%d],nRscp=[%d],nRxLev=[%d], GPS=[%.6f, %.6f], dHigh=[%.2f], dPower=[%.3f]\n", \
		(char*)m_sMsisdn, sLBSNode.nLac, sLBSNode.nCellID, nRscp, nRxLev, sPosNode.dLongitude, sPosNode.dLatitude, dHigh, dPower);
		
		sLog="DoCommand_LBSLocInfo: sMsisdn=["+m_sMsisdn+"], nLAC-CI=["+IntToStr(nLAC)+"-"+IntToStr(nCellID)+"],nRscp=["+IntToStr(nRscp)+"],nRxLev=["+IntToStr(nRxLev)+"], GPS=["+FloatToStr(sPosNode.dLongitude,6)+","+FloatToStr(sPosNode.dLatitude,6)+"], dHigh=["+FloatToStr(dHigh,6)+"], dPower=["+FloatToStr(dPower, 6)+"]\n";
		glgMls->AddDataCollectionRunLog(sLog);
		
		RecordTrackerLocInfo(m_sMsisdn, sPosNode, sLBSNode, nRscp, nRxLev);
			
		return;
	}catch(otl_exception& p){ // intercept OTL exceptions
	  cerr<<p.msg<<endl; // print out error message
	  cerr<<p.stm_text<<endl; // print out SQL that caused the error
	  cerr<<p.var_info<<endl; // print out the variable that caused the error
	  TCString sLog="InsertUuExdMR2DB SQL执行异常\n\tmsg=["+TCString((char*)p.msg)+"]\n\tvar_info=["+TCString((char*)p.var_info)+"]";
	  sLog+=(char*)p.msg;
		glgMls->AddDataCollectionRunLog(sLog);	
	  return ;		
	}catch(TCException &Excep){    
    TCString sLog = "ClientLocationReport Fail:"+ Excep.GetExceptionMessage();
    glgMls->AddDataCollectionRunLog(sLog);      
    printf("[%s]\n",(char*)sLog);
    return ;
  }catch(...){    
    TCString sLog = "ClientLocationReport Fail: UnKnown Error\n";
    glgMls->AddDataCollectionRunLog(sLog);      
    printf("[%s]\n",(char*)sLog);
    return ;
  }
}



//==========================================================================
// 函数 : void TCDataCollectionJTT808Handler::DoCommand_ClientRegister
// 用途 : 终端注册处理
// 原型 : void DoCommand_ClientRegister()
// 参数 : 无
// 返回 : 无
// 说明 :  
//==========================================================================
void TCDataCollectionJTT808Handler::DoCommand_ClientRegister(TCCustomUniSocket  &cusSocket){
		//:收到的注册消息体暂时不处理

		//:开始构建应答包
		TCString sSendMsgBody = "";
		int nMsgSerialNum = htons(m_nMsgSerialNum);
		char *pMsgSerialNum = (char*)&nMsgSerialNum;
		TCString sMsgSerialNum = TCString(pMsgSerialNum,2);
		sSendMsgBody += sMsgSerialNum;

		unsigned char cResult = 0x00;
		sSendMsgBody += TCString(cResult);

		//:鉴权码暂时设为abcdef
		TCString sAuthCode = "abcdef";	
		sSendMsgBody += sAuthCode;

		m_nRespMsgId = 0x8100;
		m_sRespMsgBody=sSendMsgBody;
		
		SendRespPkg(cusSocket, m_sReq_Command, m_sRespMsgBody);
		return ;	
}


//==========================================================================
// 函数 : void TCDataCollectionJTT808Handler::DoCommand_ClientAuth
// 用途 : 终端鉴权
// 原型 : void DoCommand_ClientAuth()
// 参数 : 无
// 返回 : 无
// 说明 :  
//==========================================================================
void TCDataCollectionJTT808Handler::DoCommand_ClientAuth(TCCustomUniSocket  &cusSocket){
		//:收到的鉴权码暂不处理
		ServerGeneralAck(cusSocket);
		return ;	
}

//==========================================================================
// 函数 : void TCDataCollectionJTT808Handler::ServerGeneralAck
// 用途 : 通用应答
// 原型 : void ServerGeneralAck()
// 参数 : 无
// 返回 : 无
// 说明 :  
//==========================================================================
void TCDataCollectionJTT808Handler::ServerGeneralAck(TCCustomUniSocket  &cusSocket){
	//:收到的鉴权码暂不处理	
	TCString sMsgBody = "";	
	int nMsgSerialNum = htons(m_nMsgSerialNum);
	char *pMsgSerialNum = (char*)&nMsgSerialNum;
	TCString sMsgSerialNum = TCString(pMsgSerialNum,2);
	sMsgBody += sMsgSerialNum;

	int nMsgId = htons(m_nMsgId);
	char *pMsgId = (char*)&nMsgId;
	TCString sMsgId = TCString(pMsgId,2);
	sMsgBody += sMsgId;

	unsigned char cResult = 0x00;
	sMsgBody += TCString(cResult);
	
	//: 响应报文类别
	m_nRespMsgId=0x8001;
	m_sRespMsgBody=sMsgBody;
	
	SendRespPkg(cusSocket, m_sReq_Command, m_sRespMsgBody);
	return ;	
}

//==========================================================================
// 函数 : void TCDataCollectionJTT808Handler::Init
// 用途 : 初始化
// 原型 : void Init()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionJTT808Handler::Init(){
	m_sPkg_Content="";
	m_sReq_Command="";
	m_sRespCommand="";
	m_sRespPkgContent="";
	return;
}


//==========================================================================
// 函数 : TCDataCollectionJTT808Handler::RecordTrackerLocInfo
// 用途 : 用户位置信息登记
// 原型 : RecordTrackerLocInfo()
// 参数 : 无
// 返回 : 无
// 说明 : 用户位置信息登记
//==========================================================================
bool TCDataCollectionJTT808Handler::RecordTrackerLocInfo(TCString phone_no, str_Space_Pos_Unit sPosNode\
	, str_Lac_Cell_Node sLBSNode\
	, int nRscp, int nRxLev, double nspeed, double high, int direction){
	try{  
		RecordInfo record_info;
		record_info.clear();
		
		record_info.unique_id_ = m_device_id;
		/*record_info.imsi = "460" + phone_no;*/
		// TODO:
		record_info.imsi = m_no_imsi;
		// phone
		record_info.phone_no = phone_no;
		// 厂商
		//record_info.vender = vender;
		record_info.longitude_ = FloatToStr(sPosNode.dLongitude,6);
		record_info.latitude_ = FloatToStr(sPosNode.dLatitude,6);
		record_info.lac_ = sLBSNode.nLac;
		record_info.cellid_ = sLBSNode.nCellID;
		record_info.direction_ = direction;
    
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
		record_info.speed = nspeed;
		record_info.elevation_ = high;
		record_info.time_stamp = TCTime::Now();
		std::string str_csv = record_info.putcsv();

		redisReply * ry_jtt = NULL;
		ry_jtt = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromDataCollection %s", (char *)str_csv.c_str());
		if (ry_jtt->type == REDIS_REPLY_ERROR)
		{
			std::cout << "record_info--csv: " << str_csv << "failed" << std::endl;
		}
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;

		// 根据imsi去LOC_POSTERM_INFO获取队列类型
		ry_jtt = (redisReply *)redisCommand(redis_context, "HGET LOC_POSTERM_INFO %s", (char *)record_info.imsi);
		if (ry_jtt->type == REDIS_REPLY_NIL || ry_jtt->type == REDIS_REPLY_ERROR) {
			LOG_WRITE("LOC_POSTERM_INFO中无对应的imsi:[%s]", (char *)record_info.imsi);
			freeReplyObject(ry_jtt);
			ry_jtt = NULL;
			return false;
		}

		TCString term_type = "";
		if (ry_jtt->type == REDIS_REPLY_STRING) {
			TCString str = ry_jtt->str;
			TCStringList str_list;
			str_list.CommaText(str, ',');
			if (str_list.GetCount() < 1) {
				freeReplyObject(ry_jtt);
				return false;
			}
			else {
				term_type = str_list[1];
			}
		}
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;

		term_type = AllTrim(term_type);
		record_info.vender = term_type;
		// 
		// LBS 计算队列
		ry_jtt = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromLBSCollection %s", (char *)str_csv.c_str());
		if (ry_jtt->type == REDIS_REPLY_ERROR)
		{
			perror("error:LPUSH WuDongDe:PositionData:FromLBSCollection");
		}
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;

		// 供原始记录入库使用
		ry_jtt = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:SrcRecordForOracle %s", (char *)str_csv.c_str());
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;

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
		str_latest_data += "";
		str_latest_data += ",";
		str_latest_data += FloatToStr(record_info.speed);
		str_latest_data += ",";
		str_latest_data += term_type;

		ry_jtt = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:PostermLastDataHash %s %s",
			(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;
		//
		// --20161026
		// 新增指纹采集队列--imsi+timestamp[20161026142021--yyyymmddhhmmss],device_type
		str_latest_data = (char *)record_info.time_stamp;
		str_latest_data += ",";
		str_latest_data += term_type;

		// 终端状态确认
		ry_jtt = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:TermStatusCheckHash %s %s",
			(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;

    TCString sLog="RecordTrackerLocInfo Record Cur phone_no=["+phone_no +"]";
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



