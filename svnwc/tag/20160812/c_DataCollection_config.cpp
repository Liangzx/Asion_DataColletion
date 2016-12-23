/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 配置文件类
File Name   : c_DataCollection_config.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#include "c_DataCollection_config.h"
//---------------------------------------------------------------------------
//==========================================================================
// 函数 :   
// 用途 :  
// 原型 : 
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionConfig::TCDataCollectionConfig(){
  	LoadIniFile();
}
//==========================================================================
// 函数 :    
// 用途 :
// 原型 : 
// 参数 :
// 返回 :
// 说明 :
//==========================================================================
TCDataCollectionConfig::~TCDataCollectionConfig(){
}

//==========================================================================
// 函数 : TCDataCollectionConfig::LoadIniFile
// 用途 : 载入配置文件
// 原型 : LoadIniFile()
// 参数 : 无
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionConfig::LoadIniFile(){
    //初始化相关基础数据
    m_bDebugLog=false;    
    m_nPort=0;
    m_sAddress="127.0.0.1";
    m_nMaxThread=30;
    m_nMinThread=20;
    m_nQueue=20;
    m_sTracker_Vendor="";
		m_bOpenLBS_Location=true;
    
    TCString  sReadStr;
    sReadStr = ProfileAppString(Application.GetAppName(),
        "GENERAL","DEBUG", "");
   	sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
    if ( sReadStr == "Y" ) m_bDebugLog = true;
        
    sReadStr = ProfileAppString(Application.GetAppName(),
        "GENERAL","OPEN_LBS_LOC", "");
   	sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
    if ( sReadStr == "N" ) m_bOpenLBS_Location= false;

    sReadStr = ProfileAppString(Application.GetAppName(),
       "GENERAL","DB_CONSTR", "");
    sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
    m_sDBConStr=sReadStr;
    ASSERT( m_sDBConStr != "" );

		sReadStr = ProfileAppString(Application.GetAppName(),
       "GENERAL","VENDOR", "");
    sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
    m_sTracker_Vendor=sReadStr;
    ASSERT( m_sTracker_Vendor != "" );

		

    //: 服务器IP地址
    sReadStr = ProfileAppString(Application.GetAppName(),
        "GENERAL","SERVER_IP", "");
    sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
    if(m_sAddress!="")
	    m_sAddress=sReadStr;    
    //: 端口号
    sReadStr = ProfileAppString(Application.GetAppName(),
        "GENERAL","SERVER_PORT", "");
    sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
     if (sReadStr != "") 
    	m_nPort = StrToInt(sReadStr);
    
		 // Redis ip and port and auth
		 m_redis_server_ip = ProfileAppString(Application.GetAppName(),
			 "GENERAL", "REDIS_SERVER_IP", "127.0.0.1");
		 sReadStr = ProfileAppString(Application.GetAppName(),
			 "GENERAL", "REDIS_SERVER_PORT", "6379");
		 m_redis_server_port = StrToInt(sReadStr);
		 m_redis_auth = ProfileAppString(Application.GetAppName(),
			 "GENERAL", "REDIS_SERVER_AUTH", "");

		//: 最大线程数；
    sReadStr = ProfileAppString(Application.GetAppName(),
        "GENERAL","MAX_THREAD", "");
    sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
     if (sReadStr != "") 
    	m_nMaxThread = StrToInt(sReadStr);
    	
    //: 最小线程数；
    sReadStr = ProfileAppString(Application.GetAppName(),
        "GENERAL","MIN_THREAD", "");
    sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
     if (sReadStr != "") 
    	m_nMinThread = StrToInt(sReadStr);
    	
    //: 队列数；
    sReadStr = ProfileAppString(Application.GetAppName(),
        "GENERAL","QUEUE", "");
    sReadStr=AllTrim(sReadStr);
    sReadStr=TrimCRLF(sReadStr);	//: 去掉回车换行
     if (sReadStr != "") 
    	m_nQueue = StrToInt(sReadStr);	

  
    return;
}
