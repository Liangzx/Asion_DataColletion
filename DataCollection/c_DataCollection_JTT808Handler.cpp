/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : ���涨λ�豸Э�鴦����
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


//: ���ݿ�����
extern otl_connect m_dbConnect;

//: ���ݿ�����
extern otl_connect m_dbConnect;


extern TCDataCollectionLog  *glgMls; 

extern map<str_Lac_Cell_Node, str_Space_Pos_Unit> g_msCellInfo_PosUnit;

//==========================================================================
// ���� : TCDataCollectionJTT808Handler::TCDataCollectionJTT808Handler
// ��; : ���캯��
// ԭ�� : TCDataCollectionJTT808Handler()
// ���� :
// ���� :
// ˵�� :
//==========================================================================
TCDataCollectionJTT808Handler::TCDataCollectionJTT808Handler(){
	redis_context = NULL;
}

//==========================================================================
// ���� : TCDataCollectionJTT808Handler::~TCDataCollectionJTT808Handler()
// ��; : ��������
// ԭ�� : ~TCDataCollectionJTT808Handler()
// ���� :
// ���� :
// ˵�� :
//==========================================================================
TCDataCollectionJTT808Handler::~TCDataCollectionJTT808Handler()
{
	if (redis_context != NULL)
	{
		redisFree(redis_context);
	}
}

//==========================================================================
// ���� : TCDataCollectionJTT808Handler::Main_Handler
// ��; : ��Ҫʵ�־����ҵ�񽻻��߼�
// ԭ�� : void Main_Handler()
// ���� :
// ���� :
// ˵�� : ��Ҫʵ�־����ҵ�񽻻��߼�
//==========================================================================
bool TCDataCollectionJTT808Handler::Main_Handler(TCCustomUniSocket  &cusSocket){
	try{
		//TODO:����imsi�ȶ�Ӧ��ϵ
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
		//: Զ�̷��͵ĵ�ַ
		m_sTacker_Send_IPAddress=cusSocket.GetRemoteAddress();
		m_nTimeOut=60;
		
		//printf("���յ� Tracker ��������IP=[%s], Port=[%d], Timeout=[%d]\n", (char*)cusSocket.GetRemoteAddress(), cusSocket.GetRemotePort(), m_nTimeOut);
		
		//: �����ӷ�ʽ
		// TODO:���ö����ӷ�ʽ20161025--�ɷ������ر�����
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
    TCString sLogMsg = TCString("At TCDataCollectionJTT808Handler::Main_Handler ��������ʱ����:") + e.GetExceptionMessage();
    glgMls->AddDataCollectionRunLog(sLogMsg);
    cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
    return false;
  }catch(...){
    glgMls->AddDataCollectionRunLog("At TCDataCollectionJTT808Handler::Main_Handler Unknow, �ر�Socket����");
    cusSocket.Close();
		redisFree(redis_context);
		redis_context = NULL;
    return false;
 	}
}

