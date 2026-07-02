//#pragma once
//class customMessage
//{
//};

/*********************************************************************************
** Copyright (c) 2025 MAK Technologies, Inc.
** All rights reserved.
*********************************************************************************/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Generated from: C:/Projects/vrengage2-2-branch/install/vrEngage/examples/sandbox/ForceFireCmd.lua
*/
/*-----------------COMMENT----------------------
Message used to force a fire on human entities with weapons.
-------------------COMMENT----------------------*/
#pragma once




#include <vreMessageManager/vreMessage.h>
#include <vreMessageManager/vreMessageId.h>
#include <vlpi/entityIdentifier.h>


namespace makVre
{

    constexpr const char* CustomMessageType = "simulation.custom.message";
    

    class CustomMessage : public makVre::DtVreMessage
    {
    public:


        // Struct for defining data
        struct CustomMessageData
        {
            CustomMessageData();

            DtEntityIdentifier myTarget;

            bool operator==(const CustomMessageData& rhs) const;
        };

        // Default static creators to promote the usage of not using new.
        static CustomMessage* create();
        static CustomMessage* createFrom(CustomMessage*);
        static CustomMessage* createFromData(const CustomMessageData);

        //! \brief static message identity functions
        static const std::string& theType();
        static const makVre::DtVreMessageId& theId();
        static UInt64 theVersionNumber();

        //! \brief Get the name of the message type
        virtual const std::string& type() const override;
        virtual const makVre::DtVreMessageId& messageId() const override;

        // default destructor
        virtual ~CustomMessage() override;

        virtual const DtEntityIdentifier& getTarget() const;
        virtual void setTarget(const DtEntityIdentifier& val);


        // Accessors
        virtual int messageSize() const override;
        static makVre::DtVreMessage* decode(char* buffer, int length);
        CustomMessageData getData();
        CustomMessageData getData() const;

        // Operators
        bool operator==(const CustomMessage& rhs) const;

    protected:
        // Inaccessible default constructor to promote the usage of the static creator functions
        CustomMessage();

        virtual void serialize(DtStreamWriter& writer) const override;
        virtual void deserialize(DtStreamReader& reader) override;

    protected:
        static makVre::DtVreMessageId theMessageId;
        static UInt64 theVersion;

        // Store the data for future use
        CustomMessageData myData;
    };

} // ::makVre


