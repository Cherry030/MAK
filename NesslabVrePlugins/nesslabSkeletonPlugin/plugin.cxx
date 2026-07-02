#include <vreUtil/version.h>
#include <vrfcgf/vrfPluginExtension.h>
#include <vrfcgf/cgf.h>
#include <vrfcgf/factoryManager.h>
#include <vrfcgf/vrfCreator.h>

#include "Skeleton.h"

using namespace makVre;


//exampleHumanHandWeaponControlActuator
extern "C" {

   DT_VRF_DLL_PLUGIN void DtPluginInformation(DtVrfPluginInformation& info)
   {
       info.pluginName = "nesslabSkeleton";
       info.pluginVersion = DtVreVersionNumber;
       info.pluginCreator = "Nesslab";
   }

   DT_VRF_DLL_PLUGIN bool DtInitializeVrfPlugin(DtCgf* cgf)
   {

       //Register the new human art part actuator with the factory.
       cgf->factoryManager()->componentFactory()->addCreatorFcn(SKELETON_PLUGIN_TYPE, SkeletonPlugin::creator);
       

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