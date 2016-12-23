/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 日志类
File Name   : c_DataCollection_log.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#include "c_DataCollection_log.h"

extern TCDataCollectionConfig* gDataCollectionConfig;

//----------------------------------------------------------
TCCriticalSection TCDataCollectionLog::csLog;

//==========================================================================
// 函数 : TCDataCollectionLog::AddDataCollectionRunLog()
// 用途 : 写运行日志
// 原型 : AddDataCollectionRunLog(const TCString& sLog)
// 参数 : sLog -- 要写入日志的内容
// 返回 : 无
// 说明 : 
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
// 函数 :TCDataCollectionLog::AddDataCollectionRunLog
// 用途 : 写运行日志
// 原型 : AddDataCollectionRunLog(char *sLog)
// 参数 : sLog -- 要写入日志的内容
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionLog::AddDataCollectionRunLog(char* sLog)
{
    AddDataCollectionRunLog(TCString(sLog));
}

//==========================================================================
// 函数 :TCDataCollectionLog::DebugLog
// 用途 : 写运行日志
// 原型 : DebugLog(char *sLog)
// 参数 : sLog -- 要写入日志的内容
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionLog::DebugLog(char* sLog)
{
    if(gDataCollectionConfig->IsDebug())
    	AddDataCollectionRunLog(TCString(sLog));
}

//==========================================================================
// 函数 :TCDataCollectionLog::DebugLog
// 用途 : 写运行日志
// 原型 : DebugLog(const TCString& sLog)
// 参数 : sLog -- 要写入日志的内容
// 返回 : 无
// 说明 : 
//==========================================================================
void TCDataCollectionLog::DebugLog(const TCString& sLog)
{
    if(gDataCollectionConfig->IsDebug())
    	AddDataCollectionRunLog(sLog);
}

