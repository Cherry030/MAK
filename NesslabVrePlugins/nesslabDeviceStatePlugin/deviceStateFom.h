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
#include "extDeviceState.h"



namespace sub_to_vris_packets {
	struct InfraPostureStatePacket;
	struct SemiInfraPostureStatePacket;
	struct GunStatePacket;
	struct HapticStatePacket;
	struct ManeuverStatePacket;
	struct PatchStatePacket;
}

namespace vris_to_dsms_packets {
	struct DeviceStatePacket;
}


namespace DEV
{
	class DevStateFom
	{
	public:
		class DeviceStateInfo
		{
		public:
			uint8_t treadmillType;
			uint8_t treadmillState;
			uint8_t manipulatorType;
			uint8_t manipulatorState;
			uint8_t postureType = 0x00;
			uint8_t postureState = 0x99;

			uint8_t gunState = 0x99;
			uint8_t hapticState = 0x99;
			uint8_t patchState[3] = { 0x99, 0x99, 0x99 };
			uint8_t maneuverState = 0x99;

			DeviceStateInfo(uint8_t tType, uint8_t treadmill, uint8_t manipulator, uint8_t patch[3])
			{
				treadmillType = tType;
				treadmillState = treadmill;
				manipulatorState = manipulator;

				std::copy(patch, patch + 3, patchState);
			}
		};

		static const uint8_t NotConnected = 0x00;
		static const uint8_t Normal = 0x01;
		static const uint8_t Error = 0x02;

		//static const std::vector<uint8_t> PostureNormalState = {
		const std::vector<uint8_t> PostureNormalState = {
			0x01, 0x02, 0x06, 0x07, 0x0B, 0x0C
		};

		const std::vector<uint8_t> PostureErrorState = {
			0x00, 0x03, 0x04,							//MASTER
			0x05, 0x08, 0x09, 0x0F, 0x10, 0x11, 0x12,	//Client1
			0x0A, 0x0D, 0x0E, 0x13, 0x14, 0x15, 0x16	//Clinet2
		};

		const std::vector<uint8_t> IsPostureMasterState = {
		0x00, 0x01, 0x02, 0x03, 0x04
		};

		const std::vector<uint8_t> IsPostureClient1State = {
			0x05, 0x06, 0x07, 0x08, 0x09,
			0x0F, 0x10, 0x11, 0x12
		};

		const std::vector<uint8_t> IsPostureClient2State = {
			0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
			0x13, 0x14, 0x15, 0x16
		};


		void PrintDeviceState(extDeviceState inter);

		void deviceStateFom(DtExerciseConn* exConn, bool runExtPropInit);
		void undeviceStateFom(DtExerciseConn* exConn);

		void initPlugin();



		//Callback Function
		void OnDataReceived(const string& clientAddr, const uint8_t* data, int len);
		void OnDsmsConnected();
		void OnDsmsDisconnected();
		void OnDeviceStatePacket(const sub_to_vris_packets::InfraPostureStatePacket& packet);
		void OnDeviceStatePacket(const sub_to_vris_packets::SemiInfraPostureStatePacket& packet);
		void OnDeviceStatePacket(const sub_to_vris_packets::GunStatePacket& packet);
		void OnDeviceStatePacket(const sub_to_vris_packets::HapticStatePacket& packet);
		void OnDeviceStatePacket(const sub_to_vris_packets::ManeuverStatePacket& packet);
		void OnDeviceStatePacket(const sub_to_vris_packets::PatchStatePacket& packet);

		//Handler
		void HandleSendDeviceStateThr();
		void HandleDsmsReconnectThr();

		void SetDeviceStateInfo(BYTE type, std::vector<BYTE> state);
		uint8_t ResolveValue(uint8_t prev, uint8_t current);
		void SetSendDataPacket();

		void SendDataToVRF();
		void SendDataToDsms();

		std::wstring Cp949ToUnicode(const std::string& cp949Str);

		void SendDeviceState(unsigned short usPublisherType, unsigned short usDeviceID, byte bTreadmill, byte bmanipulator, byte bPosture, byte bGun, byte bHaptic, byte bPatch, byte bManeuver);

		void setTreadmillType(byte val);
		void setTreadmillState(byte val);
		void setManipulatorState(byte val);

		void setHelmetPatchState(byte helmet);
		void setBodyPatchState(byte body);
		void setHandPatchState(byte hand);

		uint8_t latestTreadmillType;
		uint8_t latestTreadmillState;
		uint8_t latestManipulatorState;
		uint8_t latestPatchState[3] = { 0x99, 0x99, 0x99 };
		//DeviceStateInfo* devStateInfo;

		inline uint8_t treadmillVRF(byte treadmill)
		{
			if (treadmill == 0x01 || treadmill == 0x02)   return Normal;
			else if (treadmill == 0x03) return Error;
			else return NotConnected;
		}

		inline uint8_t manipulatorVRF(byte manipulator)
		{
			if (manipulator == 0x01 || manipulator == 0x02)   return Normal;
			else if (manipulator == 0x03) return Error;
			else return NotConnected;
		}

		inline uint8_t postureVRF(uint8_t posture)
		{
			if (posture == 0x99)
				return NotConnected;
			else if (std::find(PostureNormalState.begin(), PostureNormalState.end(), posture) != PostureNormalState.end())
				return Normal;
			else if (std::find(PostureErrorState.begin(), PostureErrorState.end(), posture) != PostureErrorState.end())
				return Error;
			else
				return NotConnected;
		}

		inline uint8_t gunVRF(uint8_t gun)
		{
			if (gun == 0x00)
				return Normal;
			else if (gun == 0x01)
				return Error;
			else
				return NotConnected;
		}

		inline uint8_t hapticVRF(uint8_t haptic)
		{
			if (haptic == 0x00)
				return Normal;
			else if (haptic == 0x01)
				return Error;
			else
				return NotConnected;
		}

		inline uint8_t patchVRF(uint8_t patch[3])
		{
			//모두 연결
			if (patch[0] == 0x01 && patch[1] == 0x01 && patch[2] == 0x01)
				return Normal;
			//하나라도 에러면
			else if (patch[0] == 0x02 || patch[1] == 0x02 || patch[2] == 0x02)
				return Error;
			//하나라도 연결끊김이면
			else
				return NotConnected;
		}

		inline uint8_t manueverVRF(uint8_t maneuver)
		{
			if (maneuver == 0x00)
				return Normal;
			else if (maneuver == 0x01)
				return Error;
			else
				return NotConnected;
		}
	};

}
//#endif

