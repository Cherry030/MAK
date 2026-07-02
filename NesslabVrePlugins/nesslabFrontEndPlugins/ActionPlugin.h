#pragma once

#include "vrePlayerStation/playerComponent.h"
#include "vrePlayerStation/playerStation.h"
#include "roles/human/vreHumanFrontend/humanControlLogic.h"

#include "utilities/vreMessageManager/vreMessageFactory.h"
#include <vreMessageManager/vreMessageManager.h>
#include <vreMessageManager/vreMessage.h>
#include <vreMessageManager/forwardMessage.h>

#include "Nesslab/nesslabCommon.h"
#include "Nesslab/StopAllPluginsMessage.h"

/*
			연세대 행동 인식 플러그인(FrontEnd)
- 연세대에서 행동라벨 받아서 VRIS에서 맞는 행동라벨 실행(+ 수류탄투척)


*/



//Forward declaration
namespace ysu_to_vris_packets {
	struct ActionLabelPacket;
	struct DeviceStatusPacket;
}


namespace nesslab_frontend_plugins
{
	// Component type string used for registration and in role configuration.
	constexpr auto actionPluginType = "DtAction";

	class ActionPlugin : public makVre::DtPlayerComponent
	{

	public:
		ActionPlugin();
		~ActionPlugin() override;

		//Base class abstract functions need to be defined
		bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config) override;
		bool postInitialize() override;
		void tick(double dt) override;
		void shutdown() override;
		const char* type() const override;
		//const std::string& type() override;


		void HandleActionLabel(int labelIndex);

	private:

		void InitPlugin();

		void SetWeaponState(nesslab_common::WeaponStateType index);
		void SetPosture(nesslab_common::HumanPostureType index);
		void SetWeaponIndex(unsigned int index);


		//Callback Function
		void OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len);
		void OnActionLabelPkt(const ysu_to_vris_packets::ActionLabelPacket& packet);
		void OnDeviceStatusPkt(const ysu_to_vris_packets::DeviceStatusPacket& packet);


		makVre::DtHumanControlLogic* humanControLogic = nullptr;
		int lastActionLabel = -1;
		std::thread netThread;

		int grenadeWeaponIndex = -1;

		//void SendVreMessage();

	};
}

/*
		//void HandleActionDisconnectThr();
		//void HandleDsmsDisconnectedThr();
		//void SendDataToDsms(uint8_t* data, const int len);

		//void OnDeviceStatusPkt(const ysu_to_vris_packets::DeviceStatusPacket& packet);
		//void OnActionDisconnected();
		//void OnDsmsDisconnected();
*/