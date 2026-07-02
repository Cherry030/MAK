#pragma once

#define Accessory_ON
#ifdef Accessory_ON
//Accessory////////////////////////////////////////////////////////
#include <vrvCore/DtDe.h>
#include <vrvCore/DtAccessoryManager.h>
#include <vrvUtil/signalslib.h>
#include <boost/bind/bind.hpp>

#define DL_DLL_IGCONVRLINK DT_DLL_EXAMPLEACCESSORYTEMPLATE
#define DT_PROTOCOL_NAMESPACE vrvHla1516e

#include "DevAccessory.h"

using namespace makVrv;
using namespace makVrv::DT_PROTOCOL_NAMESPACE;
#undef DT_PROTOCOL_NAMESPACE
//////////////////////////////////////////////////////////////////
#endif

#include "plugin.h"
#include "vrePlayerStation/vrvPlayerStationDriver.h"
#include "vrePlayerStation/playerStationApp.h"
#include "vreUtil/initializer.h"
#include "vreUtil/logger.h"
#include "vreInput/inputDeviceFactory.h"

#include "deviceStatePlugin.h"
#include "deviceStateFom.h"

//exampleNotify
bool initPlayerStationModule(makVre::DtPlayerStationApp* app)
{
   //log that the plugin was loaded using the "nesslabFrontEndPlugins" string to identify a channel
   LOG_VERBOSE("nesslabFrontEndPlugins") << "Initializing front-end" << std::endl;

   //엔진시작할때 실행(FE)
   app->componentFactory().addCreator<nesslab_frontend_plugins::DtDeviceStatePlugin>("DtDevState");
   //app->connectorFactory().addCreator<nesslab_frontend_plugins::DtFomNetworkPlugin>("DtDstNetwork");
   //init(app->de());

#ifdef Accessory_ON
   loadAccessory(&(app->de()));
#endif

   return true;
}
#ifdef Accessory_ON
static vrvSignalsLib::connection postInitializeConnection;

void loadAccessory(DtDe* de)
{
    // We're done with the signal, so disconnect from it
    postInitializeConnection.disconnect();

    // Add the accessory to the accessory manager. It will get the option
    // to install itself into any DtDriver objects that are created.
    //accessory = new DevAccessory(*de);
    //DtAccessoryManager::instance(*de).addAccessory(accessory);
    DtAccessoryManager::instance(*de).addAccessory(new DevAccessory(*de));
}

void init(DtDe& de)
{
    // Ensure that init only gets called once
    // (not strictly necessary here, but this is good practice in general)
    DT_DE_INIT_ONCE(de);

    // We only want to create the accessory if we're running in master mode:
    if (de.isInMasterMode())
    {
        // The accessory must be created after the DE's initialization is
        // finished, to make sure all the objects it uses exist.
        postInitializeConnection = de.signal_postInitialize.connect(boost::bind(
            &loadAccessory, &de));
    }
}
#endif