/******************************************************************************
** Copyright (c) 2019 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file DevAccessory.h
//! \brief Contains DevAccessory class.

#pragma once

#include "deviceStateAccessory.h"
#include <vrvCore/DtBaseAccessory.h>

#include "deviceStateFom.h"

namespace makVrv
{
   //class DtDe;
   class DtVrlinkBaseConnection;

   namespace DT_PROTOCOL_NAMESPACE
   {
      //! \brief This class defines an accessory that will install itself
      //! into DtVrlinkDriver objects
      class DevAccessory : public DtBaseAccessory
      {
      public:
         //! \brief Constructor
         DevAccessory(DtDe& de);

         //! \Default destructor
         virtual ~DevAccessory();

         //! \brief Called to find out if this accessory is compatible with a
         //! particular connector. In this case, it is compatible with any
         //! DtVrlinkDriver of the correct protocol
         virtual bool isCompatible(DtDriver* driver);

         //! \brief Called when this accessory is installed into a connector.
         virtual void install(DtDriver* driver);

         //! \brief Called when this accessory is uninstalled
         virtual void uninstall(DtDriver* baseConn);

         DEV::DevStateFom* devStateFom;
         void setHelmetPatchState(byte helmet);
         void setBodyPatchState(byte body);
         void setHandPatchState(byte hand);

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

#include "DevAccessory.inl"

