/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 相关配置表加载类
File Name   : c_DataCollection_Handle.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#include "c_DataCollection_Handle.h"

extern TCDataCollectionConfig *gDataCollectionConfig;

extern TCDataCollectionLog  *glgMls; 

//: 数据库连接
extern otl_connect m_dbConnect;

extern map<str_Lac_Cell_Node, str_Space_Pos_Unit> g_msCellInfo_PosUnit;

//==========================================================================
// 函数 :   
// 用途 :  
// 原型 : 
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionHandler::TCDataCollectionHandler(){
  	Init();
}
//==========================================================================
// 函数 :    
// 用途 :
// 原型 : 
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionHandler::~TCDataCollectionHandler(){
}

//==========================================================================
// 函数 : TCDataCollectionHandler::Init
// 用途 : 对内部变量进行初始化
// 原型 : Init()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionHandler::Init(){
	return;    
}


//==========================================================================
// 函数 : TCDataCollectionHandler::LoadAllCellPosInfo
// 用途 : 加载全网工参信息
// 原型 : LoadAllCellPosInfo()
// 参数 : 无
// 返回 : 无
// 说明 : 加载全网工参信息
//==========================================================================
bool TCDataCollectionHandler::LoadAllCellPosInfo(){
	try{ 		
    TCString sSQL="SELECT LAC, CI, LONGITUDE, LATITUDE from mb_sys_cell_info ";
    
    
    otl_stream otlParamQry(200, // buffer size
                      (char*)sSQL,
                       m_dbConnect);// connect object
    int nStatus=-1;
   	
   	TCString sLog="LoadAllCellPosInfo 加载小区经纬度信息，SQL["+sSQL+"]";
		glgMls->AddDataCollectionRunLog(sLog);
     
    while(!otlParamQry.eof()){
			int nLac=0;
			int nCI=0;
			char sLongtitude[32];
			char sLatitude[32];
			memset(sLongtitude, 0, sizeof(sLongtitude));
			memset(sLatitude, 0, sizeof(sLatitude));
      
      otlParamQry>>nLac;
      otlParamQry>>nCI;
      otlParamQry>>sLongtitude;
      otlParamQry>>sLatitude;

			str_Lac_Cell_Node sLacCellNode;
			sLacCellNode.clear();
			sLacCellNode.nLac=nLac;
			sLacCellNode.nCellID=nCI;
       
			str_Space_Pos_Unit sPosNode;
			sPosNode.dLongitude=StrToFloat(TCString(sLongtitude));
			sPosNode.dLatitude=StrToFloat(TCString(sLatitude));
       
			g_msCellInfo_PosUnit[sLacCellNode]=sPosNode;
    }

		sLog="LoadAllCellPosInfo 加载小区经纬度信息，小区数量=["+IntToStr(g_msCellInfo_PosUnit.size())+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		
		return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="LoadAllCellPosInfo SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="LoadAllCellPosInfo 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="LoadAllCellPosInfo 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}


//==========================================================================
// 函数 : TCDataCollectionHandler::TrackerLoginCheck
// 用途 : 用户认证
// 原型 : TrackerLoginCheck()
// 参数 : 无
// 返回 : 无
// 说明 : 查询该用户是否正常用户；
//==========================================================================
bool TCDataCollectionHandler::TrackerLoginCheck(TCString sImei, TCString sImsi, TCString sDev_Type, TCString sICCID, int nErrorCode){
	try{ 
		
		return true;  
    TCString sSQL="SELECT  status from mb_bss_terminal_info where  imei=:F1IMEI<char[17]>";
    
    
    otl_stream otlParamQry(200, // buffer size
                      (char*)sSQL,
                       m_dbConnect);// connect object
    int nStatus=-1;
    
    otlParamQry<<(char*)sImei;
    
    while(!otlParamQry.eof()){
       nStatus=0;       
       otlParamQry>>nStatus;       
    }
    
    if(nStatus==-1){
    	//: 无效用户
    	nErrorCode=101;
    	return false;
    }else{
    	nErrorCode=0;
    	return true;
    }
		return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="TrackerLoginCheck SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="TrackerLoginCheck 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="TrackerLoginCheck 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}



//==========================================================================
// 函数 : TCDataCollectionHandler::GetPushCommandList
// 用途 : 查询需要Push到Tracker上的消息
// 原型 : GetPushCommandList()
// 参数 : 无
// 返回 : 无
// 说明 : 查询需要Push到Tracker上的消息
//==========================================================================
bool TCDataCollectionHandler::GetPushCommandList(vector<TCString> &vsPush_Cmd_List){
	try{   
    TCString sSQL="SELECT  status from mb_bss_terminal_info where  imei=:F1IMEI<char[17]>";
    
    
    otl_stream otlParamQry(200, // buffer size
                      (char*)sSQL,
                       m_dbConnect);// connect object
    int nStatus=-1;
    
    
    while(!otlParamQry.eof()){
       nStatus=0;       
       otlParamQry>>nStatus;       
    }
    
    if(nStatus==-1){
    	//: 无效用户
    	return false;
    }else{
    	return true;
    }
		return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="GetPushCommandList SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="GetPushCommandList 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="GetPushCommandList 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}


//==========================================================================
// 函数 : TCDataCollectionHandler::GetSFNCmdParam
// 用途 : 查询 SFN 命令需要的参数
// 原型 : GetSFNCmdParam()
// 参数 : 无
// 返回 : 无
// 说明 : 查询 SFN 命令需要的参数
//==========================================================================
bool TCDataCollectionHandler::GetSFNCmdParam(TCString sCustID, map<int, TCString> &msIdxID_FamilyMsisdn, otl_datetime &MaxTimer){
	try{   
    TCString sSQL="SELECT  key_number, msisdn, Oper_time from mb_bss_user_family_infor where  imei=:F1IMEI<char[17]> and status=1 order by Oper_time";
    
    msIdxID_FamilyMsisdn.clear();
    
    otl_stream otlParamQry(200, // buffer size
                      (char*)sSQL,
                       m_dbConnect);// connect object
    
    otlParamQry<<(char*)sCustID;
    while(!otlParamQry.eof()){
       char sKeyID[32];
       char sMsisdn[32];
       memset(sKeyID, 0, sizeof(sKeyID));
       memset(sMsisdn, 0, sizeof(sMsisdn));
       
       otlParamQry>>sKeyID; 
       otlParamQry>>sMsisdn; 
       otlParamQry>>MaxTimer;
            
       msIdxID_FamilyMsisdn[atol(sKeyID)]=TCString(sMsisdn);
    }
    return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="GetSFNCmdParam SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="GetSFNCmdParam 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="GetSFNCmdParam 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}


//==========================================================================
// 函数 : TCDataCollectionHandler::UpdateSFNCmdParamStatus
// 用途 : 更新SFN设置状态
// 原型 : UpdateSFNCmdParamStatus()
// 参数 : 无
// 返回 : 无
// 说明 : 更新SFN设置状态
//==========================================================================
bool TCDataCollectionHandler::UpdateSFNCmdParamStatus(TCString sCustID, otl_datetime &MaxTimer){
	try{   
    TCString sSQL="update mb_bss_user_family_infor set status=2, Oper_time=sysdate ";
    sSQL+=" where  imei=:F1IMEI<char[17]> and status=1 and Oper_time<=:F2Oper_Time<timestamp>";
    
    TCString sCurSFNOperTimestamp="";
    OTLDateTime2String(MaxTimer, sCurSFNOperTimestamp);
    
    if(!TCTime::IsValidDatetime(sCurSFNOperTimestamp)){
    	//: 非法时间格式
    	sCurSFNOperTimestamp=TCTime::Now();
    	String2OTLDateTime(sCurSFNOperTimestamp, MaxTimer);
    }else{
    	//: 合法时间格式，时间差超过阀值;
    	if(abs((int)(TCTime::SecondsAfter(sCurSFNOperTimestamp, TCTime::Now())))>2*3600){
    		//: 过期时间;
    		sCurSFNOperTimestamp=TCTime::Now();
    		String2OTLDateTime(sCurSFNOperTimestamp, MaxTimer);
    	}
    }
    
    otl_stream otlParamQry(200, // buffer size
                      (char*)sSQL,
                       m_dbConnect);// connect object
                 
    otlParamQry<<(char*)sCustID;
    otlParamQry<<MaxTimer;
    otlParamQry.flush();
    return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="UpdateSFNCmdParamStatus SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="UpdateSFNCmdParamStatus 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="UpdateSFNCmdParamStatus 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}

//==========================================================================
// 函数 : TCDataCollectionHandler::RecordTrackerStatus
// 用途 : 状态码登记处理
// 原型 : RecordTrackerStatus()
// 参数 : 无
// 返回 : 无
// 说明 : 状态码登记处理
//==========================================================================
bool TCDataCollectionHandler::RecordTrackerStatus(TCString sCustID, TCString sStatus_Code){
	try{   
		TCString sCurTimeStamp=Mid(sStatus_Code, 1, 12);
		
		int nCurPowerLevel=StrToInt(Mid(sStatus_Code, 13, 3));

		int nLoc_Func_OffOn=StrToInt(Mid(sStatus_Code, 16, 1));
		
		int nLoc_Func_Dur=StrToInt(Mid(sStatus_Code, 17, 2));
		
		int nBreak_Func_OffOn=StrToInt(Mid(sStatus_Code, 19, 1));

		int nBreak_Func_Level=StrToInt(Mid(sStatus_Code, 20, 1));

		int nStep_Func_OffOn=StrToInt(Mid(sStatus_Code, 21, 1));

		int nWatch_Tape_OffOn=StrToInt(Mid(sStatus_Code, 22, 1));

		int nWatch_Tape_Status=StrToInt(Mid(sStatus_Code, 23, 1));

		int nBlueTooth_OffOn=StrToInt(Mid(sStatus_Code, 24, 1));
		
		printf("CurTime=[%s], PowerLevel=[%d],LocFunc=[%d],Loc_Dur=[%d],Break_Func=[%d],Break_Level=[%d],BLE=[%d]\n",\
					(char*)sCurTimeStamp,nLoc_Func_OffOn, nLoc_Func_Dur, nBreak_Func_OffOn, nBreak_Func_Level, nBlueTooth_OffOn);

		
/*		
    TCString sSQL="SELECT  status from mb_bss_terminal_info where  imei=:F1IMEI<char[17]>";
    
    
    otl_stream otlParamQry(200, // buffer size
                      (char*)sSQL,
                       m_dbConnect);// connect object
    int nStatus=-1;
    
    
    while(!otlParamQry.eof()){
       nStatus=0;       
       otlParamQry>>nStatus;       
    }
    
    if(nStatus==-1){
    	//: 无效用户
    	return false;
    }else{
    	return true;
    }
*/    
		return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="RecordTrackerStatus SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="RecordTrackerStatus 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="RecordTrackerStatus 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}


//==========================================================================
// 函数 : TCDataCollectionHandler::RecordTrackerLocationInfo
// 用途 : 用户位置信息登记
// 原型 : RecordTrackerLocationInfo()
// 参数 : 无
// 返回 : 无
// 说明 : 用户位置信息登记
//==========================================================================
bool TCDataCollectionHandler::RecordTrackerLocationInfo(TCString sCustID,TCString sImei, TCString sImsi, str_Space_Pos_Unit sPosNode, int nPower_Level, int nLoc_Type,  TCString sTimeStamp, str_Lac_Cell_Node sLBSNode, int nRxLev){
	try{   
		if(!gDataCollectionConfig->IsOpenLBSLocation() && abs(sPosNode.dLongitude-0.00001)<=0.0001 && abs(sPosNode.dLatitude-0.00001)<=0.0001 ){
			//: 关闭 LBS 定位方式；此时不能为 0.000 . 
			return false;
		}
		
		//: 插入数据到结果表中
		TCString sSQL="INSERT INTO mb_bss_terminal_location(day,imei,longitude,latitude,location_type,electricity,active_time";
		if(nLoc_Type==1){
			//: LBS 定位的时候，需要增加小区信息和场强
			sSQL+=",LBS_LAC,LBS_CELL,LBS_RXLEV";
		}
		sSQL+=")";
		sSQL+="VALUES(";
		sSQL+=":F0Day<timestamp>, :F1IMEI<char[65]>,:F2LONGTITUDE<char[33]>, :F3LATITUDE<char[33]>, :F4LOC_TYPE<char[9]>, :F5electricity<char[9]>,:F6ACT_TIME<timestamp>";
		if(nLoc_Type==1){
			//: LBS 定位的时候，需要增加小区信息和场强
			sSQL+=",:F7LBS_LAC<int>,:F8LBS_CELL<int>,:F9LBS_RXLEV<int>";
		}
		sSQL+=")";
    
    otl_stream otlParamQry(200, // buffer size
                      (char*)sSQL,
                       m_dbConnect);// connect object
    
    otl_datetime otl_DayTimer;
    String2OTLDateTime(Mid(sTimeStamp, 1, 8)+"000000", otl_DayTimer);
    
    otlParamQry<<otl_DayTimer;
    
    otlParamQry<<(char*)sImei;
    otlParamQry<<(char*)FloatToStr(sPosNode.dLongitude,6);
    otlParamQry<<(char*)FloatToStr(sPosNode.dLatitude,6);
    otlParamQry<<(char*)IntToStr(nLoc_Type);
    otlParamQry<<(char*)IntToStr(nPower_Level);
    
    otl_datetime otl_ActTimer;
    String2OTLDateTime(sTimeStamp, otl_ActTimer);
    
    otlParamQry<<otl_ActTimer;
    
    if(nLoc_Type==1){
    	otlParamQry<<sLBSNode.nLac;
    	otlParamQry<<sLBSNode.nCellID;
    	otlParamQry<<nRxLev;
    }
    
    TCString sLog="RecordTrackerStatus Record Cur IMEI=["+sImei +"], Coordinate=("+FloatToStr(sPosNode.dLongitude,6)+","+FloatToStr(sPosNode.dLatitude,6)+")";
		glgMls->AddDataCollectionRunLog(sLog);
    
		return true;
	}catch(otl_exception& p){ // intercept OTL exceptions	
		cerr<<p.msg<<endl; // print out error message
    cerr<<p.stm_text<<endl; // print out SQL that caused the error
    cerr<<p.var_info<<endl; // print out the variable that caused the error
    TCString sLog="RecordTrackerLocationInfo SQL Error: Msg=["+TCString((char*)p.msg) +"] SQL=["+TCString((char*)p.stm_text)+"]";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(TCException &Excep){
		TCString sLog="RecordTrackerLocationInfo 异常情况："+Excep.GetMessage();
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}catch(...){
		TCString sLog="RecordTrackerLocationInfo 出现未知错误";
		printf("%s\n", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}

