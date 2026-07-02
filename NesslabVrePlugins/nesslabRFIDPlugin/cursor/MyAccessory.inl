/******************************************************************************
** Copyright (c) 2024 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file MyAccessory.inl
//! \brief Port of example MyAccessory.inl for HLA 1516 Evolved

#include "vreAccessoryTemplate.h"

#include <vrvVrl/DtVrlinkConnection.h>
#include <vrvVrl/DtVrlinkDriver.h>

#include <boost/bind/bind.hpp>
#include <iostream>

#include <RTI/RTI1516.h>
#include <vl/exerciseConn.h>
#include <vl/fom.h>

namespace makVrv
{
    namespace DT_PROTOCOL_NAMESPACE
    {

#ifdef DtDIS
        static std::string accessoryName = "MyAccessoryDis";
#else
#ifdef DtHLA_1516
#ifdef DtHLA_4
        static std::string accessoryName = "MyAccessoryHla4";
#elif DtHLA_1516_EVOLVED
        static std::string accessoryName = "MyAccessoryHla1516e";
#else
        static std::string accessoryName = "MyAccessoryHla1516";
#endif
#else
        static std::string accessoryName = "MyAccessoryHla13";
#endif
#endif

        MyAccessory::MyAccessory()
            : DtBaseAccessory(accessoryName)
        {
        }

        MyAccessory::~MyAccessory()
        {
        }

        bool MyAccessory::isCompatible(DtDriver* driver)
        {
            DtVrlinkDriver* dd = dynamic_cast<DtVrlinkDriver*>(driver);
            return dd != NULL;
        }

        void MyAccessory::install(DtDriver* driver)
        {
            std::cout << "[ INFO ] Install Accessory" << std::endl;
            DtVrlinkDriver* vld = dynamic_cast<DtVrlinkDriver*>(driver);
            if (vld)
            {
                vld->signal_connectionCreated.connect(
                    boost::bind(&MyAccessory::slot_onSimCreated, this, boost::placeholders::_1));
            }
        }

        void MyAccessory::uninstall(DtDriver* baseConn)
        {
            DtVrlinkDriver* vld = dynamic_cast<DtVrlinkDriver*>(baseConn);
            if (vld)
            {
                vld->signal_connectionCreated.disconnect(
                    boost::bind(&MyAccessory::slot_onSimCreated, this, boost::placeholders::_1));
            }
        }

        void MyAccessory::slot_onSimCreated(DtVrlinkBaseConnection* connection)
        {
            connection->signal_connected.connect(
                boost::bind(&MyAccessory::slot_onSimConnected, this, connection));

            connection->signal_toBeDisconnected.connect(
                boost::bind(&MyAccessory::slot_onSimAboutToBeDisconnected, this, connection));
        }

        void MyAccessory::slot_onSimConnected(DtVrlinkBaseConnection* connection)
        {
            DtVrlinkConnection* vrlCon = dynamic_cast<DtVrlinkConnection*>(connection);
            if (vrlCon)
            {
                std::cout << "[vreAccessoryTemplate] VR-Link connected (HLA1516e)" << std::endl;

                DtExerciseConn& exConn = *(vrlCon->exerciseConn());

                DtObjClassDesc* classDesc = exConn.fom()->objClassByName("BaseEntity.PhysicalEntity");
                if (NULL != classDesc)
                {
                    classDesc->unsubscribe();
                }
                classDesc = exConn.fom()->objClassByName("BaseEntity.PhysicalEntity.Lifeform");
                if (NULL != classDesc)
                {
                    classDesc->unsubscribe();
                }
                classDesc = exConn.fom()->objClassByName("BaseEntity.PhysicalEntity.Lifeform.Human");
                if (NULL != classDesc)
                {
                    classDesc->unsubscribe();
                }
                classDesc = exConn.fom()->objClassByName("BaseEntity.PhysicalEntity.Lifeform.NonHuman");
                if (NULL != classDesc)
                {
                    classDesc->unsubscribe();
                }
            }
        }

        void MyAccessory::slot_onSimAboutToBeDisconnected(DtVrlinkBaseConnection* sim)
        {
            DtVrlinkConnection* vrlsim = dynamic_cast<DtVrlinkConnection*>(sim);
            if (vrlsim)
            {
            }
        }
    }
}
