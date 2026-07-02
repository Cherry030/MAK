/*******************************************************************************
** Copyright (c) 2018 MAK Technologies, Inc.
** All rights reserved.
*******************************************************************************/
#pragma once
using namespace std;
//#if DtHLA
#include <thread>

#include <vl/exerciseConn.h>
#include "MyNetType.h"

//Fom Interaction Class
#include "extLogInInfo.h"

namespace rfid_to_vris_packets {
	struct TraineeInfoPacket;
}

namespace RFID
{
	std::string  GetString_DtNetLogInInfo10(DtNetLogInInfo10 info);
	DtNetLogInInfo10  GetDtNetLogInInfo10_String(const std::string& str);
	void PrintLoginInfo(extLogInInfo inter);

	void loginFom(DtExerciseConn* exConn, bool runExtPropInit);
	void unloginFom(DtExerciseConn* exConn);

	//void SendTraineeSensor(unsigned short usPublisherType, unsigned short usDeviceID, byte heartbeat, byte concentration, byte stress, byte fatigue);
	//void SendLoginInfo(unsigned short usPublisherType, unsigned short usDeviceID, string strServiceNumber, string strLoginName, string strAffiliation);
	void SendLoginInfo(unsigned short usPublisherType, unsigned short usDeviceID, DtNetLogInInfo10 aryServiceNumber, DtNetLogInInfo10 aryLoginName, DtNetLogInInfo10 aryAffiliation);

	//void ProcessIncomingData();

	void initPlugin();

	//Callback Function
	void OnDataReceived(const string& clientAddr, const uint8_t* data, int len);
	void OnDsmsConnected();
	void OnDsmsDisconnected();
	void OnTraineeInfoPacket(const rfid_to_vris_packets::TraineeInfoPacket& packet);

	void HandleDsmsReconnectThr();
	void SendDataToVRF();
	void SendDataToDsms();

	std::wstring Cp949ToUnicode(const std::string& cp949Str);
}
//#endif
