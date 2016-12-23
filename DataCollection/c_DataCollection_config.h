/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : �����ļ���
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
	bool 	m_bDebugLog;			//: �Ƿ�򿪵���	
	bool	m_bOpenLBS_Location;	//: �Ƿ��LBS��λ

  long     m_nPort;				//: �������ı��ض˿�
  TCString m_sAddress;		//: �������ĵ�ַ��һ����Ǳ���
  long     m_nMaxThread;	//: ����߳���
  long     m_nMinThread;	//: ��С�߳���
  long     m_nQueue;			//: ���г���
	TCString m_redis_server_ip;
	long     m_redis_server_port;
	TCString m_redis_auth;
	TCString m_log_dir;
  
  TCString m_sTracker_Vendor;	//: ǰ�˳���
  bool jtt_update_flag;
  TCString jtt_update_param_str;
	
	TCString m_sDBConStr;		//: ���ݿ����Ӵ�
	TCString m_sTableSpace;	//: ���ռ���
	
	//: ָ�� �������ĳ�ʼ���������ļ���
	map<TCString, TCString> m_ssTable_InitCrtFileName;	//: �������� --- ��ʼ����������ļ���
	
	//: ָ�� ��ط������Ĵ������������ļ���
	map<TCString, TCString> m_ssTable_AddPartFileName;	//: �������� --- ��������������ļ���
	
	
private:	
	void LoadIniFile();
	
public:

	TCDataCollectionConfig();
	~TCDataCollectionConfig();  
  
	bool IsDebug(){ return m_bDebugLog; }; 

	bool IsOpenLBSLocation() { return m_bOpenLBS_Location; };
		
	TCString  GetDBConnStr(){
		return m_sDBConStr;
	};

	TCString GetLogDir() {
		return m_log_dir;
	}

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

	bool GetJttUpdateFlag() {
		return jtt_update_flag;
	}

	TCString GetJttUpdateParam(){
		return jtt_update_param_str;
	}
};
#endif