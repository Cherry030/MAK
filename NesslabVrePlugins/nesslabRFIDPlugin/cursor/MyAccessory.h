/******************************************************************************
** Copyright (c) 2019 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file MyAccessory.h
//! \brief Port of example MyAccessory.h (protocol namespace set by including TU)

#pragma once

#include "vreAccessoryTemplate.h"
#include <vrvCore/DtBaseAccessory.h>

namespace makVrv
{
    class DtVrlinkBaseConnection;

    namespace DT_PROTOCOL_NAMESPACE
    {
        class MyAccessory : public DtBaseAccessory
        {
        public:
            MyAccessory();
            virtual ~MyAccessory();

            virtual bool isCompatible(DtDriver* driver);
            virtual void install(DtDriver* driver);
            virtual void uninstall(DtDriver* baseConn);

        protected:
            virtual void slot_onSimCreated(DtVrlinkBaseConnection* connection);
            virtual void slot_onSimConnected(DtVrlinkBaseConnection* connection);
            virtual void slot_onSimAboutToBeDisconnected(DtVrlinkBaseConnection* connection);
        };
    }
}

#include "MyAccessory.inl"
