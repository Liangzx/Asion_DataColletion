/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : OBD定位设备协议处理类
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
	unsigned long speed_; //速度
	unsigned long direction_; // 方向
	unsigned long bat_vol_; // 电压
	unsigned long  latitude_;// 纬度
	unsigned long  longitude_;// 经度
	unsigned long  altitude_;//海拔

}GpsInfo;

class TCDataCollectionOBDHandler
{
private:
	long  m_nTimeOut;
	long  m_nProcessTime;
	// 写入redis的结构
	RecordInfo record_info_;

	// JT/T808-2011部颁标准的关键数据
	unsigned short m_nMsgId;	// 消息类型
	unsigned short m_nMsgAttr;	// 消息属性
	int m_nMsgBodyLen;			// 消息体长度
	TCString m_sMsgName;		// 消息名称
	TCString m_sMsisdn;			// 手机号码
	// obd 非imei
	TCString m_device_id;		// 终端id
	TCString m_no_imsi;			// 手机imsi
	TCString m_no_imei;			// 手机imsi
	unsigned short m_nMsgSerialNum;	// 消息序号
	TCString m_sRecvMsgBody;	// 接收到的消息体

	unsigned short m_nRespMsgSerialNum;	// 序号;
	unsigned short m_nRespMsgId;	// 消息类型
	TCString m_sRespMsgBody;	// 响应消息体；


	TCString  m_sRecvTime;
	TCString  m_sSendTime;

	// 终端上报IP地址；
	TCString m_sTacker_Send_IPAddress;

	// 报文头的前缀 CWT
	TCString m_sPkg_Head_Flag;
	// 报文头的内容的长度
	long m_nContent_Len;

	// 报文内容域的整个内容--OBD不包含头尾--为解包后的
	TCString m_sPkg_Content;

	// 分割字符
	char m_cDelimter;

	// 报文内容域解析后的请求命令字
	TCString m_sReq_Command;

	// 报文一次解析 （按照 & 分割解析） 后的字段列表
	TCStringList m_lsContentFieldList;

	// 报文内容处理后的响应命令字
	TCString m_sRespCommand;

	// 报文内容处理后的响应报文内容
	TCString m_sRespPkgContent;

	static TCCriticalSection m_csLogLock;


private:
	// OBD的数据处理方式，接收数据, 按照无阻塞接收
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
	// 终端登陆
	void DoCommand_Login_Resp(TCCustomUniSocket  &cusSocket);

	// 平台通用应答
	void DoCommand_GeneralResp(TCCustomUniSocket  &cusSocket, char msg_arr[2]);

	// 终端上传GPS定位数据
	void DoCommand_ParseGpsInfo(TCCustomUniSocket  &cusSocket);

	// 终端上传基站数据
	void DoCommand_ParseLbsInfo(TCCustomUniSocket  &cusSocket);
	void DoCommand_ParseLbsInfo_4G(TCCustomUniSocket  &cusSocket);
	// 报文转义--解报文
	TCString unpack_pkg(TCString sPkg_Content);
	// 报文转义--打包报文
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


