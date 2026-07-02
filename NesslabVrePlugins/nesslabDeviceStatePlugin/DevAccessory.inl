/******************************************************************************
** Copyright (c) 2024 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file DevAccessory.inl
//! \brief Contains implementation for DevAccessory. This class is inline
//! so that its protocol can be defined by macros elsewhere.

#include "deviceStateAccessory.h"

#include <vrvCore/DtDe.h>
#include <vrvVrl/DtVrlinkConnection.h>
#include <vrvVrl/DtVrlinkDriver.h>

#include <boost/bind/bind.hpp>
#include <iostream>

#include <RTI1516.h>//<RTI.hh>
#include <vl/exerciseConn.h>
#include <vl/fom.h>
#include "DevAccessory.h"

namespace makVrv
{
   namespace DT_PROTOCOL_NAMESPACE
   {
   
#ifdef DtDIS
      static std::string accessoryName="DevAccessoryDis";
#else
#ifdef DtHLA_1516
   #ifdef DtHLA_4
      static std::string accessoryName = "DevAccessoryHla4";
   #elif DtHLA_1516_EVOLVED
      static std::string accessoryName = "DevAccessoryHla1516e";
   #else
      static std::string accessoryName="DevAccessoryHla1516";
   #endif
#else
   static std::string accessoryName="DevAccessoryHla13";
#endif
#endif

       DevAccessory::DevAccessory(DtDe& de)
       :    DtBaseAccessory(accessoryName)
       ,    myDe(de)
       {
           std::cout << "[ INFO ] Accessory Start : " << accessoryName << std::endl;
       }

      DevAccessory::~DevAccessory()
      {
      }

      bool DevAccessory::isCompatible(DtDriver* driver)
      {
         // This accessory is compatible with any DtVrlinkDriver
         DtVrlinkDriver* dd = dynamic_cast<DtVrlinkDriver*>(driver);
         if(dd)
         {
            return true;
         }

         return false;
      }

      void DevAccessory::install(DtDriver* driver)
      {
         DtVrlinkDriver* vld = dynamic_cast<DtVrlinkDriver*>(driver);

         //Test
         //DtVrlinkBaseConnection* baseConn = vld->connection();
         //DtVrlinkConnection* vrlCon = dynamic_cast<DtVrlinkConnection*>(baseConn);

         //std::cout << "[ INFO ] Create Device State Fom Mapper " << std::endl;
         //DEV::deviceStateFom(vrlCon->exerciseConn(), false);

         ///*
         if(vld)
         {
             std::cout << "Accessory install" << std::endl;
            vld->signal_connectionCreated.connect(
               boost::bind(&DevAccessory::slot_onSimCreated, this, boost::placeholders::_1));
         }
        // */
      }

      void DevAccessory::uninstall(DtDriver* baseConn)
      {
         DtVrlinkDriver* vld = dynamic_cast<DtVrlinkDriver*>(baseConn);
         DtVrlinkBaseConnection* myBaseConn = vld->connection();
         DtVrlinkConnection* vrlCon = dynamic_cast<DtVrlinkConnection*>(myBaseConn);
         std::cout << "[ INFO ] Unload Device State Fom Mapper " << std::endl;
          devStateFom->undeviceStateFom(vrlCon->exerciseConn());

         if(vld)
         {
             std::cout << "Accessory unInstall" << std::endl;
            //! disconnect accessory from connection signals.
            vld->signal_connectionCreated.disconnect(
               boost::bind(&DevAccessory::slot_onSimCreated, this, boost::placeholders::_1));
         }
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
            boost::bind(&DevAccessory::slot_onSimConnected, this, connection));

         connection->signal_toBeDisconnected.connect(
            boost::bind(&DevAccessory::slot_onSimAboutToBeDisconnected, this, connection));
      }

      void DevAccessory::slot_onSimConnected(DtVrlinkBaseConnection* connection)
      {
         DtVrlinkConnection* vrlCon = dynamic_cast<DtVrlinkConnection*>(connection);
         if(vrlCon)
         {
            //DtExerciseConn& exConn = *(vrlCon->exerciseConn());

            std::cout << "[ INFO ] Create Device State Fom Mapper " << std::endl;
            devStateFom = new DEV::DevStateFom();
            devStateFom->deviceStateFom(vrlCon->exerciseConn(), false);
         }
      }

      void DevAccessory::slot_onSimAboutToBeDisconnected(DtVrlinkBaseConnection* sim)
      {
         DtVrlinkConnection* vrlsim=dynamic_cast<DtVrlinkConnection*>(sim);
         if(vrlsim)
         {
            // Do any necessary cleanup here
         }
      }
   }
}
