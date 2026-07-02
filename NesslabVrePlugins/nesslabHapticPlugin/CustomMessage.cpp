

/*********************************************************************************
** Copyright (c) 2025 MAK Technologies, Inc.
** All rights reserved.
*********************************************************************************/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Generated from: C:/Projects/vrengage2-2-branch/install/vrEngage/examples/sandbox/ForceFireCmd.lua
*/

#include "CustomMessage.h"

#include <vlutil/vlPrint.h>

using namespace makVre;

//this is the full name of this event
//DtVreMessageId CustomMessage::theMessageId(CustomMessageType);
DtVreMessageId CustomMessage::theMessageId(CustomMessageType);
UInt64 CustomMessage::theVersion = 1;

CustomMessage::CustomMessage()
	: DtVreMessage()
	, myData()
{
}

CustomMessage::~CustomMessage()
{
}

CustomMessage* CustomMessage::create()
{
	return new CustomMessage();
}

CustomMessage* CustomMessage::createFrom(CustomMessage* origMsg)
{
	return new CustomMessage(*origMsg);
}

CustomMessage* CustomMessage::createFromData(const CustomMessage::CustomMessageData data)
{
	auto* msg = new CustomMessage();
	msg->myData = data;
	return msg;
}

const std::string& CustomMessage::theType()
{
	return theMessageId.type();
}

const DtVreMessageId& CustomMessage::theId()
{
	return theMessageId;
}

UInt64 CustomMessage::theVersionNumber()
{
	return theVersion;
}

const std::string& CustomMessage::type() const
{
	return theMessageId.type();
}

const DtVreMessageId& CustomMessage::messageId() const
{
	return theMessageId;
}

bool CustomMessage::operator==(const CustomMessage& rhs) const
{
	return getData().myTarget == rhs.getData().myTarget;
}

const DtEntityIdentifier& CustomMessage::getTarget() const
{
	return myData.myTarget;
}

void CustomMessage::setTarget(const DtEntityIdentifier& val)
{
	myData.myTarget = val;
}

int CustomMessage::messageSize() const
{
	int size = DtVreMessage::messageSize();
	size += 6;

	return size;
}

DtVreMessage* CustomMessage::decode(char* buffer, int length)
{
	DtStreamReader reader(buffer, length);
	CustomMessage* msg = new CustomMessage();
	msg->deserialize(reader);

	return msg;
}

CustomMessage::CustomMessageData::CustomMessageData()
{
}

bool CustomMessage::CustomMessageData::operator==(const CustomMessage::CustomMessageData& rhs) const
{
	return this->myTarget == rhs.myTarget;
}

CustomMessage::CustomMessageData CustomMessage::getData()
{
	return myData;
}

CustomMessage::CustomMessageData CustomMessage::getData() const
{
	return myData;
}

void CustomMessage::serialize(DtStreamWriter& writer) const
{
	DtVreMessage::serialize(writer);
	writer.writeUInt16(myData.myTarget.site());
	writer.writeUInt16(myData.myTarget.host());
	writer.writeUInt16(myData.myTarget.entityNum());


}

void CustomMessage::deserialize(DtStreamReader& reader)
{
	DtVreMessage::deserialize(reader);
	myData.myTarget.setSite(reader.readUInt16());
	myData.myTarget.setHost(reader.readUInt16());
	myData.myTarget.setEntityNum(reader.readUInt16());


}



