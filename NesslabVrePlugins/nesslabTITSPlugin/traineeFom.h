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
//>Send
#include "extTraineeSensor.h"
//>Receive
#include "extTrainingCtrl.h"

#include "titsPlugin.h"

//Forward declaration
namespace vris_to_tits_packets {
	struct TraineeInfoPacket;
}
namespace tits_to_vris_packets {
	struct TraineeStatePacket;
	struct PatchStatePacket;
}
namespace vris_to_dsms_packets {
	struct TraineeStatePacket;
}


namespace TITS
{
	class TraineeFom
	{
	public :
		void PrintTraineeSensor(extTraineeSensor inter);

		void traineeFom(DtExerciseConn* exConn, bool runExtPropInit);
		void untraineeFom(DtExerciseConn* exConn);

		void SendTraineeSensor(unsigned short usPublisherType, unsigned short usDeviceID, std::uint8_t heartbeat, std::uint8_t concentration, std::uint8_t stress, std::uint8_t fatigue);

		void initPlugin();

		//Callback Function
		void OnDataReceived(const uint8_t* data, int len);
		void OnTitsConnected();
		void OnTitsDisconnected();
		void OnDsmsConnected();
		void OnDsmsDisconnected();
		void OnTraineeStatePacket(const tits_to_vris_packets::TraineeStatePacket& packet);
		void OnPatchStatePacket(const tits_to_vris_packets::PatchStatePacket& packet);

		

		void HandleTitsReconnectThr();
		void HandleDsmsReconnectThr();
		void SendDataToTITS(BYTE trainState);
		void SendDataToVRF();
		void SendDataToDsms();

		std::wstring Cp949ToUnicode(const std::string& cp949Str);

		void setPatchState(byte helmet, byte body, byte hand);
		/*
		void setHelmetPatchState(byte helmet);
		void setBodyPatchState(byte body);
		void setHandPatchState(byte hand);
		*/
		void setPluginClass(void* usr);
	protected:
		nesslab_frontend_plugins::TITSPlugin* myPlugin = nullptr;
	};

	//callBack 
	void OnTrainingCtrlCB(extTrainingCtrl* inter, void* usr);
}
//#endif
