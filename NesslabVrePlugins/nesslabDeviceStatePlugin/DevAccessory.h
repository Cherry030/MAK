/******************************************************************************
** Copyright (c) 2019 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

#pragma once

#include "deviceStateAccessory.h"
#include <vrvCore/DtBaseAccessory.h>
#include "deviceStateFom.h"

namespace makVrv
{
   class DtVrlinkBaseConnection;

   namespace DT_PROTOCOL_NAMESPACE
   {
      class DevAccessory : public DtBaseAccessory
      {
      public:
         DevAccessory(DtDe& de);
         virtual ~DevAccessory();

         // Singleton
         static DevAccessory* instance();

         virtual bool isCompatible(DtDriver* driver);
         virtual void install(DtDriver* driver);
         virtual void uninstall(DtDriver* driver);

         void setTreadmillType(byte val);
         void setTreadmillState(byte val);
         void setManipulatorState(byte val);

         void setHelmetPatchState(byte helmet);
         void setBodyPatchState(byte body);
         void setHandPatchState(byte hand);

      protected:
         virtual void slot_onSimCreated(DtVrlinkBaseConnection* connection);
         virtual void slot_onSimConnected(DtVrlinkBaseConnection* connection);
         virtual void slot_onSimAboutToBeDisconnected(DtVrlinkBaseConnection* connection);

      protected:
         DEV::DevStateFom* devStateFom = nullptr;
         makVrv::DtDe& myDe;

      private:
         static DevAccessory* s_instance;
      };
   }
}