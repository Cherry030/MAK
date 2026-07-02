/******************************************************************************
** Copyright (c) 2024 MAK Technologies
** All rights reserved.
******************************************************************************/
//#define Accessory_ON

#include "deviceStatePlugin.h"

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

#include "DevAccessory.h"

using namespace makVrv;
using namespace makVrv::DT_PROTOCOL_NAMESPACE;

#undef DT_PROTOCOL_NAMESPACE
/////////////////////////////////////////////////////////////
#endif

#define DL_DLL_IGCONVRLINK DT_DLL_EXAMPLEACCESSORYTEMPLATE
#define DT_PROTOCOL_NAMESPACE vrvHla1516e
#include "DevAccessory.h"
#undef DT_PROTOCOL_NAMESPACE

using namespace makVrv::vrvHla1516e;

using namespace makVre;
using namespace makVrv;

namespace nesslab_frontend_plugins
{
    DevAccessory* accessory = nullptr;
    //CTOR
    DtDeviceStatePlugin::DtDeviceStatePlugin()
        :DtPlayerComponent()
    {
    }

    //DTOR
    DtDeviceStatePlugin::~DtDeviceStatePlugin()
    {

    }
    bool DtDeviceStatePlugin::initialize(DtPlayerStation* player, DtInitTable& config)
    {
        if (!DtPlayerComponent::initialize(player, config))
        {
            return false;
        }

#ifdef Accessory_ON
        auto* accessory = new DstAccessory(*myDe);
        DtAccessoryManager::instance(*myDe).addAccessory(accessory);
        
        DtDriverManager::Drivers& drivers = myDe->driverManager().drivers();

        for (auto* drv : drivers)
        {
            std::cout << "[ INFO ] Driver Instance Name : " << drv->instanceName() << std::endl;

            if(drv->instanceName() == "HLA 1516 Evolved")
                accessory->install(drv);
        }
#endif
        accessory = DevAccessory::instance();

        myTreadmillTypeHandle = playerAttributeStore()["treadmill-type"].as<int>();
        myTreadmillStateHandle = playerAttributeStore()["treadmill"].as<int>();
        myManipulatorStateHandle = playerAttributeStore()["manipulator"].as<int>();

        myHelmetAttributeHandle = playerAttributeStore()["helmet-patch"].as<int>();
        myBodyAttributeHandle = playerAttributeStore()["body-patch"].as<int>();
        myHandAttributeHandle = playerAttributeStore()["hand-patch"].as<int>();

        return true;
    }

    void DtDeviceStatePlugin::shutdown()
    {
        DtPlayerComponent::shutdown();
    }

    const char* DtDeviceStatePlugin::type() const
    {
        return DtDeviceStatePluginType;
    }

    bool DtDeviceStatePlugin::postInitialize()
    {
        if (!DtPlayerComponent::postInitialize())
        {
            return false;
        }

        
        myHelmetPatch = 0x00;
        myBodyPatch = 0x00;
        myHandPatch = 0x00;

        myAttributeCallbacks.connect(myTreadmillTypeHandle, this, &DtDeviceStatePlugin::OnTreadmillTypeChanged);
        myAttributeCallbacks.connect(myTreadmillStateHandle, this, &DtDeviceStatePlugin::OnTreadmillStateChanged);
        myAttributeCallbacks.connect(myManipulatorStateHandle, this, &DtDeviceStatePlugin::OnManipulatorStateChanged);

        myAttributeCallbacks.connect(myHelmetAttributeHandle, this, &DtDeviceStatePlugin::OnHelmetStateChanged);
        myAttributeCallbacks.connect(myBodyAttributeHandle, this, &DtDeviceStatePlugin::OnBodytStateChanged);
        myAttributeCallbacks.connect(myHandAttributeHandle, this, &DtDeviceStatePlugin::OnHandStateChanged);
        
        return true;
    }

    void DtDeviceStatePlugin::tick(double dt)
    {

    }

    void DtDeviceStatePlugin::OnTreadmillTypeChanged(const int& val)
    {
        myTreadmillType = val;
        //if (auto* accessory = DevAccessory::instance())
            accessory->setTreadmillType(val);
        
        std::cout << "[PlayerStateAttribute][KCL] Treadmill Type Changed : " << static_cast<int> (val) << std::endl;
    }

    void DtDeviceStatePlugin::OnTreadmillStateChanged(const int& val)
    {
        myTreadmillState = val;
        //if (auto* accessory = DevAccessory::instance())
            accessory->setTreadmillState(val);

        std::cout << "[PlayerStateAttribute][KCL] Treadmill State Changed : " << static_cast<int> (val) << std::endl;
    }

    void DtDeviceStatePlugin::OnManipulatorStateChanged(const int& val)
    {
        myManipulatorState = val;
        //if (auto* accessory = DevAccessory::instance())
            accessory->setManipulatorState(val);

        std::cout << "[PlayerStateAttribute][KCL] ManipulatorType State Changed : " << static_cast<int> (val) << std::endl;
    }

    void DtDeviceStatePlugin::OnHelmetStateChanged(const int& state)
    {
        myHelmetPatch = state;
        accessory->setHelmetPatchState(state);
        /*
        if (auto* accessory = DevAccessory::instance())
        {
            accessory->setHelmetPatchState(state);
        }
        */
        std::cout << "[PlayerStateAttribute][KCL] Helmet State Changed : " << static_cast<int> (state) << std::endl;
    }

    void DtDeviceStatePlugin::OnBodytStateChanged(const int& state)
    {
        myBodyPatch = state;
        accessory->setBodyPatchState(state);
        /*
        if (auto* accessory = DevAccessory::instance())
        {
            accessory->setBodyPatchState(state);
        }
        */
        std::cout << "[PlayerStateAttribute][KCL] Body State Changed : " << static_cast<int> (state) << std::endl;

    }

    void DtDeviceStatePlugin::OnHandStateChanged(const int& state)
    {
        myHandPatch = state;
        accessory->setHandPatchState(state);
        /*
        if (auto* accessory = DevAccessory::instance())
        {
            accessory->setHandPatchState(state);
        }
        */
        std::cout << "[PlayerStateAttribute][KCL] Hand State Changed : " << static_cast<int> (state) << std::endl;
    }
}