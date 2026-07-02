#include "SimCommmands.h"

#include <vrfobjcore/localObject.h>
#include <vrfobjcore/simComponentManager.h>
#include <vreMessageManager/vreMessageManager.h>

//#include "ForceFireCmd.h"
//#include "CustomMessage.h"
#include "Nesslab/StopAllPluginsMessage.h"


DtCustomMessageCommand::DtCustomMessageCommand(DtSimObjectManager* objectManager)
    : myObjectManager(objectManager)
{

}


//※주의: 커맨드에 커맨드스트링값 + 캐릭터 UUID 합쳐서 보내야함, 실제사용은 밑에 Message 보내는부분만 확인해도될듯
bool DtCustomMessageCommand::execute(const DtString& parameters)
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

    /*
    //Message 생성
    std::cout << "SendMessage \n";
    makVre::CustomMessage* msg = makVre::CustomMessage::create();
    msg->setTarget(obj->entityId());
    
    makVre::DtVreMessageManager::instance().queueMessage(msg);
    */


    std::cout << "SendMessage \n";
    makVre::StopAllPluginsMessage* msg = makVre::StopAllPluginsMessage::create();
    msg->setSender(obj->entityId());
    msg->setStopRequested(true);

    makVre::DtVreMessageManager::instance().queueMessage(msg);

    return true;
}



/*
DtForceFireCommand::DtForceFireCommand(DtSimObjectManager* objectManager)
    : myObjectManager(objectManager)
{

}

bool DtForceFireCommand::execute(const DtString& parameters)
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

    makVre::ForceFireMessage* msg = makVre::ForceFireMessage::create();
    msg->setTarget(obj->entityId());
    makVre::DtVreMessageManager::instance().queueMessage(msg);

    return true;
}
*/