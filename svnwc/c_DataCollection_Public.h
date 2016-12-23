/*****************************************************************************
CAPTION
Project Name: �ڶ��¶�λϵͳ--���ݲɼ�ģ��
Description : ͨ�ýṹ�嶨��
File Name   : c_DataCollection_Public.h
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/


#ifndef C_DataCollection_PUBLIC_H
#define C_DataCollection_PUBLIC_H

#include "cmpublic.h"
#include "cmpublic_db.h"
#include <map>
#include <vector>
#define OTL_ORA8I
#include "otlv4.h"
#include <cmath>
#include <string>


#define NO_LOCATION 0
#define LOCATION 1


using namespace std;

struct RecordInfo {
	TCString unique_id_;			// �豸Ψһ��ʶ��app Ϊimei
	TCString longitude_;			// ����
	TCString latitude_;				// γ��
	double elevation_;				// ����
	TCString ble_x;					// ble x
	TCString ble_y;					// ble y
	TCString reserve_x_1;			// ����
	TCString reserve_y_1;			// ����
	int cellid_;					// cell id
	int lac_;						// lac
	TCString mobile_;				// ���� mobile ռδ�õ�
	TCString blemac_;				// ble mac
	TCString blemac_1;				// ble mac1 Ԥ��
	TCString wifimac;				// wifi mac
	TCString cell_x;				// С������
	TCString cell_y;				// С��γ��
	double rscp_;
	double rxlev_;
	int sos_flag_;					// sos flag �����豸��t����
	int electricity_;				// ����
	int useralarmstatus;			// pad app
	TCString time_stamp;			// ʱ���
	float speed;					// �����ٶ�
	TCString vender;				// ����
	TCString imsi;					// �ֻ�imsi
	TCString phone_no;				// �ֻ�����
	TCString ble_z;					// ble �߶�
	TCString reserve_z_1;			// ����

	void clear()
	{
		unique_id_ = "";
		longitude_ = "";
		latitude_ = "";
		elevation_ = 0.0;
		reserve_x_1 = "";
		reserve_y_1 = "";
		ble_x = "";
		ble_y = "";
		cellid_ = 0;
		lac_ = 0;
		mobile_ = "";
		blemac_ = "";
		blemac_1 = "";
		wifimac = "";
		cell_x = "";
		cell_y = "";
		rscp_ = 0.0;
		rxlev_ = 0.0;
		sos_flag_ = 0;
		electricity_ = 100;
		useralarmstatus = 0;
		time_stamp = "";
		speed = 0.0;
		vender = "";
		imsi = "";
		phone_no = "";
		ble_z = "";
		reserve_z_1 = "";
	}
	std::string putcsv()
	{
		std::string str(unique_id_);
		str += ',';
		str += longitude_;
		str += ',';
		str += latitude_;
		str += ',';
		str += FloatToStr(elevation_, 6);
		str += ',';
		str += ble_x;
		str += ',';
		str += ble_y;
		str += ',';
		str += reserve_x_1;
		str += ',';
		str += reserve_y_1;
		str += ',';
		str += IntToStr(cellid_);
		str += ',';
		str += IntToStr(lac_);
		str += ',';
		str += mobile_;
		str += ',';
		str += blemac_;
		str += ',';
		str += blemac_1;
		str += ',';
		str += wifimac;
		str += ',';
		str += cell_x;
		str += ',';
		str += cell_y;
		str += ',';
		str += FloatToStr(rscp_, 6);
		str += ',';
		str += FloatToStr(rxlev_, 6);
		str += ',';
		str += IntToStr(sos_flag_);
		str += ',';
		str += IntToStr(electricity_);
		str += ',';
		str += IntToStr(useralarmstatus);
		str += ',';
		str += time_stamp;
		str += ',';
		str += FloatToStr(speed, 1);
		str += ',';
		str += vender;
		str += ',';
		str += imsi;
		str += ',';
		str += phone_no;
		str += ',';
		str += ble_z;
		str += ',';
		str += reserve_z_1;

		return str;
	}
};

struct str_LBS_Location_Node {
	int nLac;
	int nCellID;
	int nRxLev;
	void clear(){
		nLac=0;
		nCellID=0;
		nRxLev=0;
	};
	
};

struct str_Lac_Cell_Node {
	int nLac;
	int nCellID;
	void clear(){
		nLac=0;
		nCellID=0;
	};
	bool operator < ( const str_Lac_Cell_Node sKey) const{
    /* ����key���붼ƥ������� */
    return ( (nLac < sKey.nLac)  || \
    	( (nLac == sKey.nLac) && (nCellID < sKey.nCellID) ) );
  };
};

#define LOG_DEBUGV2(fmt, ...) \
    do \
    { \
        printf("DEBUG-->" fmt"\n", ##__VA_ARGS__); \
    }while(0);

#define LOG_INFO(fmt, ...) \
    do \
    { \
        char arrLog[512] = {0}; \
        sprintf(arrLog, fmt " [%s,%s(),line %d]", ##__VA_ARGS__, __FILE__,__FUNCTION__,__LINE__); \
        LOG_DEBUGV2("%s", arrLog); \
        TCDataCollectionLog  *g_TCDataCollectionLog; \
        g_TCDataCollectionLog->AddDataCollectionRunLog(arrLog);\
    }while(0);

#define LOG_WRITE(fmt, ...) \
    do \
    { \
        char arrLog[512] = {0}; \
        sprintf(arrLog, fmt " ", ##__VA_ARGS__); \
        LOG_DEBUGV2("%s", arrLog); \
        TCDataCollectionLog  *g_TCDataCollectionLog; \
        g_TCDataCollectionLog->AddDataCollectionRunLog(arrLog);\
    }while(0);

#define __ENTERFUNCTION;  try{

#define __LEAVEFUNCTION; \
} \
	catch (otl_exception& e) \
{ \
	LOG_INFO("SQLִ���쳣\n\tmsg=[%s]\n\tSQL=[%s]\n\tvar_info=[%s]", e.msg, e.stm_text, e.var_info); \
} \
	catch (TCException &e) \
{ \
	LOG_INFO("Error:%s", (char*)e.GetExceptionMessage()); \
} \
	catch (...) \
{ \
	LOG_INFO("UnKnown Error"); \
}
#endif


