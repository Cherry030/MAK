/******************************************************************************
** Copyright (c) 2024 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

#define DL_DLL_IGCONVRLINK DT_DLL_EXAMPLEACCESSORYTEMPLATE
#define DT_PROTOCOL_NAMESPACE vrvHla1516e

#include "DevAccessory.h"

#include "deviceStateAccessory.h"

#include <vrvCore/DtDe.h>
#include <vrvVrl/DtVrlinkConnection.h>
#include <vrvVrl/DtVrlinkDriver.h>

#include <boost/bind/bind.hpp>
#include <iostream>

#include <RTI1516.h>
#include <vl/exerciseConn.h>
#include <vl/fom.h>

namespace makVrv
{
namespace DT_PROTOCOL_NAMESPACE
{

#ifdef DtDIS
static std::string accessoryName = "DevAccessoryDis";
#else
#ifdef DtHLA_1516
    #ifdef DtHLA_4
        static std::string accessoryName = "DevAccessoryHla4";
    #elif DtHLA_1516_EVOLVED
        static std::string accessoryName = "DevAccessoryHla1516e";
    #else
        static std::string accessoryName = "DevAccessoryHla1516";
    #endif
#else
static std::string accessoryName = "DevAccessoryHla13";
#endif
#endif

//------------------------------------------------------------
// Singleton
//------------------------------------------------------------

DevAccessory* DevAccessory::s_instance = nullptr;

DevAccessory* DevAccessory::instance()
{
    return s_instance;
}

//------------------------------------------------------------

DevAccessory::DevAccessory(DtDe& de)
   : DtBaseAccessory(accessoryName)
   , myDe(de)
{
    s_instance = this;
    std::cout << "[ INFO ] Accessory Start : " << accessoryName << std::endl;
}

DevAccessory::~DevAccessory()
{
    if (s_instance == this)
        s_instance = nullptr;
}

bool DevAccessory::isCompatible(DtDriver* driver)
{
    DtVrlinkDriver* dd = dynamic_cast<DtVrlinkDriver*>(driver);
    return (dd != nullptr);
}

void DevAccessory::install(DtDriver* driver)
{
    DtVrlinkDriver* vld = dynamic_cast<DtVrlinkDriver*>(driver);

    if (vld)
    {
        std::cout << "Accessory install" << std::endl;

        vld->signal_connectionCreated.connect(
            boost::bind(
                &DevAccessory::slot_onSimCreated,
                this,
                boost::placeholders::_1));
    }
}

void DevAccessory::uninstall(DtDriver* baseConn)
{
    DtVrlinkDriver* vld = dynamic_cast<DtVrlinkDriver*>(baseConn);

    DtVrlinkBaseConnection* myBaseConn = vld->connection();
    DtVrlinkConnection* vrlCon =
        dynamic_cast<DtVrlinkConnection*>(myBaseConn);

    std::cout << "[ INFO ] Unload Device State Fom Mapper " << std::endl;

    devStateFom->undeviceStateFom(vrlCon->exerciseConn());

    if (vld)
    {
        std::cout << "Accessory unInstall" << std::endl;

        vld->signal_connectionCreated.disconnect(
            boost::bind(
                &DevAccessory::slot_onSimCreated,
                this,
                boost::placeholders::_1));
    }
}

void DevAccessory::setTreadmillType(byte val)
{
    devStateFom->setTreadmillType(val);
}

void DevAccessory::setTreadmillState(byte val)
{
    devStateFom->setTreadmillState(val);
}

void DevAccessory::setManipulatorState(byte val)
{
    devStateFom->setManipulatorState(val);
}

void DevAccessory::setHelmetPatchState(byte helmet)
{
    devStateFom->setHelmetPatchState(helmet);
}

void DevAccessory::setBodyPatchState(byte body)
{
    devStateFom->setBodyPatchState(body);
}

void DevAccessory::setHandPatchState(byte hand)
{
    devStateFom->setHandPatchState(hand);
}
void DevAccessory::slot_onSimCreated(DtVrlinkBaseConnection* connection)
{
    connection->signal_connected.connect(
        boost::bind(
            &DevAccessory::slot_onSimConnected,
            this,
            connection));

    connection->signal_toBeDisconnected.connect(
        boost::bind(
            &DevAccessory::slot_onSimAboutToBeDisconnected,
            this,
            connection));
}

void DevAccessory::slot_onSimConnected(DtVrlinkBaseConnection* connection)
{
    DtVrlinkConnection* vrlCon =
        dynamic_cast<DtVrlinkConnection*>(connection);

    if (vrlCon)
    {
        std::cout << "[ INFO ] Create Device State Fom Mapper " << std::endl;

        devStateFom = new DEV::DevStateFom();
        devStateFom->deviceStateFom(
            vrlCon->exerciseConn(),
            false);
    }
}

void DevAccessory::slot_onSimAboutToBeDisconnected(
    DtVrlinkBaseConnection* sim)
{
    DtVrlinkConnection* vrlsim =
        dynamic_cast<DtVrlinkConnection*>(sim);

    if (vrlsim)
    {
        // Do any necessary cleanup here
    }
}

} // namespace DT_PROTOCOL_NAMESPACE
} // namespace makVrv

#undef DT_PROTOCOL_NAMESPACE
#undef DL_DLL_IGCONVRLINK