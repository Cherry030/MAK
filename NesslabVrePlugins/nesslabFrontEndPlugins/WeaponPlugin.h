#pragma once
#include "export.h"
#include "vrePlayerStation/playerComponent.h"
#include "roles/human/vreHumanFrontend/humanControlLogic.h"
#include <vector>

#include "Nesslab/nesslabCommon.h"
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab/nesslabTcpClient.h"



//Function declaration
namespace weapon_to_vris_packets
{
	struct FireInfoDataPacket;
}


namespace nesslab_frontend_plugins {

	constexpr const char* weaponPluginType = "DtWeapon";

	
	class WeaponPlugin : public makVre::DtPlayerComponent
	{
	public:

		WeaponPlugin();
		~WeaponPlugin() override;

		//Base class abstract functions need to be defined
		bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config) override;
		void tick(double dt) override;
		void shutdown() override;
		bool postInitialize() override;

		const char* type() const override;

	private:

		bool InitPlugin();
		void SendDataToDsms();

		//Callback Function
		void OnWeaponIndexChanged(const int& index);
		void OnTotalRemainingAmmo(const double& totalAmmo);
		void OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len);
		void OnWeaponFire(const weapon_to_vris_packets::FireInfoDataPacket& p);
		void OnDsmsDisconnected();
		void HandleDsmsDisconnected();
		void HandleWeaponFire();


		//현재 탄창에 남은 총알 개수
		int GetCurrentAmmoInMag(int weaponIndex);
		//탄창에 보관할 수 있는 최대 총알 수
		int GetMaxAmmoInMag(int weaponIndex);
		//전체 남은총알 개수
		int GetTotalRemainingAmmo(int weaponIndex);


		//Indicates whether the weapon has been fired.
		bool weaponFired = false;
		bool pluginInitialized = false;

		//현재 탄창개수(재장전여부확인용), 무기변경시 변경
		uint8_t	currentMagCount = 0;
		uint8_t currentWeaponIndex = 0;

		bool TryRegisterWeaponInfo();
		bool TryRegisterWeaponCallbacks();


		makVre::DtHumanControlLogic* humanControLogic = nullptr;
		std::thread netThread;


	};

}