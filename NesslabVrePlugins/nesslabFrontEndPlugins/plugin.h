
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
	NESSLAB_FRONTEND_DLL bool initPlayerStationModule(makVre::DtPlayerStationApp* app);
};
