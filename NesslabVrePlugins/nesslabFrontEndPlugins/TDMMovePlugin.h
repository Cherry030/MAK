//
//#pragma once
//
//#include "export.h"
//#include "vrePlayerStation/playerComponent.h"
//#include "vrePlayerStation/playerStation.h"
//#include "roles/human/vreHumanFrontend/humanControlLogic.h"
//#include<thread>
//
//struct TDMDataPacket;
//
//namespace nesslab_frontend_plugins {
//	class  TDMMovePlugin : public makVre::DtPlayerComponent
//	{
//	public:
//		TDMMovePlugin();
//		virtual ~TDMMovePlugin();
//
//		//Base class abstract functions need to be defined
//		virtual bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config);
//		virtual void tick(double dt);
//		virtual void shutdown();
//		
//	private:
//		void InitPlugin();
//		void SendDataToDsms();
//
//		void PlayerMove(double val);
//		void PlayerStrafe(double val);
//
//		void SetCharacterMovement(float headingDeg, const float& worldSpeedXKmh, const float& worldSpeedYKmh);
//
//		//Callback Function
//		void OnDataReceived(unsigned char* data, int len);
//		void OnTdmDataPkt(const TDMDataPacket& packet);
//
//		makVre::DtHumanControlLogic* humanControLogic = nullptr;
//		std::thread netThread;
//
//		//테스트 지워주기
//		int tdmTickCount = 0;
//	};
//}
