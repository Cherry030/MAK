#pragma once

#include "plugin.h"
#include "vrePlayerStation/vrvPlayerStationDriver.h"
#include "vrePlayerStation/playerStationApp.h"

#include "vreUtil/initializer.h"
#include "vreUtil/logger.h"

#include "vreInput/inputDeviceFactory.h"
#include <string>

//Nesslab FrontEnd Plugins
#include "titsPlugin.h"


using namespace makVre;
using namespace nesslab_frontend_plugins;

extern "C"
{
	bool initPlayerStationModule(makVre::DtPlayerStationApp* app)
	{
		
		//log that the plugin was loaded using the "nesslabFrontEndPlugins" string to identify a channel
		LOG_VERBOSE("Nesslab") << "Initializing front-end Plugins" << std::endl;

		makVre::DtComponentFactory& factory = app->componentFactory();
		factory.addCreator<TITSPlugin>(titsPluginType);//심리분석

		return true;
	}
}
