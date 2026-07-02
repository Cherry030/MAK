/*******************************************************************************
** Copyright (c) 2020 MAK Technologies
** All rights reserved.
*******************************************************************************/

#include "HandposePlugin.h"

#include <vreUtil/version.h>
#include <vrfcgf/vrfPluginExtension.h>
#include <vrfcgf/cgf.h>
#include <vrfcgf/factoryManager.h>
#include <vrfcgf/vrfCreator.h>
#include <vrfobjcore/vrfProcessStateRepositoryFactory.h>

using namespace makVre;
using namespace nesslab_backend_plugins;

extern "C" {

   DT_VRF_DLL_PLUGIN void DtPluginInformation(DtVrfPluginInformation& info)
   {
      info.pluginName = "nesslab Handpose Plugin";
      info.pluginVersion = DtVreVersionNumber;
      info.pluginCreator = "Nesslab";
   }
   /*
   DT_VRF_DLL_PLUGIN bool DtInitializeVrfPlugin(DtCgf* cgf)
   {
       cgf->factoryManager()->componentFactory()->addCreatorFcn(HANDPOSE_PLUGIN_TYPE, HandposePlugin::creator);


       return true;
   }
   */
   DT_VRF_DLL_PLUGIN bool DtInitializeVrfPlugin(const DtCgf* cgf)
   {
       // Get the factory manager from the CGF (Common Game Framework) instance
       DtFactoryManager* factoryManager = cgf->factoryManager();

       // Register the human art part actuator component with the component factory.
        // This makes the component available for use in entity definitions and allows
        // VR-Forces to instantiate it when needed.
        //
        // The type string (DtExampleHumanArtPartActuatorType) becomes the identifier
        // used in .entity files to reference this component.
       factoryManager->componentFactory()->addCreatorFcn(HANDPOSE_PLUGIN_TYPE, HandposePlugin::creator);

       cout << "[ INFO ] Hand Pose Plugin Loading ..." << endl;
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