/*******************************************************************************
** Copyright (c) 2024 MAK Technologies
** All rights reserved.
*******************************************************************************/

#pragma once

#include "export.h"

//forward declare the Player Station App
namespace makVre
{
   class DtPlayerStationApp;
}

//The plugin loading function
extern "C"
{
   RFID_DLL bool initPlayerStationModule(makVre::DtPlayerStationApp* app);
};

// Accessory
namespace makVrv
{
	class DtDe;
}
//RFID_DLL void init(makVrv::DtDe& de);
//void loadAccessory(makVrv::DtDe* de);