//==========================================================================
// ���� : TCDataCollectionJTT808Handler::DealRequest
// ��; : ��������
// ԭ�� : void DealRequest()
// ���� : ��
// ���� : ��
// ˵�� :
//==========================================================================
void TCDataCollectionJTT808Handler::DealRequest(TCCustomUniSocket  &cusSocket){
	try{
		//: ���������ֽ��д���
#ifdef __TEST__
   	printf("============receive Tracker request==============\n");
   	printf("m_nMsgId:=%d, m_nMsgAttr=[%d], m_nMsgBodyLen=[%d], m_sMsisdn=[%s], Time=[%s]\n", \
   	m_nMsgId, m_nMsgAttr, m_nMsgBodyLen, (char*)m_sMsisdn, (char*)TCTime::Now());
#endif
		
		TCString sLog="";

		//======ִ�з������==========================		
		switch(m_nMsgId){
			case 0x0100:   
				//: �ն�ע��
				sLog = "Recv["+m_sMsisdn+"]��[Client Register]";
				glgMls->AddDataCollectionRunLog(sLog);
				//m_sRecvMsgBody = Mid(m_sPkg_Content,13,m_nMsgBodyLen);
				// TODO:new
				m_sRecvMsgBody = Mid(m_sPkg_Content, 29, m_nMsgBodyLen);
				DoCommand_ClientRegister(cusSocket);
				break;
			case 0x0102:   
				//: �ն˼�Ȩ	
				sLog = "Recv["+m_sMsisdn+"]��[Client Auth]";
				glgMls->AddDataCollectionRunLog(sLog);
				//m_sRecvMsgBody = Mid(m_sPkg_Content,13,m_nMsgBodyLen);
				// TODO:--new
				m_sRecvMsgBody = Mid(m_sPkg_Content, 29, m_nMsgBodyLen);
				DoCommand_ClientAuth(cusSocket);
				break;
			case 0x0002:   
	  		//:�ն�����
				sLog = "Recv["+m_sMsisdn+"]��[Heart Break]";
				glgMls->AddDataCollectionRunLog(sLog);
				ServerGeneralAck(cusSocket);
				break;
			case 0x0200:  
	  		// λ����Ϣ�㱨
				sLog = "Recv["+m_sMsisdn+"]��[Location Report]";
				glgMls->AddDataCollectionRunLog(sLog);
				// m_sRecvMsgBody = Mid(m_sPkg_Content,13,m_nMsgBodyLen);
				// TODO:--new����Э����н���--����imei/imsi
				m_sRecvMsgBody = Mid(m_sPkg_Content, 29, m_nMsgBodyLen);
				DoCommand_LocInfo(cusSocket);
				break;
			case 0x0900:  
	  		//: ��������͸�� (BLE���֣���δ����)
				sLog = "Recv["+m_sMsisdn+"]��[Data Transparent]";
				glgMls->AddDataCollectionRunLog(sLog);
				//m_sRecvMsgBody = Mid(m_sPkg_Content,13,m_nMsgBodyLen);
				// TODO:--new
				m_sRecvMsgBody = Mid(m_sPkg_Content, 29, m_nMsgBodyLen);
				DoCommand_DataTrans(cusSocket);
	    	break;
	    default:
				//:������Ϣ��ƽ̨ͨ��Ӧ��
				sLog = "Recv["+m_sMsisdn+"]��[General Msg]";
				glgMls->AddDataCollectionRunLog(sLog);
				ServerGeneralAck(cusSocket);
		}
  }catch(TCException &e){
  	try{
      TCString sLogMsg = TCString ("ϵͳ����,At DealRequest: ") + e.GetExceptionMessage();
      glgMls->AddDataCollectionRunLog(sLogMsg);
    }catch(TCException &e1){
      TCString sLogMsg = TCString("ϵͳ����: when Catch a Error and SendRespPkg: ") + e1.GetExceptionMessage();
      glgMls->AddDataCollectionRunLog(sLogMsg);
    }catch(...){
      glgMls->AddDataCollectionRunLog("ϵͳ����δ֪�쳣");
    }
  }
}


