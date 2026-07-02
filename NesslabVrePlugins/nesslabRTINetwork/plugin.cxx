/*******************************************************************************
** Copyright (c) 2020 MAK Technologies
** All rights reserved.
*******************************************************************************/

#include "rtiNetworkPlugin.h"

#include <vreUtil/version.h>
#include <vrfcgf/vrfPluginExtension.h>
#include <vrfcgf/cgf.h>
#include <vrfcgf/factoryManager.h>
#include <vrfcgf/vrfCreator.h>
#include <vrfobjcore/vrfProcessStateRepositoryFactory.h>

using namespace nesslab_backend_plugins;

extern "C" {

   DT_VRF_DLL_PLUGIN void DtPluginInformation(DtVrfPluginInformation& info)
   {
      info.pluginName = "nesslab Handpose Plugin";
      info.pluginVersion = DtVreVersionNumber;
      info.pluginCreator = "Nesslab";
   }

   DT_VRF_DLL_PLUGIN bool DtInitializeVrfPlugin(const DtCgf* cgf)
   {
       DtFactoryManager* factoryManager = cgf->factoryManager();

       factoryManager->componentFactory()->addCreatorFcn(RTI_NETWORK_PLUGIN_TYPE, RTINetworkPlugin::creator);

       std::cout << "[ INFO ] RTI Network Plugin Loading ..." << std::endl;
       return true;
   }


   DT_VRF_DLL_PLUGIN bool DtPostInitializeVrfPlugin(DtCgf* cgf)
   {
      return true;
   }

   DT_VRF_DLL_PLUGIN void DtUnloadVrfPlugin()
   {
   }

}