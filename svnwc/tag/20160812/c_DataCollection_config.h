/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 配置文件类
File Name   : c_DataCollection_config.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#ifndef C_DataCollection_CONFIG_H
#define C_DataCollection_CONFIG_H

#include "cmpublic.h"
#include "c_DataCollection_Public.h"
#include <map>

using namespace std;

class TCDataCollectionConfig{
private:
	bool 	m_bDebugLog;			//: 是否打开调试	
	bool	m_bOpenLBS_Location;	//: 是否打开LBS定位

  long     m_nPort;				//: 服务器的本地端口
  TCString m_sAddress;		//: 服务器的地址；一般就是本机
  long     m_nMaxThread;	//: 最大线程数
  long     m_nMinThread;	//: 最小线程数
  long     m_nQueue;			//: 队列长度
	TCString m_redis_server_ip;
	long     m_redis_server_port;
	TCString m_redis_auth;
  
  TCString m_sTracker_Vendor;	//: 前端厂家
	
	TCString m_sDBConStr;		//: 数据库连接串
	TCString m_sTableSpace;	//: 表空间名
	
	//: 指定 分区表的初始创建语句的文件名
	map<TCString, TCString> m_ssTable_InitCrtFileName;	//: 分区表名 --- 初始创建的语句文件名
	
	//: 指定 相关分区表的创建分区语句的文件名
	map<TCString, TCString> m_ssTable_AddPartFileName;	//: 分区表名 --- 新增分区的语句文件名
	
	
private:	
	void LoadIniFile();
	
public:

	TCDataCollectionConfig();
	~TCDataCollectionConfig();  
  
	bool IsDebug(){ return m_bDebugLog; }; 

	bool IsOpenLBSLocation() { return m_bOpenLBS_Location; };
		
	TCString & GetDBConnStr(){
		return m_sDBConStr;
	};

	TCString GetServerAddress(){
  	return m_sAddress;
  };

  long GetServerPort(){
   	return m_nPort;
  };
	TCString GetRedisServerAddress() {
		return m_redis_server_ip;
	};
	long GetRedisServerPort() {
		return m_redis_server_port;
	};
  long GetMaxThread(){
   	return m_nMaxThread;
  };

  long GetMinThread(){
  	return m_nMinThread;
 	};

  long GetQueue(){
  	return m_nQueue;
  };
  
  TCString GetTrackerVendor(){
  	return m_sTracker_Vendor;
  };
	TCString GetRedisAuth() {
		return m_redis_auth;
	}
};
#endif
