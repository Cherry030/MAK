/*********************************************************************************
** Copyright (c) 2025 MAK Technologies, Inc.
** All rights reserved.
*********************************************************************************/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Generated from: .\MyMessage.lua
*/
/*-----------------COMMENT----------------------
Message used to stop all plugins operations.
-------------------COMMENT----------------------*/
#pragma once

#include <vreMessageManager/vreMessage.h>
#include <vreMessageManager/vreMessageId.h>
#include <vlpi/entityIdentifier.h>




//messgae많아지면 lib로 묶기

namespace makVre
{

constexpr const char* StopAllPluginsMessageType = "stopAllPlugins";

class  StopAllPluginsMessage : public makVre::DtVreMessage
{
public:


   // Struct for defining data
   struct StopAllPluginsData 
   {
      StopAllPluginsData();

      DtEntityIdentifier mySender;
      bool myStopRequested;

      bool operator==(const StopAllPluginsData& rhs) const;
   };

   // Default static creators to promote the usage of not using new.
   static StopAllPluginsMessage* create();
   static StopAllPluginsMessage* createFrom(StopAllPluginsMessage*);
   static StopAllPluginsMessage* createFromData(const StopAllPluginsData);
   
   //! \brief static message identity functions
   static const std::string& theType();
   static const makVre::DtVreMessageId& theId();
   static UInt64 theVersionNumber();

   //! \brief Get the name of the message type
   virtual const std::string& type() const override;
   virtual const makVre::DtVreMessageId& messageId() const override;

   // default destructor
   virtual ~StopAllPluginsMessage() override;

   virtual const DtEntityIdentifier& getSender() const;
   virtual void setSender(const DtEntityIdentifier& val);

   virtual bool getStopRequested() const;
   virtual void setStopRequested(bool val);


   // Accessors
   virtual int messageSize() const override;
   static makVre::DtVreMessage* decode(char* buffer, int length);
   StopAllPluginsData getData();
   StopAllPluginsData getData() const;

   // Operators
   bool operator==(const StopAllPluginsMessage& rhs) const;

protected:
   // Inaccessible default constructor to promote the usage of the static creator functions
   StopAllPluginsMessage();

   virtual void serialize(DtStreamWriter& writer) const override;
   virtual void deserialize(DtStreamReader& reader) override;

protected:
   static makVre::DtVreMessageId theMessageId;
   static UInt64 theVersion;

   // Store the data for future use
   StopAllPluginsData myData;
};

} // ::makVre  