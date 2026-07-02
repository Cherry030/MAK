/*******************************************************************************
** Copyright (c) 2018 MAK Technologies, Inc.
** All rights reserved.
*******************************************************************************/
#pragma once
using namespace std;
//#if DtHLA

#include <vl/exerciseConn.h>
#include "MyNetType.h"

//Fom Interaction Class
#include "extTraineeSensor.h"
//#include "extLogInInfo.h"
#include "extTrainingCtrl.h"



std::string GetString_Publisher(unsigned short type);
std::string GetString_DeviceStatus(unsigned char state);
std::string  GetString_DtNetLogInInfo10(DtNetLogInInfo10 info);
//DtNetLogInInfo10  GetDtNetLogInInfo10_String(const std::string& str);

void configFomMapper(DtExerciseConn* exConn, bool runExtPropInit);
void unconfigFomMapper(DtExerciseConn* exConn);

void SendTraineeSensor(unsigned short usPublisherType, unsigned short usDeviceID, byte heartbeat, byte concentration, byte stress, byte fatigue);
//void SendLoginInfo(unsigned short usPublisherType, unsigned short usDeviceID, string strServiceNumber, string strLoginName, string strAffiliation);

void ProcessIncomingData();
//#endif
