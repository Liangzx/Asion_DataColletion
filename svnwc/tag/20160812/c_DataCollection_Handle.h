/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 相关配置表加载类
File Name   : c_DataCollection_Handle.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#ifndef C_DataCollection_HANDLER_H
#define C_DataCollection_HANDLER_H

#include "cmpublic.h"
#include "c_DataCollection_Public.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_log.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#define OTL_ORA8I
#include "otlv4.h"
#include <algorithm>

using namespace std;

class TCDataCollectionHandler{
	private:
		//: 初始化相关的数据
		void Init();	
	public:
		TCDataCollectionHandler();
	
		~TCDataCollectionHandler();
	
		//: 用户认证
		bool TrackerLoginCheck(TCString sImei, TCString sImsi, TCString sDev_Type, TCString sICCID, int nErrorCode);	
		//: 查询需要Push到Tracker上的消息;
		bool GetPushCommandList(vector<TCString> &vsPush_Cmd_List);
		//: 查询 SFN 命令需要的参数
		bool GetSFNCmdParam(TCString sCustID, map<int, TCString> &msIdxID_FamilyMsisdn, otl_datetime &MaxTimer);
		//: 更新亲情号码状态
		bool UpdateSFNCmdParamStatus(TCString sCustID, otl_datetime &MaxTimer);		
		//: 状态码登记处理
		bool RecordTrackerStatus(TCString sCustID, TCString sStatus_Code);		
		//: 用户位置信息登记;
		bool RecordTrackerLocationInfo(TCString sCustID,TCString sImei, TCString sImsi, str_Space_Pos_Unit sPosNode, int nPower_Level, int nLoc_Type,  TCString sTimeStamp, str_Lac_Cell_Node sLBSNode, int nRxLev);	
		//: 加载全网工参信息
		bool LoadAllCellPosInfo();
		
};


#endif


