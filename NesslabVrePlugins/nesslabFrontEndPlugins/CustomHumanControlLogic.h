#pragma once
#include "export.h"
#include "vrePlayerStation/playerComponent.h"
#include "vrePlayerStation/playerStation.h"
#include "roles/human/vreHumanFrontend/humanControlLogic.h"
//#include "vreMessageManager/vreMessageManager.h"
#include <vreMessageManager/vreMessageManager.h>
#include <vreMessageManager/vreMessage.h>


struct TDMDataPacket;

/*
			HumanControlLogic(테스트 진행 중)
- 캐릭터 이동하면서 수류탄 투척 기능
- 캐릭터 회전 기능(매니퓰레이터 회전)
- 리소스 업데이트 딜레이 엔진 버그(VRE 2.2c 해결)
- 플러그인 정지 트리거를 담은 VreMessage 수신
(트레드밀 --TCP--> BackEndPlugin --VreMessage--> CustomHumanControllLogic --> FrontEnd Plugins)

*/





namespace nesslab_frontend_plugins {


	constexpr auto customHumanCtlPluginType = "DtCustomHumanControlLogic";

	class CustomHumanControlLogic : public makVre::DtHumanControlLogic
	{

	public:
		CustomHumanControlLogic();
		~CustomHumanControlLogic() override;

		//Base class abstract functions need to be defined
		bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config) override;
		void shutdown() override;
		void tick(double dt) override;
		const char* type() const override;

	protected:
		virtual void throwGrenade() override;


		void InitPlugin();

		float ConvertManipAngle(float oldAngle);

		//Callback Function
		void OnDataReceived(unsigned char* data, int len);
		void OnTdmDataPkt(const TDMDataPacket& packet);


		std::thread netThread;

		//테스트 지워주기
		int tdmTickCount = 0;


		void SendDataToSim();


		//! Handles incoming CustomMessage from the simulation backend.
		makVre::DtVreMessageResult HandleStopPluginMessage(makVre::DtVreMessage* msg);

	};
}