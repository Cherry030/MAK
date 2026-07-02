/*********************************************************************************
** Copyright (c) 2025 MAK Technologies, Inc.
** All rights reserved.
*********************************************************************************/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Generated from: C:/Projects/vrengage2-2-branch/install/vrEngage/examples/sandbox/ForceFireCmd.lua
*/

//#include <C:/Projects/vrengage2-2-branch/install/vrEngage/examples/sandbox/ForceFireCmd.h>
  
#include "ForceFireCmd.h"
#include <vlutil/vlPrint.h>

using namespace makVre;

//this is the full name of this event
DtVreMessageId ForceFireMessage::theMessageId(ForceFireMessageType);
UInt64 ForceFireMessage::theVersion = 1;

ForceFireMessage::ForceFireMessage()
   : DtVreMessage()
   , myData()
{
}

ForceFireMessage::~ForceFireMessage()
{
}

ForceFireMessage* ForceFireMessage::create()
{
   return new ForceFireMessage();
}

ForceFireMessage* ForceFireMessage::createFrom(ForceFireMessage* origMsg)
{
   return new ForceFireMessage(*origMsg);
}

ForceFireMessage* ForceFireMessage::createFromData(const ForceFireMessage::ForceFireData data)
{
   auto* msg = new ForceFireMessage();
   msg->myData = data;
   return msg;
}

const std::string& ForceFireMessage::theType()
{
   return theMessageId.type();
}

const DtVreMessageId& ForceFireMessage::theId()
{
   return theMessageId;
}

UInt64 ForceFireMessage::theVersionNumber()
{
   return theVersion;
}

const std::string& ForceFireMessage::type() const
{
   return theMessageId.type();
}

const DtVreMessageId& ForceFireMessage::messageId() const
{
   return theMessageId;
}

bool ForceFireMessage::operator==(const ForceFireMessage& rhs) const
{
   return getData().myTarget == rhs.getData().myTarget;
}

const DtEntityIdentifier& ForceFireMessage::getTarget() const
{
   return myData.myTarget;
}

void ForceFireMessage::setTarget(const DtEntityIdentifier& val)
{
   myData.myTarget = val;
}
   
int ForceFireMessage::messageSize() const
{
   int size = DtVreMessage::messageSize();
   size += 6;

   return size;
}

DtVreMessage* ForceFireMessage::decode(char* buffer, int length)
{
   DtStreamReader reader(buffer, length);
   ForceFireMessage* msg = new ForceFireMessage();
   msg->deserialize(reader);
   
   return msg;
}

ForceFireMessage::ForceFireData::ForceFireData()
{
}

bool ForceFireMessage::ForceFireData::operator==(const ForceFireMessage::ForceFireData& rhs) const
{
   return this->myTarget == rhs.myTarget;
}

ForceFireMessage::ForceFireData ForceFireMessage::getData()
{
   return myData;
}

ForceFireMessage::ForceFireData ForceFireMessage::getData() const
{
   return myData;
}

void ForceFireMessage::serialize(DtStreamWriter& writer) const
{
   DtVreMessage::serialize(writer);
   writer.writeUInt16(myData.myTarget.site());
   writer.writeUInt16(myData.myTarget.host());
   writer.writeUInt16(myData.myTarget.entityNum());
   

}

void ForceFireMessage::deserialize(DtStreamReader& reader)
{
   DtVreMessage::deserialize(reader);
   myData.myTarget.setSite(reader.readUInt16());
   myData.myTarget.setHost(reader.readUInt16());
   myData.myTarget.setEntityNum(reader.readUInt16());
   

}



   