/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : ������ñ������
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
		//: ��ʼ����ص�����
		void Init();	
	public:
		TCDataCollectionHandler();
	
		~TCDataCollectionHandler();
	
		//: �û���֤
		bool TrackerLoginCheck(TCString sImei, TCString sImsi, TCString sDev_Type, TCString sICCID, int nErrorCode);	
		//: ��ѯ��ҪPush��Tracker�ϵ���Ϣ;
		bool GetPushCommandList(vector<TCString> &vsPush_Cmd_List);
		//: ��ѯ SFN ������Ҫ�Ĳ���
		bool GetSFNCmdParam(TCString sCustID, map<int, TCString> &msIdxID_FamilyMsisdn, otl_datetime &MaxTimer);
		//: �����������״̬
		bool UpdateSFNCmdParamStatus(TCString sCustID, otl_datetime &MaxTimer);		
		//: ״̬��ǼǴ���
		bool RecordTrackerStatus(TCString sCustID, TCString sStatus_Code);		
		//: �û�λ����Ϣ�Ǽ�;
		bool RecordTrackerLocationInfo(TCString sCustID,TCString sImei, TCString sImsi, str_Space_Pos_Unit sPosNode, int nPower_Level, int nLoc_Type,  TCString sTimeStamp, str_Lac_Cell_Node sLBSNode, int nRxLev);	
		//: ����ȫ��������Ϣ
		bool LoadAllCellPosInfo();
		
};


#endif


