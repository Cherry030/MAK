/******************************************************************************
** Copyright (c) 2024 MAK Technologies
** All rights reserved.
******************************************************************************/
#define Accessory_ON
#include "rfidPlugin.h"

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

/*
//Nesslab
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab/nesslabTcpClient.h"
#include "Nesslab/nesslabCommon.h"

//Common
using namespace makVre;
using namespace makVrv;
using namespace nesslab_common;


static vrvSignalsLib::connection postInitializeConnection;
*/
using namespace makVre;
using namespace makVrv;
namespace nesslab_frontend_plugins
{

    //CTOR
    DtRFIDPlugin::DtRFIDPlugin()
        :DtPlayerComponent()
    {
    }

    //DTOR
    DtRFIDPlugin::~DtRFIDPlugin()
    {

    }

    bool DtRFIDPlugin::initialize(DtPlayerStation* player, DtInitTable& config)
    {
        if (!DtPlayerComponent::initialize(player, config))
        {
            return false;
        }

#ifdef Accessory_ON
        auto* accessory = new MyAccessory(*myDe);
        DtAccessoryManager::instance(*myDe).addAccessory(accessory);
        
        DtDriverManager::Drivers& drivers = myDe->driverManager().drivers();

        for (auto* drv : drivers)
        {
            std::cout << "[ INFO ] Driver Instance Name : " << drv->instanceName() << std::endl;

            if(drv->instanceName() == "HLA 1516 Evolved")
                accessory->install(drv);
        }
#endif

        return true;
    }

    void DtRFIDPlugin::shutdown()
    {
        DtPlayerComponent::shutdown();
    }

    const char* DtRFIDPlugin::type() const
    {
        return DtRFIDPluginType;
    }

    bool DtRFIDPlugin::postInitialize()
    {
        if (!DtPlayerComponent::postInitialize())
        {
            return false;
        }

        //init(*myDe);
        return true;
    }

    void DtRFIDPlugin::tick(double dt)
    {

    }
}