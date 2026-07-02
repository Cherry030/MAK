#include <vreUtil/version.h>
#include <vrfcgf/vrfPluginExtension.h>
#include <vrfcgf/cgf.h>
#include <vrfcgf/factoryManager.h>
#include <vrfcgf/vrfCreator.h>


#include "Haptic.h"





extern "C" {

   DT_VRF_DLL_PLUGIN void DtPluginInformation(DtVrfPluginInformation& info)
   {
       info.pluginName    = "nesslabHaptic";
       info.pluginVersion = DtVreVersionNumber;
       info.pluginCreator = "Nesslab";
   }

   DT_VRF_DLL_PLUGIN bool DtInitializeVrfPlugin(DtCgf* cgf)
   {
      return true;
   }


   DT_VRF_DLL_PLUGIN bool DtPostInitializeVrfPlugin(DtCgf* cgf)
   {

      // Register the new human art part actuator with the factory.  Note that we are registering it with
      // the same type as the base class (DtVreHumanDamageActuator), so it replaces any instances of the
      // stock VR-Engage version with our derived version.  As a result, all VR-Engage human entities
      // will use this derived class at runtime.
       cgf->factoryManager()->componentFactory()->addCreatorFcn(makVre::DtVreHumanDamageActuatorType, nesslab_backend_plugins::HapticPlugin::creator);


      return true;
   }

   DT_VRF_DLL_PLUGIN void DtUnloadVrfPlugin()
   {
   }

}