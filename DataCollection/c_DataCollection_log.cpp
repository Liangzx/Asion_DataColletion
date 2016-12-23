/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : ��־��
File Name   : c_DataCollection_log.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#include "c_DataCollection_log.h"

extern TCDataCollectionConfig* gDataCollectionConfig;

//----------------------------------------------------------
TCCriticalSection TCDataCollectionLog::csLog;

//==========================================================================
// ���� : TCDataCollectionLog::AddDataCollectionRunLog()
// ��; : д������־
// ԭ�� : AddDataCollectionRunLog(const TCString& sLog)
// ���� : sLog -- Ҫд����־������
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionLog::AddDataCollectionRunLog(const TCString& sLog)
{
    csLog.Enter();
    try
    {
        TCString strFileName;
        TCString sLogPath;
        TCString strAppName;
        TCString strBuff;
        TCFileStream cFile;
        strAppName = Application.GetAppName();
        //sLogPath = TAppPath::AppLog() + strAppName;
				sLogPath = IncludeTrailingSlash(gDataCollectionConfig->GetLogDir()) + strAppName;
        ForceDirectories(sLogPath);
        //strFileName = TCAppLog::GetDailyLogFileNameWithTag(Application.GetProcessFlag());
				strFileName = IncludeTrailingSlash(sLogPath) + Application.GetAppName() + TCTime::Today();
				strFileName += "_";
				strFileName += Application.GetProcessFlag();
				strFileName += ".mlg";
				//printf("log file: %s\n", (char *)strFileName);
        cFile.Open(strFileName,omAppend);
        strBuff = TCTime::Now()+": ";
        strBuff += sLog;
        cFile.WriteLn(strBuff);
        cFile.Close();
    }
    catch(TCException& e)
    {
    	printf("\nLog Exception:%s",(char*)e.GetExceptionMessage());
    }
    csLog.Leave();
}

//==========================================================================
// ���� :TCDataCollectionLog::AddDataCollectionRunLog
// ��; : д������־
// ԭ�� : AddDataCollectionRunLog(char *sLog)
// ���� : sLog -- Ҫд����־������
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionLog::AddDataCollectionRunLog(char* sLog)
{
    AddDataCollectionRunLog(TCString(sLog));
}

//==========================================================================
// ���� :TCDataCollectionLog::DebugLog
// ��; : д������־
// ԭ�� : DebugLog(char *sLog)
// ���� : sLog -- Ҫд����־������
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionLog::DebugLog(char* sLog)
{
    if(gDataCollectionConfig->IsDebug())
    	AddDataCollectionRunLog(TCString(sLog));
}

//==========================================================================
// ���� :TCDataCollectionLog::DebugLog
// ��; : д������־
// ԭ�� : DebugLog(const TCString& sLog)
// ���� : sLog -- Ҫд����־������
// ���� : ��
// ˵�� : 
//==========================================================================
void TCDataCollectionLog::DebugLog(const TCString& sLog)
{
    if(gDataCollectionConfig->IsDebug())
    	AddDataCollectionRunLog(sLog);
}

