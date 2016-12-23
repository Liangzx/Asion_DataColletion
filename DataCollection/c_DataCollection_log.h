/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : ��־��
File Name   : c_DataCollection_log.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#ifndef C_DataCollection_LOG_H
#define C_DataCollection_LOG_H

#include "cmpublic.h"
#include "c_critical_section.h"
#include "c_DataCollection_config.h"

class TCDataCollectionLog
{
public:
        static TCCriticalSection csLog;
public:
        void AddDataCollectionRunLog(const TCString&);        
        void AddDataCollectionRunLog(char*);
        void DebugLog(const TCString&);
        void DebugLog(char*);
};


#endif
