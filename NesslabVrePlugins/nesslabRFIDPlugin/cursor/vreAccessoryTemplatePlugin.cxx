/******************************************************************************
** Copyright (c) 2024 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file vreAccessoryTemplatePlugin.cxx
//! \brief Port of exampleAccessoryTemplatePlugin.cxx for VR-Engage (HLA 1516e)

#include "vreAccessoryTemplatePlugin.h"

#include <framework/vrePlayerStation/playerStationApp.h>

#include <vrvCore/DtDe.h>
#include <vrvCore/DtAccessoryManager.h>
#include <vrvUtil/signalslib.h>

#include <boost/bind/bind.hpp>

// Sets DL_DLL_IGCONVRLINK for vrvVrl. Do not use namespace vrvHla1516e (init() name clash).
#include <vrvHla1516e/vrvHla1516e.h>

#define DT_PROTOCOL_NAMESPACE vrvHla1516e
#include "MyAccessory.h"
#undef DT_PROTOCOL_NAMESPACE

using namespace makVrv;

static vrvSignalsLib::connection postInitializeConnection;

void loadAccessory(DtDe* de)
{
    postInitializeConnection.disconnect();
    DtAccessoryManager::instance(*de).addAccessory(new vrvHla1516e::MyAccessory());
    std::cout << "[ INFO ] Load Accessory Plugin" << std::endl;
}

void init(DtDe& de)
{
    DT_DE_INIT_ONCE(de);
    bool mode = de.isInMasterMode();
    std::cout << "[ INFO ] Init Accessory Plugin  Mode : " << mode << std::endl;
    if (mode)
    {
        postInitializeConnection = de.signal_postInitialize.connect(
            boost::bind(&loadAccessory, &de));
    }
}

bool initDeModule(makVrv::DtDe* de)
{
    init(*de);
    return true;
}

extern "C" DT_DLL_VREACCESSORYTEMPLATE bool initPlayerStationModule(makVre::DtPlayerStationApp* app)
{
    if (!app)
    {
        return false;
    }
    return initDeModule(&app->de());
}