//==========================================================================
// ���� : TCDataCollectionJTT808Handler::RecvRequest(TCCustomUniSocket  &cusSocket)
// ��; : JT/T808-2011�����׼�����ݴ���ʽ����������, ��������������
// ԭ�� : void RecvRequest()
// ���� : ��
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionJTT808Handler::RecvRequest(TCCustomUniSocket  &cusSocket){
	try{
		const int nMaxPkg_Length=1024*1000;
		char sbuff[nMaxPkg_Length];
		memset(sbuff, 0, sizeof(sbuff));

		m_sReq_Command="";
		m_sPkg_Content="";
				
		int nTimeOut=600 * 1000;
		
		//: ��ʼ���� select �������ж�
		if(!cusSocket.WaitForData(nTimeOut)){
			//: ��ʱ120���ˣ�����ֱ�ӹر����ӣ�
			TCString sLog="JTT808 Tracker[" + m_sTacker_Send_IPAddress + "]�Ѿ���ʱ120��δ�������ݣ��Ͽ�����";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}
		
		int nGetPkgLen=0;
		nGetPkgLen=cusSocket.ReceiveBuf(sbuff, nMaxPkg_Length);

		if(nGetPkgLen==0){
			//: ��ʱ120���ˣ�����ֱ�ӹر����ӣ�
			TCString sLog="JTT808-2011 Tracker[" + m_sTacker_Send_IPAddress + "]�Ѿ��Ͽ�����";
			printf("%s\n", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			throw TCException(sLog);
		}
			
		TCString sPkg_Content(sbuff, nGetPkgLen);
		printf("sPkg_Content[1]: %2x sPkg_Content[Length(sPkg_Content)]:%2x\n", sPkg_Content[1], sPkg_Content[Length(sPkg_Content)]);
		//LOG_WRITE("Pkg_content:%s\n", (char *)sPkg_Content);
		
		//: ��һ���ֽں����һ���ֽڶ���0x7e
		// TODO:
    if(Length(sPkg_Content)<2 || sPkg_Content[1]!=0x7e ||  sPkg_Content[Length(sPkg_Content)]!=0x7e){
    	//: �쳣����������
    	TCString sLogMsg;
			sLogMsg = "";
			sLogMsg += TCString("ERROR: �����ĳ����쳣=[")+IntToStr(Length(sPkg_Content))+"]" ;
	  	glgMls->AddDataCollectionRunLog(sLogMsg);
    	printf("Error Packet Format: %s\n", (char*)sLogMsg);
    	cusSocket.Close();
			throw TCException(sLogMsg);
    	//return ;
    }
		//ԭʼ����
		TCString ms_sRow_Pkt = "";
		char sBuff[32] = { 0 };
		for (int nSeq = 0; nSeq<sPkg_Content.GetLength(); nSeq++) {
			memset(sBuff, 0, sizeof(sBuff));
			sprintf(sBuff, "%02x:", sbuff[nSeq]);
			ms_sRow_Pkt += Right(TCString(sBuff), 3);
		}
		printf("JTT808:received original packet: %s\n", (char *)ms_sRow_Pkt);
    sPkg_Content=Mid(sPkg_Content, 2, Length(sPkg_Content)-2);
    
    //:����Ϣ����reverse ��ת��
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
    //:��ϢID
		m_nMsgId = AscStrToInt(Mid(sMessage,1,2));
		std::cout << "��ϢID" << m_nMsgId << std::endl;

		//:��Ϣ����
		m_nMsgAttr = AscStrToInt(Mid(sMessage,3,2));
		std::cout << "��Ϣ���ԣ�" << m_nMsgAttr << std::endl;

		//:��Ϣ�峤��
		m_nMsgBodyLen = (~(63<<9))&(m_nMsgAttr);
		std::cout << "��Ϣ�峤�ȣ�" << m_nMsgBodyLen << std::endl;

		//:�ֻ��ţ�����ǰ��bcd��--old
		m_sMsisdn = "";
		//m_sMsisdn = IntToStr(AscStrToInt(Mid(sMessage, 5, 1)) >> 4)
		//		+ IntToStr(AscStrToInt(Mid(sMessage,5,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,6,1))>>4)+ IntToStr(AscStrToInt(Mid(sMessage,6,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,7,1))>>4)+ IntToStr(AscStrToInt(Mid(sMessage,7,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,8,1))>>4) + IntToStr(AscStrToInt(Mid(sMessage,8,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,9,1))>>4) + IntToStr(AscStrToInt(Mid(sMessage,9,1))&(~(15<<4)))\
		//		+ IntToStr(AscStrToInt(Mid(sMessage,10,1))>>4) + IntToStr(AscStrToInt(Mid(sMessage,10,1))&(~(15<<4)));
		
		// TODO:Э�����--device id
		// �豸imei
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
		
		// �ֻ���--new
		m_sMsisdn = IntToStr(AscStrToInt(Mid(sMessage, 21, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 22, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 22, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 23, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 23, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 24, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 24, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 25, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 25, 1))&(~(15 << 4)))\
			+ IntToStr(AscStrToInt(Mid(sMessage, 26, 1)) >> 4) + IntToStr(AscStrToInt(Mid(sMessage, 26, 1))&(~(15 << 4)));
		//:��ˮ��
		//m_nMsgSerialNum = AscStrToInt(Mid(sMessage,11,2));

		//TODO:
		m_nMsgSerialNum = AscStrToInt(Mid(sMessage, 27, 2));

		m_sPkg_Content=sMessage;
		
		TCString pkg_time = "";
    //======== 5. ��¼��־ =============
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
// ���� : TCDataCollectionJTT808Handler::SendRespPkg
// ��; : ����Ӧ���
// ԭ�� : void SendRespPkg(TCString sRespCommand, TCString sRespContent)
// ���� : sRespCommand---�����������   sRespContent ---- ��������(ǰ�����ֽھ���������)
// ���� : ��
// ˵�� :
//==========================================================================
void TCDataCollectionJTT808Handler::SendRespPkg(TCCustomUniSocket  &cusSocket, TCString sRespCommand, TCString sRespContent){
	TCString sMsgHeader = "";
	
	int nMsgId = htons(m_nRespMsgId);
	char *pMsgId = (char*)&nMsgId;
	TCString sMsgId = TCString(pMsgId,2);
	sMsgHeader += sMsgId;

	//:���˰����ȣ���������Ϊ0
	unsigned short nMsgAttr = Length(m_sRespMsgBody);
	nMsgAttr = htons(nMsgAttr);
	char *pMsgAttr = (char*)&nMsgAttr;
	TCString sMsgAttr = TCString(pMsgAttr,2);
	sMsgHeader += sMsgAttr;

	//:�ֻ�����ʱΪ0
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
	//:����ת�崦��
		
	//:����Ϣ����ת��
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

	//:������Ϣ��־
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
// ���� : void TCDataCollectionJTT808Handler::DoCommand_LocInfo
// ��; : �ɴ���ʽ�ն��ϱ�λ����Ϣ
// ԭ�� : void DoCommand_LocInfo()
// ���� : ��
// ���� : ��
// ˵�� :  
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

			//:γ��
			TCString sLatitude = Mid(m_sRecvMsgBody,9,4);
			std::string str_t = (char *)sLatitude;
			//printf("//:γ��:%s\n", str_t.c_str());
			//for (size_t i = 0; i <4; i++)
			//{
			//	printf("%02x:", str_t[i]);
			//}
			//printf("\n");
			unsigned int nLatitude = AscStrToInt(sLatitude);
			//printf("//:γ��int:%d\n", nLatitude);
			float fLatitude = nLatitude/1000000.0;
			sLatitudeDB = FloatToStr(fLatitude,6);

			//:����
			TCString sLongitude = Mid(m_sRecvMsgBody,13,4);
			unsigned int nLongitude = AscStrToInt(sLongitude);
			float fLongitude = nLongitude/1000000.0;
			sLongitudeDB = FloatToStr(fLongitude,6);			
		}

		//:��ӳ����ٶȵ����ݵ����
		TCString sSpeed = Mid(m_sRecvMsgBody,19,2);
		unsigned int nSpeed = AscStrToInt(sSpeed);
		//printf("nSpeed=%u\n",nSpeed);
		float fSpeed = nSpeed/10.0;
		TCString sSpeedDB = FloatToStr(fSpeed,1);
		// ��������: ������X��б��,������Y��б��,������Z��б��,���ٶ�Xֵ,���ٶ�Xֵ,���ٶ�Zֵ--�з���
		/*
		����(g) = �����ŵ�ԭʼ���� * 8 / 32768
		������(dps) = �����ŵ�ԭʼ���� * 125 / 32768
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
		// ���ٶ�=�����ŵ�ԭʼ����*8/32768
		// ������=�����ŵ�ԭʼ����*125/32768

		str_Space_Pos_Unit sPosNode;
		sPosNode.clear();
		sPosNode.dLongitude=StrToFloat(sLongitudeDB);
		sPosNode.dLatitude=StrToFloat(sLatitudeDB);
		
		//: ��Ϣ͸����վ��С����Ϣ;
		TCString sAdditonalLocInfo_Str=Right(m_sRecvMsgBody, 8);
				
		sLog = "Additional Data Lac-Cell Msg";
		glgMls->AddDataCollectionRunLog(sLog);

		int nCurLen=sAdditonalLocInfo_Str[2];	//: ������: Ӧ�õ���6;
		
		TCString sLac_Field=Mid(sAdditonalLocInfo_Str, 3, 2);
		std::string lac_str = (char *)sLac_Field;
		//for (size_t i = 0; i < 2; i++)
		//{
		//	printf("%02x:", lac_str[i]);
		//}
		int nLAC=AscStrToInt(sLac_Field);
		
		TCString sCellID_Field=Mid(sAdditonalLocInfo_Str, 5, 2);
		int nCellID=AscStrToInt(sCellID_Field);
		
		int nRscp= AscStrToInt(sAdditonalLocInfo_Str[7]);	//: RSCP�����ź��빦��
		int nRxLev= AscStrToInt(sAdditonalLocInfo_Str[8]);	//: RSSI�����ź�ǿ��
			
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
		int direction = AscStrToInt(Mid(m_sRecvMsgBody, 21, 1));	//: ����
		
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
// ���� : void TCDataCollectionJTT808Handler::DoCommand_DataTrans
// ��; : ��������͸��
// ԭ�� : void DoCommand_DataTrans()
// ���� : ��
// ���� : ��
// ˵�� :  
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
			//:�������İ�����������
			ServerGeneralAck(cusSocket);	
			return ;
		}
			
		unsigned char cTag = (unsigned char)m_sRecvMsgBody[1];
		if(cTag != 0xf1){
			//:����͸������ibeacon,��������
			ServerGeneralAck(cusSocket);	
			return ;
		}
		
		TCString sLog = "Data Transparent Msg";
		glgMls->AddDataCollectionRunLog(sLog);

		int nCurLen=m_sRecvMsgBody[2];	//: ������: Ӧ�õ���6;
		
		TCString sLac_Field=Mid(m_sRecvMsgBody, 3, 2);
		int nLAC=AscStrToInt(sLac_Field);
		
		TCString sCellID_Field=Mid(m_sRecvMsgBody, 5, 2);
		int nCellID=AscStrToInt(sCellID_Field);
		
		int nRscp=m_sRecvMsgBody[7];	//: RSCP�����ź��빦��
		int nRxLev=m_sRecvMsgBody[8];	//: RSSI�����ź�ǿ��
		
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
	  TCString sLog="InsertUuExdMR2DB SQLִ���쳣\n\tmsg=["+TCString((char*)p.msg)+"]\n\tvar_info=["+TCString((char*)p.var_info)+"]";
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
// ���� : void TCDataCollectionJTT808Handler::DoCommand_ClientRegister
// ��; : �ն�ע�ᴦ��
// ԭ�� : void DoCommand_ClientRegister()
// ���� : ��
// ���� : ��
// ˵�� :  
//==========================================================================
void TCDataCollectionJTT808Handler::DoCommand_ClientRegister(TCCustomUniSocket  &cusSocket){
		//:�յ���ע����Ϣ����ʱ������

		//:��ʼ����Ӧ���
		TCString sSendMsgBody = "";
		int nMsgSerialNum = htons(m_nMsgSerialNum);
		char *pMsgSerialNum = (char*)&nMsgSerialNum;
		TCString sMsgSerialNum = TCString(pMsgSerialNum,2);
		sSendMsgBody += sMsgSerialNum;

		unsigned char cResult = 0x00;
		sSendMsgBody += TCString(cResult);

		//:��Ȩ����ʱ��Ϊabcdef
		TCString sAuthCode = "abcdef";	
		sSendMsgBody += sAuthCode;

		m_nRespMsgId = 0x8100;
		m_sRespMsgBody=sSendMsgBody;
		
		SendRespPkg(cusSocket, m_sReq_Command, m_sRespMsgBody);
		return ;	
}


//==========================================================================
// ���� : void TCDataCollectionJTT808Handler::DoCommand_ClientAuth
// ��; : �ն˼�Ȩ
// ԭ�� : void DoCommand_ClientAuth()
// ���� : ��
// ���� : ��
// ˵�� :  
//==========================================================================
void TCDataCollectionJTT808Handler::DoCommand_ClientAuth(TCCustomUniSocket  &cusSocket){
		//:�յ��ļ�Ȩ���ݲ�����
		ServerGeneralAck(cusSocket);
		return ;	
}

//==========================================================================
// ���� : void TCDataCollectionJTT808Handler::ServerGeneralAck
// ��; : ͨ��Ӧ��
// ԭ�� : void ServerGeneralAck()
// ���� : ��
// ���� : ��
// ˵�� :  
//==========================================================================
void TCDataCollectionJTT808Handler::ServerGeneralAck(TCCustomUniSocket  &cusSocket){
	//:�յ��ļ�Ȩ���ݲ�����	
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
	
	//: ��Ӧ�������
	m_nRespMsgId=0x8001;
	m_sRespMsgBody=sMsgBody;
	
	SendRespPkg(cusSocket, m_sReq_Command, m_sRespMsgBody);
	return ;	
}

//==========================================================================
// ���� : void TCDataCollectionJTT808Handler::Init
// ��; : ��ʼ��
// ԭ�� : void Init()
// ���� : ��
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionJTT808Handler::Init(){
	m_sPkg_Content="";
	m_sReq_Command="";
	m_sRespCommand="";
	m_sRespPkgContent="";
	return;
}


//==========================================================================
// ���� : TCDataCollectionJTT808Handler::RecordTrackerLocInfo
// ��; : �û�λ����Ϣ�Ǽ�
// ԭ�� : RecordTrackerLocInfo()
// ���� : ��
// ���� : ��
// ˵�� : �û�λ����Ϣ�Ǽ�
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
		// ����
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
			//: ��ǰ��С����Ϣ;
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

		// ����imsiȥLOC_POSTERM_INFO��ȡ��������
		ry_jtt = (redisReply *)redisCommand(redis_context, "HGET LOC_POSTERM_INFO %s", (char *)record_info.imsi);
		if (ry_jtt->type == REDIS_REPLY_NIL || ry_jtt->type == REDIS_REPLY_ERROR) {
			LOG_WRITE("LOC_POSTERM_INFO���޶�Ӧ��imsi:[%s]", (char *)record_info.imsi);
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
		// LBS �������
		ry_jtt = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:PositionData:FromLBSCollection %s", (char *)str_csv.c_str());
		if (ry_jtt->type == REDIS_REPLY_ERROR)
		{
			perror("error:LPUSH WuDongDe:PositionData:FromLBSCollection");
		}
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;

		// ��ԭʼ��¼���ʹ��
		ry_jtt = (redisReply *)redisCommand(redis_context, "LPUSH WuDongDe:SrcRecordForOracle %s", (char *)str_csv.c_str());
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;

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

		ry_jtt = (redisReply *)redisCommand(redis_context, "HSET WuDongDe:PostermLastDataHash %s %s",
			(char *)AllTrim(record_info.imsi), (char *)str_latest_data.c_str());
		freeReplyObject(ry_jtt);
		ry_jtt = NULL;
		//
		// --20161026
		// ����ָ�Ʋɼ�����--imsi+timestamp[20161026142021--yyyymmddhhmmss],device_type
		str_latest_data = (char *)record_info.time_stamp;
		str_latest_data += ",";
		str_latest_data += term_type;

		// �ն�״̬ȷ��
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
		TCString sLog="RecordTrackerLocInfo �쳣�����"+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="RecordTrackerLocInfo ����δ֪����";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}



