/*******************************************************************************
** Copyright (c) 2020 MAK Technologies
** All rights reserved.
*******************************************************************************/

//#include "exampleHumanDamageActuator.h"

#include <vreUtil/version.h>
#include <vrfcgf/vrfPluginExtension.h>
#include <vrfcgf/cgf.h>
#include <vrfcgf/factoryManager.h>
#include <vrfcgf/vrfCreator.h>
#include <vrfobjcore/simComponentTypes.h>

#include "MyHumanMovementActuator.h"


#include "utilities/vreMessageManager/vreMessageFactory.h"
#include <vreMessageManager/forwardMessage.h>
#include "Nesslab/StopAllPluginsMessage.h"

using namespace makVre;

extern "C" {

    void DT_VRF_DLL_PLUGIN DtPluginInformation(DtVrfPluginInformation& info)
    {
        info.pluginName = "nesslabHumanMovement";
        info.pluginVersion = DtVreVersionNumber;
        info.pluginCreator = "Nesslab";
    }



   DT_VRF_DLL_PLUGIN bool DtInitializeVrfPlugin(DtCgf* cgf)
   {
      return true;
   }

   // Post-initialization (optional)
   DT_VRF_DLL_PLUGIN bool DtPostInitializeVrfPlugin(DtCgf* cgf)
   {

       //캐릭터 이동 플러그인(BackEnd)
       cgf->factoryManager()->componentFactory()->addCreatorFcn(makVre::DtVreHumanMovementActuatorType, MyHumanMovementActuator::creator);


       //ForwardMessageMessage -> Send to start forwarding the specified event manager message type to the network. -> Front+BackEnd
       makVre::DtVreMessageManager::instance().factory().registerMessage<makVre::StopAllPluginsMessage>();
       auto* fMsg = makVre::ForwardMessageMessage::create();
       fMsg->setMessageName(makVre::StopAllPluginsMessage::theType());
       makVre::DtVreMessageManager::instance().queueMessage(fMsg);


      return true;
   }


}