/******************************************************************************
** Copyright (c) 2019 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file MyAccessory.h
//! \brief Contains MyAccessory class.

#pragma once

#include "rfidAccessory.h"
#include <vrvCore/DtBaseAccessory.h>

#include "loginFom.h"

namespace makVrv
{
   //class DtDe;
   class DtVrlinkBaseConnection;

   namespace DT_PROTOCOL_NAMESPACE
   {
      //! \brief This class defines an accessory that will install itself
      //! into DtVrlinkDriver objects
      class MyAccessory : public DtBaseAccessory
      {
      public:
         //! \brief Constructor
         MyAccessory(DtDe& de);

         //! \Default destructor
         virtual ~MyAccessory();

         //! \brief Called to find out if this accessory is compatible with a
         //! particular connector. In this case, it is compatible with any
         //! DtVrlinkDriver of the correct protocol
         virtual bool isCompatible(DtDriver* driver);

         //! \brief Called when this accessory is installed into a connector.
         virtual void install(DtDriver* driver);

         //! \brief Called when this accessory is uninstalled
         virtual void uninstall(DtDriver* baseConn);

      protected:

         //! \brief Called when the driver creates a new connection
         virtual void slot_onSimCreated(DtVrlinkBaseConnection* connection);

         //! \brief Called when a sim connection is connected
         virtual void slot_onSimConnected(DtVrlinkBaseConnection* connection);

         //! \brief Called when a sim connection is disconnected
         virtual void slot_onSimAboutToBeDisconnected(DtVrlinkBaseConnection* connection);

      protected:
          makVrv::DtDe& myDe;
      };
   }
}

#include "MyAccessory.inl"

