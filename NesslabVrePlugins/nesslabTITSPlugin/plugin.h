
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
	TITS_DLL bool initPlayerStationModule(makVre::DtPlayerStationApp* app);
};

// Accessory
namespace makVrv
{
	class DtDe;
}
