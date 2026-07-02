/*******************************************************************************
** Copyright (c) 2024 MAK Technologies
** All rights reserved.
*******************************************************************************/



// VR-Engage Includes
#include "vreUtil/logger.h"
#include <vreUtil/version.h>

// VR-Forces Includes
#include <vrfcgf/vrfPluginExtension.h>
#include <vrfcgf/cgf.h>
#include <vrfcgf/factoryManager.h>


#include "TerrainSlope.h"
#include "TerrainSoil.h"



//Message 전송 테스트
//#include <vreMessageManager/vreMessageManager.h>
//#include "utilities/vreMessageManager/vreMessageFactory.h"
//#include <vrfutil/consoleCommandManager.h>
//#include "Nesslab/CustomMessage/generated/StopAllPluginsMessage.h"
//#include "SimCommmands.h"
//#include <vreMessageManager/forwardMessage.h>
//#include "CustomMessage.h"

extern "C" {


	void DT_VRF_DLL_PLUGIN DtPluginInformation(DtVrfPluginInformation& info)
	{
		info.pluginName = "nesslabTerrainSurfaceSimPlugin";
		info.pluginVersion = DtVreVersionNumber;
		info.pluginCreator = "Nesslab";
	}

	DT_VRF_DLL_PLUGIN bool DtInitializeVrfPlugin(DtCgf* cgf)
	{

		//지형기울기(BE)
		cgf->factoryManager()->componentFactory()->addCreatorFcn(nesslab_backend_plugins::TERRAIN_SLOPE_PLUGIN_TYPE, nesslab_backend_plugins::TerrainSlopePlugin::creator);
		
		//지형타입(BE)
		cgf->factoryManager()->componentFactory()->addCreatorFcn(nesslab_backend_plugins::TERRAIN_SOIL_PLUGIN_TYPE, nesslab_backend_plugins::TerrainSoilPlugin::creator);


		return true;
	}



	// Post-initialization (optional)
	DT_VRF_DLL_PLUGIN bool DtPostInitializeVrfPlugin(DtCgf* cgf)
	{


		//커맨드 등록
		/*
		
		사용방법
		-> commandString + UUID 조합으로 호출

		std::string customCommand = "CustomMessage " + entity()->uuid().uuidString();
		std::cout << "customCommand: " << customCommand << std::endl;
		DtConsoleCommandManager::globalConsoleCommandManager()->runCommand(customCommand);

		DtConsoleCommandManager::globalConsoleCommandManager()->addCommand("CustomMessage",
		new DtCustomMessageCommand(cgf->simObjectManager()));

		*/
		


		return true;
	}


	// Cleanup on unload (optional)
	DT_VRF_DLL_PLUGIN void DtUnloadVrfPlugin()
	{
		// Release resources
		//DtConsoleCommandManager::globalConsoleCommandManager()->removeCommand("CustomMessage");
	}


}
