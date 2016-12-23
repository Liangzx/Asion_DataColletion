/*****************************************************************************
CAPTION
Project Name: 乌东德定位系统--数据采集模块
Description : 通用结构体定义
File Name   : c_DataCollection_Public.cpp
Requires    : LinuxAS3 , Oracle 9i
Create Date : 2016-05-04
******************************************************************************/

#include "c_DataCollection_Public.h"

bool  AffectDegreeMin(const str_AFP_SuggestFreq_Evaluate &s1,const str_AFP_SuggestFreq_Evaluate &s2){
        return ((int)s1.nAffect_Degrese < (int)s2.nAffect_Degrese);
};

bool  DistinceMin(const str_Disturb_Affect_Model &s1,const str_Disturb_Affect_Model &s2){
        return (s1.fDistance < s2.fDistance);
};


bool  CellIDMin(const int &s1,const int &s2){
        return (s1 < s2);
};

