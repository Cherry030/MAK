#include "HumanGunJoyActuatorCmds.h"




#include <vrfobjcore/localObject.h>
#include <vrfobjcore/simComponentManager.h>
#include "myHumanGunJoyActuator.h"

//#include "ForceReloadCmd.h"
//#include "SetAmmoCmd.h"
//#include "SetRemainingAmmoCmd.h"
#include "ForceFireCmd.h"

#include <vreMessageManager/vreMessageManager.h>


/*
DtSetAmmoCommand::DtSetAmmoCommand(DtSimObjectManager* objectManager)
   : myObjectManager(objectManager)
{
}

bool DtSetAmmoCommand::execute(const DtString& parameters)
{
   std::vector<DtString> tokens;

   parameters.tokenize(' ', tokens);

   if (tokens.empty())
   {
      return false;
   }

   DtUUID entityUUID = DtUUID(tokens[0]);
   DtString ammoCountStr = tokens.size() > 1 ? tokens[1] : "";
   int ammoCount = ammoCountStr.toInt();

   DtSimObjectReference obj = myObjectManager->lookup(entityUUID);
   
   if (!obj)
   {
      return false;
   }

   makVre::SetAmmoMessage* msg = makVre::SetAmmoMessage::create();
   msg->setAmmoCount(ammoCount);
   msg->setTarget(obj->entityId());
   makVre::DtVreMessageManager::instance().queueMessage(msg);

   return true;
}


DtSetRoundsRemainingCommand::DtSetRoundsRemainingCommand(DtSimObjectManager* objectManager)
   : myObjectManager(objectManager)
{
}

bool DtSetRoundsRemainingCommand::execute(const DtString& parameters)
{
   std::vector<DtString> tokens;
   parameters.tokenize(' ', tokens);

   if (tokens.empty())
   {
      return false;
   }

   DtUUID entityUUID = DtUUID(tokens[0]);

   DtString ammoCountStr = tokens.size() > 1 ? tokens[1] : "";
   int ammoCount = ammoCountStr.toInt();

   DtSimObjectReference obj = myObjectManager->lookup(entityUUID);

   if (!obj)
   {
      return false;
   }

   makVre::SetRemainingAmmoMessage* msg = makVre::SetRemainingAmmoMessage::create();
   msg->setAmmoCount(ammoCount);
   msg->setTarget(obj->entityId());
   makVre::DtVreMessageManager::instance().queueMessage(msg);

   return true;
}

DtForceReloadCommand::DtForceReloadCommand(DtSimObjectManager* objectManager)
   : myObjectManager(objectManager)
{
}

bool DtForceReloadCommand::execute(const DtString& parameters)
{
   std::vector<DtString> tokens;
   parameters.tokenize(' ', tokens);

   if (tokens.empty())
   {
      return false;
   }

   DtUUID entityUUID = DtUUID(tokens[0]);
   DtSimObjectReference obj = myObjectManager->lookup(entityUUID);

   if (!obj)
   {
      return false;
   }

   makVre::ForceReloadMessage* msg = makVre::ForceReloadMessage::create();
   msg->setTarget(obj->entityId());
   makVre::DtVreMessageManager::instance().queueMessage(msg);

   return true;
}




*/


DtForceFireCommand::DtForceFireCommand(DtSimObjectManager* objectManager)
   : myObjectManager(objectManager)
{
    std::cout << "[DtForceFireCommand] Create DtForceFireCommand class \n";

}


//True on success of the command.
bool DtForceFireCommand::execute(const DtString& parameters)
{
    std::cout << "[DtForceFireCommand] objectManager->simObjects().size():" << myObjectManager->simObjects().size() << std::endl;

    std::cout << "[DtForceFireCommand] execute fn \n";
    //std::cout << "[DtForceFireCommand] showInHelp: " << std::boolalpha<< showInHelp()   << std::endl;
    //std::cout << "[DtForceFireCommand] help: "       << help(parameters)                << std::endl;

   std::vector<DtString> tokens;
   parameters.tokenize(' ', tokens);

   std::cout << "[DtForceFireCommand] parameters:" << parameters.string() << std::endl;
   std::cout << "[DtForceFireCommand] parameters:" << parameters.size()   << std::endl;

   if (tokens.empty())
   {
       std::cout << "[DtForceFireCommand] false 1\n";
      
       return false;
   }

   // 1. 콘솔 입력으로부터 대상 UUID 받기
   DtUUID entityUUID = DtUUID(tokens[0]);
   // 2. UUID로 SimObject 찾기
   DtSimObjectReference obj = myObjectManager->lookup(entityUUID);
   if (!obj)
   {
       
       std::cout << "[DtForceFireCommand] false 2\n";

      return false;
   }

   // 3. ForceFireMessage 생성
   makVre::ForceFireMessage* msg = makVre::ForceFireMessage::create();
   // 4. 메시지에 대상 entityId 설정
   msg->setTarget(obj->entityId());
   // 5. 메시지 송신(큐에 등록)
   makVre::DtVreMessageManager::instance().queueMessage(msg);


   std::cout << "[DtForceFireCommand] true \n";

   return true;
}