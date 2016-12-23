/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : �������
File Name   : DataCollection.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#include "cmpublic.h"
#include "c_DataCollection_Public.h"
#include "c_DataCollection_config.h"
#include "c_DataCollection_Handle.h"
#include "c_DataCollection_log.h"
#include "c_DataCollection_server.h"
#define OTL_ORA8I
#include "otlv4.h"
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include "hiredis.h"

using namespace std;

#define VERSION "v1.1"

//: �ڴ�ȫ������
static bool bFirstRunFlag = true;
static bool nCurNeedReloadParam=true;

//: ���ݿ�����
otl_connect m_dbConnect;

TCString g_msCurTableDay;

redisContext * redis_context = NULL;


TCDataCollectionConfig *gDataCollectionConfig;
TCDataCollectionServer *gDataCollectionServer;

TCDataCollectionHandler gsDataCollectionHandler;

//: ��־����
TCDataCollectionLog  *glgMls; 

map<str_Lac_Cell_Node, str_Space_Pos_Unit> g_msCellInfo_PosUnit;

void MainFunc();
void ReleaseMainFunc();
bool DbConnect();

//==========================================================================
// ���� : main
// ��; : ������
// ԭ�� : void main()
// ���� : 
// ���� : 
// ˵�� :
//==========================================================================
int main(int argc, char* argv[]){
	
		if(argc>=2){
			//: �����ǰ汾��ѯ��Ϣ��
			TCString sArgv_Str=argv[1];
			if(sArgv_Str=="-v"){
				printf("��������:DataCollection.bin\n");
				printf("��������:��Ա��λ��Ϣ�ɼ�\n");
    		printf("�����ļ�:~/config/DataCollection.ini\n");
    		printf("��ǰ�汾��:[%s]\n", (char*)VERSION);
    		exit(0);
			}
  	}		
  	
    g_msCurTableDay="";
    gDataCollectionServer =NULL;
    printf("���������� Time=[%s]\n", (char*)TCTime::Now());
		otl_connect::otl_initialize(1);
    Application.MutiProcess(20);		//: ��������ã�


    Application.Initialize("DataCollection", argc, argv);
    Application.SetRunningHandle(MainFunc);
    
    try{    
    Application.SetRunningDelay(2000);
		StartupSocket(); 
		Application.SetDiskFreePercentPause(0);
    Application.SetDiskFreePercentStop(0);
    Application.DisableCheckTime(); 

		gDataCollectionConfig = new TCDataCollectionConfig;
		glgMls = new TCDataCollectionLog;

		
    Application.Run();
  }catch(TCException &e){
    TCString sLogMsg = TCString("At MainFuc ") + e.GetExceptionMessage();
    printf("sLogMsg=[%s]\n", (char*)sLogMsg);
  }
  
		if (gDataCollectionConfig != NULL){
			delete gDataCollectionConfig;
		}
		if (glgMls!=NULL){
			delete glgMls;
		}
		otl_connect::otl_terminate();

	  return 0;
}

//==========================================================================
// ���� : MainFunc
// ��; : ���س���
// ԭ�� : void MainFunc()
// ���� : 
// ���� : 
// ˵�� :
//==========================================================================
void MainFunc(){
		if(gDataCollectionServer==NULL){
		//: �����������ݿ�
		if(!DbConnect()){
    	TCString sLog="Error DB Connect\n";
			printf("%s", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
			exit(0);
		}else{
			TCString sLog="\nMainFunc: ϵͳ���������ݿ����ӳɹ�����ʼ��������������\n";
			printf("%s", (char*)sLog);
			glgMls->AddDataCollectionRunLog(sLog);
		}

		
		TCDataCollectionHandler sLacCellParamHandler;
		sLacCellParamHandler.LoadAllCellPosInfo();

		TCString sLog="Main , LoadAllCellPosInfo: Start Load\n";
		printf("%s", (char*)sLog);
		glgMls->AddDataCollectionRunLog(sLog);
		
  	Application.InstallExitHandle(ReleaseMainFunc);
	
    try{
    	//����MSC���� �������߳�
      TCString sServerAddr = gDataCollectionConfig->GetServerAddress();
      long nPort = gDataCollectionConfig->GetServerPort();
      long nMinThread = gDataCollectionConfig->GetMinThread();
      long nMaxThread = gDataCollectionConfig->GetMaxThread();
      long nQueue= gDataCollectionConfig->GetQueue();
      
      printf("ServerAddr=[%s], Port=[%d]\n", (char*)sServerAddr, nPort);
        
      gDataCollectionServer = new TCDataCollectionServer(sServerAddr, "",nPort, nQueue, nMinThread, nMaxThread);

      if(gDataCollectionServer!=NULL){
        gDataCollectionServer->Do();
      }
    }catch(exception & e){
#ifdef __TEST__
			printf("MainFunc Recv a Fatal Error, System Exit:%s\n", e.what());
#endif
      exit(-1);
    }
  }  
}


//==========================================================================
// ���� : ReleaseMainFunc
// ��; : ����ִ�н����Ĵ������
// ԭ�� : void ReleaseMainFunc()
// ���� : 
// ���� : 
// ˵�� :
//==========================================================================
void ReleaseMainFunc(){
	if(gDataCollectionServer!=NULL)
		delete  gDataCollectionServer;
	gDataCollectionServer=NULL;
  CleanupSocket(); 
}

//==========================================================================
// ���� : DbConnect
// ��; : �������ݿ�
// ԭ�� : DbConnect()
// ���� : ��
// ���� : ��
// ˵�� : 
//==========================================================================
bool DbConnect()
{
  try{
		//: ��ȡ���ݿ����Ӵ�
		TCString sDBConnStr;
		sDBConnStr=gDataCollectionConfig->GetDBConnStr();

		char connstr[256];
		memset(connstr, 0, sizeof(connstr));
		strcpy(connstr, (char*)sDBConnStr); 
#ifdef __TEST__	
		printf("���Ӵ���connstr=[%s]\n", (char*)connstr);
#endif
		//: �������ݿ�

		m_dbConnect.rlogon(connstr); // connect to Oracle
		return true;
	}catch(...){
		TCString     sLog;
		sLog="initDbConnect()ִ��ʧ��";
		glgMls->AddDataCollectionRunLog(sLog);
		return false;
	}
}



