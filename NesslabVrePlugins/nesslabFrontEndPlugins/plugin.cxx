#pragma once

#include "plugin.h"
#include "vrePlayerStation/vrvPlayerStationDriver.h"
#include "vrePlayerStation/playerStationApp.h"

#include "vreUtil/initializer.h"
#include "vreUtil/logger.h"

#include "vreInput/inputDeviceFactory.h"
#include <string>

//Nesslab FrontEnd Plugins
#include "RFIDPlugin.h"
#include "WeaponPlugin.h"
#include "ActionPlugin.h"
#include "WeaponPosePlugin.h"
#include "ScreenWindowPlugin.h"
#include "CustomHumanControlLogic.h"


using namespace makVre;
using namespace nesslab_frontend_plugins;

extern "C"
{
	bool initPlayerStationModule(makVre::DtPlayerStationApp* app)
	{
		
		//log that the plugin was loaded using the "nesslabFrontEndPlugins" string to identify a channel
		LOG_VERBOSE("Nesslab") << "Initializing front-end Plugins" << std::endl;

		//엔진시작할때 실행(FE)
		//app->inputDeviceFactory()->addCreator<RFIDPlugin>("DtRFID");//RFID

		//Engage누르면 실행(FE)
		//register a new logic in the app logic factory
		makVre::DtComponentFactory& factory = app->componentFactory();
		factory.addCreator<CustomHumanControlLogic>(customHumanCtlPluginType);//이동 중 수류탄 투척 + 매니퓰레이터
		factory.addCreator<ScreenWindowPlugin>(screenPluginType);//스크린
		factory.addCreator<WeaponPosePlugin>("DtNesslabWeaponPoseLogic");//총기 자세
		factory.addCreator<WeaponPlugin>(weaponPluginType);//모의총기
		factory.addCreator<ActionPlugin>(actionPluginType);//훈련자 행동 인식

		return true;
	}
}
