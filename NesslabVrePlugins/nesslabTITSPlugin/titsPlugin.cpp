/******************************************************************************
** Copyright (c) 2024 MAK Technologies
** All rights reserved.
******************************************************************************/
#define Accessory_ON

#include "titsPlugin.h"

#include "vrePlayerStation/playerStation.h"
#include "vrePlayerStation/playerStationApp.h"
#include "vrePlayerStation/playerStationTempNotification.h"

#include "vreUtil/initializer.h"
#include "vreUtil/logger.h"

#include <vlutil/vlUtil.h>

#ifdef Accessory_ON
//RTI/////////////////////////////////////////////////////////////
#include <vrvCore/DtDe.h>
#include <vrvCore/DtAccessoryManager.h>
#include <vrvCore/DtDriverManager.h>
#include <vrvUtil/signalslib.h>
#include <boost/bind/bind.hpp>

#define DL_DLL_IGCONVRLINK DT_DLL_EXAMPLEACCESSORYTEMPLATE
#define DT_PROTOCOL_NAMESPACE vrvHla1516e

#include "MyAccessory.h"

using namespace makVrv;
using namespace makVrv::DT_PROTOCOL_NAMESPACE;

#undef DT_PROTOCOL_NAMESPACE
/////////////////////////////////////////////////////////////
#endif

using namespace makVre;
using namespace makVrv;
namespace nesslab_frontend_plugins
{
	MyAccessory* accessory = nullptr;

	TITSPlugin::TITSPlugin()
		:DtPlayerComponent()
	{
		std::cout << "[TITSPlugin][Trace] Constructor " << std::endl;
	}


	TITSPlugin::~TITSPlugin()
	{
		std::cout << "[TITSPlugin][Trace] Destructor " << std::endl;
	}


	bool TITSPlugin::initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config)
	{
		std::cout << "[TITSPlugin][Trace] initialize \n ";
		//do base class init.  Shouldn't fail
		if (!DtPlayerComponent::initialize(player, config))
		{
			return false;
		}


#ifdef Accessory_ON
		accessory = new MyAccessory(*myDe);
		DtAccessoryManager::instance(*myDe).addAccessory(accessory);

		DtDriverManager::Drivers& drivers = myDe->driverManager().drivers();

		for (auto* drv : drivers)
		{
			std::cout << "[ INFO ] Driver Instance Name : " << drv->instanceName() << std::endl;

			if (drv->instanceName() == "HLA 1516 Evolved")
				accessory->install(drv);
		}
#endif

		accessory->traineeFom->setPluginClass(this);

		return true;
	}

	bool TITSPlugin::postInitialize()
	{
		std::cout << "[TITSPlugin][Trace] postInitialize \n ";

		helmetAttrName = "helmet-patch";
		bodyAttrName = "body-patch";
		handAttrName = "hand-patch";
		return DtPlayerComponent::postInitialize();
	}

	void TITSPlugin::setHelmetPatchState(uint8_t state)
	{
		playerAttributeStore()->setAttribute("helmet-patch", (int)state);
	}

	void TITSPlugin::setBodyPatchState(uint8_t state)
	{
		playerAttributeStore()->setAttribute("body-patch", (int)state);
	}

	void TITSPlugin::setHandPatchState(uint8_t state)
	{
		playerAttributeStore()->setAttribute("hand-patch", (int)state);
	}



	void TITSPlugin::tick(double dt)
	{
	}

	void TITSPlugin::shutdown()
	{
		std::cout << "[TITSPlugin][Trace] shutdown " << std::endl;
	}
	const char* TITSPlugin::type() const
	{
		return titsPluginType;
	}

}