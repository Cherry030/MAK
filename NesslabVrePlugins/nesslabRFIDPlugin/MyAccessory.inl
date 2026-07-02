/******************************************************************************
** Copyright (c) 2024 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file MyAccessory.inl
//! \brief Contains implementation for MyAccessory. This class is inline
//! so that its protocol can be defined by macros elsewhere.

#include "rfidAccessory.h"

#include <vrvCore/DtDe.h>
#include <vrvVrl/DtVrlinkConnection.h>
#include <vrvVrl/DtVrlinkDriver.h>

#include <boost/bind/bind.hpp>
#include <iostream>

#include <RTI1516.h>//<RTI.hh>
#include <vl/exerciseConn.h>
#include <vl/fom.h>

namespace makVrv
{
   namespace DT_PROTOCOL_NAMESPACE
   {
   
#ifdef DtDIS
      static std::string accessoryName="MyAccessoryDis";
#else
#ifdef DtHLA_1516
   #ifdef DtHLA_4
      static std::string accessoryName = "MyAccessoryHla4";
   #elif DtHLA_1516_EVOLVED
      static std::string accessoryName = "MyAccessoryHla1516e";
   #else
      static std::string accessoryName="MyAccessoryHla1516";
   #endif
#else
   static std::string accessoryName="MyAccessoryHla13";
#endif
#endif

       MyAccessory::MyAccessory(DtDe& de)
       :    DtBaseAccessory(accessoryName)
       ,    myDe(de)
       {
           std::cout << "[ INFO ] Accessory Start : " << accessoryName << std::endl;

       }

      MyAccessory::~MyAccessory()
      {
      }

      bool MyAccessory::isCompatible(DtDriver* driver)
      {
         // This accessory is compatible with any DtVrlinkDriver
         DtVrlinkDriver* dd = dynamic_cast<DtVrlinkDriver*>(driver);
         if(dd)
         {
            return true;
         }

         return false;
      }

      void MyAccessory::install(DtDriver* driver)
      {
         DtVrlinkDriver* vld = dynamic_cast<DtVrlinkDriver*>(driver);

         //Test
         DtVrlinkBaseConnection* baseConn = vld->connection();
         DtVrlinkConnection* vrlCon = dynamic_cast<DtVrlinkConnection*>(baseConn);

         std::cout << "[ INFO ] Create Login Fom Mapper " << std::endl;
         RFID::loginFom(vrlCon->exerciseConn(), false);

         /*
         if(vld)
         {
             std::cout << "Accessory install" << std::endl;
            vld->signal_connectionCreated.connect(
               boost::bind(&MyAccessory::slot_onSimCreated, this, boost::placeholders::_1));
         }
         */
      }

      void MyAccessory::uninstall(DtDriver* baseConn)
      {
         DtVrlinkDriver* vld = dynamic_cast<DtVrlinkDriver*>(baseConn);
         DtVrlinkBaseConnection* myBaseConn = vld->connection();
         DtVrlinkConnection* vrlCon = dynamic_cast<DtVrlinkConnection*>(myBaseConn);
         std::cout << "[ INFO ] Unload Login Info Fom Mapper " << std::endl;
         RFID::untraineeFom(vrlCon->exerciseConn());

         if(vld)
         {
             std::cout << "Accessory unInstall" << std::endl;
            //! disconnect accessory from connection signals.
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
         if(vrlCon)
         {
            // Here you can install register object creators, as well as access the
            // VR-Link exercise connection
            DtExerciseConn& exConn = *(vrlCon->exerciseConn());
            DtObjClassDesc* classDesc = exConn.fom()->
               objClassByName("BaseEntity.PhysicalEntity");
            if (NULL != classDesc)
            {
               classDesc->unsubscribe();
            }
            classDesc = exConn.fom()->
               objClassByName("BaseEntity.PhysicalEntity.Lifeform");
            if (NULL != classDesc)
            {
               classDesc->unsubscribe();
            }
            classDesc = exConn.fom()->
               objClassByName("BaseEntity.PhysicalEntity.Lifeform.Human");
            if (NULL != classDesc)
            {
               classDesc->unsubscribe();
            }
            classDesc = exConn.fom()->
               objClassByName("BaseEntity.PhysicalEntity.Lifeform.NonHuman");
            if (NULL != classDesc)
            {
               classDesc->unsubscribe();
            }
         }
      }

      void MyAccessory::slot_onSimAboutToBeDisconnected(DtVrlinkBaseConnection* sim)
      {
         DtVrlinkConnection* vrlsim=dynamic_cast<DtVrlinkConnection*>(sim);
         if(vrlsim)
         {
            // Do any necessary cleanup here
         }
      }
   }
}
