#pragma once

#include "vreHumanFrontend/humanWeaponPoseLogic.h"
//#include "vrePlayerStation/playerStationAttributeStore.h"
#include "vrePlayerStation/playerStation.h"
#include "vrvCore/DtObserver.h"
#include "vrvCore/DtDeSharedState.h"//=>coordinateSystem
#include "vrvUtil/DtCoordinateSystem.h"//=>localToNetPos
#include "vrvCore/DtObserverObject.h"
#include "vrvCore/DtObserverObjectAgent.hpp"
#include "terrainCS/coordSystem.h"
#include "matrix/topoCoord.h"



struct WeaponPoseDataPacket;

namespace nesslab_frontend_plugins {

	class WeaponPosePlugin : public DtHumanWeaponPoseLogic
	{
	public:
		WeaponPosePlugin();
		virtual ~WeaponPosePlugin();

		//Base class abstract functions need to be defined
		virtual bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config);
		virtual void shutdown();
		virtual void tick(double dt);


	private:

		void InitPlugin();

		//Callback Function
		void OnDataReceived(unsigned char* data, int len);
		void OnWeaponPosePkt(const WeaponPoseDataPacket& packet);


		//무기 조준점 원점 고정 옵저버
		makVrv::DtObserverObject* weaponAimOriginObs = nullptr;

		//실제 조준처리 옵저버
		makVrv::DtObserverObject* weaponAimObs = nullptr;

		bool isObserverReady = false;
		std::thread netThread;

		//bool isMotiveConnected = false;

	};

}