#pragma warning(disable: 4251)
#pragma warning(disable: 4786)
#pragma warning(disable: 4290)

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "rtiFedAmb1516e.h"
#include "rtiString.h"
#include "rtiNetworkPlugin.h"

using namespace std;
using namespace rti1516e;
using namespace nesslab_backend_plugins;

namespace {

    VariableLengthData const* paramByHandle(ParameterHandleValueMap const& values, ParameterHandle h)
    {
        auto vi = values.find(h);
        return vi == values.end() ? nullptr : &vi->second;
    }

    VariableLengthData const* paramByName(
        ParameterHandleValueMap const& values,
        DtParamNameHandleMap const& pmap,
        wstring const& name)
    {
        auto ni = pmap.find(name);
        if (ni == pmap.end())
            return nullptr;
        return paramByHandle(values, ni->second);
    }

    std::vector<unsigned char> ConvertVariableLengthData(VariableLengthData const& d)
    {
        const unsigned char* base = static_cast<const unsigned char*>(d.data());
        return std::vector<unsigned char>(base, base + d.size());
    }

    std::string GetString_Vector10(std::vector<unsigned char> const& v)
    {
        return std::string(v.begin() + 1, v.end());
    }

    /** byte array (FOM): printable 32..126 -> character, space between; else [decimal] */
    void writeByteArrayAsChars(const unsigned char* p, size_t len)
    {
        if (!p || !len)
        {
            wcout << L"(empty)";
            return;
        }
        for (size_t i = 0; i < len; ++i)
        {
            if (i)
                wcout << L' ';
            unsigned char c = p[i];
            if (c >= 32 && c <= 126)
                wcout << (wchar_t)c;
            else
                wcout << L'[' << (unsigned)c << L']';
        }
    }

    void writeByteArrayAsChars(VariableLengthData const* d)
    {
        if (!d || !d->size())
        {
            wcout << L"(absent)";
            return;
        }
        writeByteArrayAsChars(static_cast<const unsigned char*>(d->data()), d->size());
    }

    void writeHexFallback(VariableLengthData const& d, size_t maxBytes = 48)
    {
        wcout << L"hex len=" << d.size();
        const unsigned char* p = static_cast<const unsigned char*>(d.data());
        size_t n = d.size() < maxBytes ? d.size() : maxBytes;
        for (size_t i = 0; i < n; ++i)
            wcout << L' ' << hex << uppercase << setfill(L'0') << setw(2) << (unsigned)p[i];
        wcout << dec;
        if (d.size() > maxBytes)
            wcout << L" ...";
    }

    void printTargetInfoReply(ParameterHandleValueMap const& values)
    {
        std::cout << "\n[KCL] TargetInfoReply" << std::endl;
        if (VariableLengthData const* p = paramByName(values, ParamNameHandleMap_TargetInfoReply, L"requireID"))
            std::cout << "[KCL] requireID " << GetString_Vector10(ConvertVariableLengthData(*p)) << std::endl;
        else
            std::cout << "[KCL] requireID Missing" << std::endl;


        wcout << L"[KCL] targetNumber ";
        if (VariableLengthData const* d = paramByName(values, ParamNameHandleMap_TargetInfoReply, L"targetNumber"))
        {
            if (d->size() >= sizeof(unsigned short))
            {
                unsigned short us = 0;
                memcpy(&us, d->data(), sizeof(us));
                wcout << us;
            }
        }
        else
            wcout << L"Missing";
        wcout << endl;

        if (VariableLengthData const* p = paramByName(values, ParamNameHandleMap_TargetInfoReply, L"targetName"))
            std::cout << "[KCL] targetName " << GetString_Vector10(ConvertVariableLengthData(*p)) << std::endl;
        else
            std::cout << "[KCL] targetName Missing" << std::endl;
    }

    void printLogInInfo(ParameterHandleValueMap const& values)
    {
        wcout << L"\n[recv] LogInInfo" << endl;
        wcout << L"  publisherType ";
        if (VariableLengthData const* d = paramByName(values, ParamNameHandleMap_LogInInfo, L"publisherType"))
        {
            if (d->size() >= 2)
            {
                unsigned short v = 0;
                memcpy(&v, d->data(), 2);
                wcout << v;
            }
            else
                wcout << L"(short)";
        }
        else
            wcout << L"(absent)";
        wcout << endl;

        wcout << L"  deviceID ";
        if (VariableLengthData const* d = paramByName(values, ParamNameHandleMap_LogInInfo, L"deviceID"))
        {
            if (d->size() >= 2)
            {
                unsigned short v = 0;
                memcpy(&v, d->data(), 2);
                wcout << v;
            }
            else
                wcout << L"(short)";
        }
        else
            wcout << L"(absent)";
        wcout << endl;

        wcout << L"  serviceNumber ";
        writeByteArrayAsChars(paramByName(values, ParamNameHandleMap_LogInInfo, L"serviceNumber"));
        wcout << endl;
        wcout << L"  logInName ";
        writeByteArrayAsChars(paramByName(values, ParamNameHandleMap_LogInInfo, L"logInName"));
        wcout << endl;
        wcout << L"  affiliation ";
        writeByteArrayAsChars(paramByName(values, ParamNameHandleMap_LogInInfo, L"affiliation"));
        wcout << endl;
    }

    void printReceivedInteractionData(
        InteractionClassHandle theInteraction,
        ParameterHandleValueMap const& values)
    {
        if (theInteraction == InterClassHandle_TargetInfoReply)
        {
            printTargetInfoReply(values);
            return;
        }


        wcout << L"\n[recv] (unknown interaction class) classHandle=" << theInteraction.toString() << endl;
        for (ParameterHandleValueMap::const_iterator iter = values.begin(); iter != values.end(); ++iter)
        {
            wcout << L"  param " << iter->first.toString() << L"  ";
            writeHexFallback(iter->second);
            wcout << endl;
        }
    }

} // namespace

void printAttrValues(AttributeHandleValueMap const& attrVals)
{
    AttributeHandleValueMap::const_iterator iter;

    for (iter = attrVals.begin(); iter != attrVals.end(); iter++)
    {
        wcout << L"\t" << iter->first.toString() << L" " << DtToWString((char*)iter->second.data()) << endl;
    }
}

void printParamValues(ParameterHandleValueMap const& paramVals)
{
    ParameterHandleValueMap::const_iterator iter;

    for (iter = paramVals.begin(); iter != paramVals.end(); iter++)
    {
        wcout << L"\t" << iter->first.toString() << L" " << DtToWString((char*)iter->second.data()) << endl;
    }
}

MyFederateAmbassador::MyFederateAmbassador(DtSimpleData& data)
    : NullFederateAmbassador(), myData(data)
{
}

MyFederateAmbassador::~MyFederateAmbassador()
throw ()
{
}

void MyFederateAmbassador::connectionLost(
    std::wstring const& faultDescription)
    throw (FederateInternalError)
{
    wcout << L"\nconnectionLost: " << faultDescription << std::endl;
    myData.connectionLost = true;
}

void MyFederateAmbassador::reportFederationExecutions(
    FederationExecutionInformationVector const& theFederationExecutionInformationList)
    throw (FederateInternalError)
{
    myData.listFederationsReturned = true;
    wcout << L"\nreportFederationExecutions \n";
    FederationExecutionInformationVector::const_iterator fedIter = theFederationExecutionInformationList.begin();
    FederationExecutionInformationVector::const_iterator fedEnd = theFederationExecutionInformationList.end();
    for (; fedIter != fedEnd; ++fedIter)
    {
        wcout << fedIter->federationExecutionName << L" " << fedIter->logicalTimeImplementationName
            << std::endl;
    }
}

void MyFederateAmbassador::objectInstanceNameReservationSucceeded(
    std::wstring const& theObjectInstanceName)
    throw (FederateInternalError)
{
    wcout << L"\nobjectInstanceNameReservationSucceeded: " << theObjectInstanceName << endl;
    myData.nameReservationReturned =
        myData.nameReservationSucceeded = true;
}

void MyFederateAmbassador::objectInstanceNameReservationFailed(
    std::wstring const& theObjectInstanceName)
    throw (FederateInternalError)
{
    wcout << L"\nobjectInstanceNameReservationFailed: " << theObjectInstanceName << endl;
    myData.nameReservationReturned = true;
    myData.nameReservationSucceeded = false;
}

void MyFederateAmbassador::discoverObjectInstance(
    ObjectInstanceHandle theObject,
    ObjectClassHandle theObjectClass,
    std::wstring const& theObjectInstanceName)
    throw (FederateInternalError)
{
    wcout << L"\ndiscoverObjectInstance: "
        << theObjectInstanceName << L"("
        << theObject.toString() << L") of class "
        << myData.objectClassMap[theObjectClass] << L"( "
        << theObjectClass.toString() << L")" << endl;
    myData.objectInstanceMap[theObject] = theObjectInstanceName;
}

void MyFederateAmbassador::discoverObjectInstance(
    ObjectInstanceHandle theObject,
    ObjectClassHandle theObjectClass,
    std::wstring const& theObjectInstanceName,
    FederateHandle producingFederate)
    throw (FederateInternalError)
{
    wcout << L"\ndiscoverObjectInstance: "
        << theObjectInstanceName << L"("
        << theObject.toString() << L") of class "
        << myData.objectClassMap[theObjectClass] << L"( "
        << theObjectClass.toString() << L")"
        << L" produced by " << producingFederate.toString()
        << endl;
    myData.objectInstanceMap[theObject] = theObjectInstanceName;
}

void MyFederateAmbassador::reflectAttributeValues(
    ObjectInstanceHandle theObject,
    AttributeHandleValueMap const& theAttributeValues,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    TransportationType theType,
    SupplementalReflectInfo theReflectInfo)
    throw (FederateInternalError)
{
    const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    wcout << L"\nreflectAttributeValues: "
        << myData.objectInstanceMap[theObject] << L"("
        << theObject.toString() << L") "
        << DtToWString(tag) << L" "
        << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theType == RELIABLE ? L"RELIABLE " : L"BEST_EFFORT ")
        << (theReflectInfo.hasProducingFederate ? L"produced by " : L" ")
        << (theReflectInfo.hasProducingFederate ? theReflectInfo.producingFederate.toString() : L"")
        << L"#regions: " << (theReflectInfo.hasSentRegions ? theReflectInfo.sentRegions.size() : 0)
        << L" #attributes: " << theAttributeValues.size() << endl;

    printAttrValues(theAttributeValues);
}

void MyFederateAmbassador::reflectAttributeValues(
    ObjectInstanceHandle theObject,
    AttributeHandleValueMap const& theAttributeValues,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    TransportationType theType,
    LogicalTime const& theTime,
    OrderType receivedOrder,
    SupplementalReflectInfo theReflectInfo)
    throw (FederateInternalError)
{
    const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    wcout << L"\nreflectAttributeValues: "
        << myData.objectInstanceMap[theObject] << L"("
        << theObject.toString() << L") "
        << DtToWString(tag) << L" "
        << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theType == RELIABLE ? L"RELIABLE " : L"BEST_EFFORT ")
        << theTime.toString() << L" "
        << (receivedOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theReflectInfo.hasProducingFederate ? L"produced by " : L" ")
        << (theReflectInfo.hasProducingFederate ? theReflectInfo.producingFederate.toString() : L"")
        << L"#regions: " << (theReflectInfo.hasSentRegions ? theReflectInfo.sentRegions.size() : 0)
        << L" #attributes: " << theAttributeValues.size() << endl;

    printAttrValues(theAttributeValues);
}

void MyFederateAmbassador::reflectAttributeValues(
    ObjectInstanceHandle theObject,
    AttributeHandleValueMap const& theAttributeValues,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    TransportationType theType,
    LogicalTime const& theTime,
    OrderType receivedOrder,
    MessageRetractionHandle theHandle,
    SupplementalReflectInfo theReflectInfo)
    throw (FederateInternalError)
{
    const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    wcout << L"\nreflectAttributeValues: "
        << myData.objectInstanceMap[theObject] << L"("
        << theObject.toString() << L") "
        << DtToWString(tag) << L" "
        << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theType == RELIABLE ? L"RELIABLE " : L"BEST_EFFORT ")
        << theTime.toString() << L" "
        << (receivedOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << theHandle.toString() << L" "
        << (theReflectInfo.hasProducingFederate ? L"produced by " : L" ")
        << (theReflectInfo.hasProducingFederate ? theReflectInfo.producingFederate.toString() : L"")
        << L"#regions: " << (theReflectInfo.hasSentRegions ? theReflectInfo.sentRegions.size() : 0)
        << L" #attributes: " << theAttributeValues.size() << endl;

    printAttrValues(theAttributeValues);
}

// 6.9
void MyFederateAmbassador::receiveInteraction(
    InteractionClassHandle theInteraction,
    ParameterHandleValueMap const& theParameterValues,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    TransportationType theType,
    SupplementalReceiveInfo theReceiveInfo)
    throw (FederateInternalError)
{
    //const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    /*
    wcout << L"\nreceiveInteraction: "
       << theInteraction.toString() << L" "
       << DtToWString(tag) << L" "
       << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
       << (theType == RELIABLE ? L"RELIABLE " : L"BEST_EFFORT ")
       << (theReceiveInfo.hasProducingFederate ? L"produced by " : L" ")
       << (theReceiveInfo.hasProducingFederate ? theReceiveInfo.producingFederate.toString() : L"")
       << L"#regions: " << (theReceiveInfo.hasSentRegions ? theReceiveInfo.sentRegions.size() : 0)
       << L" #parameters: " << theParameterValues.size() << endl;
    */
    printReceivedInteractionData(theInteraction, theParameterValues);
}

void MyFederateAmbassador::receiveInteraction(
    InteractionClassHandle theInteraction,
    ParameterHandleValueMap const& theParameterValues,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    TransportationType theType,
    LogicalTime const& theTime,
    OrderType receivedOrder,
    SupplementalReceiveInfo theReceiveInfo)
    throw (FederateInternalError)
{
    const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    wcout << L"\nreceiveInteraction: "
        << theInteraction.toString() << L" "
        << DtToWString(tag) << L" "
        << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theType == RELIABLE ? L"RELIABLE " : L"BEST_EFFORT ")
        << theTime.toString() << L" "
        << (receivedOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theReceiveInfo.hasProducingFederate ? L"produced by " : L" ")
        << (theReceiveInfo.hasProducingFederate ? theReceiveInfo.producingFederate.toString() : L"")
        << L"#regions: " << (theReceiveInfo.hasSentRegions ? theReceiveInfo.sentRegions.size() : 0)
        << L" #parameters: " << theParameterValues.size() << endl;

    printReceivedInteractionData(theInteraction, theParameterValues);
}

void MyFederateAmbassador::receiveInteraction(
    InteractionClassHandle theInteraction,
    ParameterHandleValueMap const& theParameterValues,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    TransportationType theType,
    LogicalTime const& theTime,
    OrderType receivedOrder,
    MessageRetractionHandle theHandle,
    SupplementalReceiveInfo theReceiveInfo)
    throw (FederateInternalError)
{
    const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    wcout << L"\nreceiveInteraction: "
        << theInteraction.toString() << L" "
        << DtToWString(tag) << L" "
        << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theType == RELIABLE ? L"RELIABLE " : L"BEST_EFFORT ")
        << theTime.toString() << L" "
        << (receivedOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << theHandle.toString() << L" "
        << (theReceiveInfo.hasProducingFederate ? L"produced by " : L" ")
        << (theReceiveInfo.hasProducingFederate ? theReceiveInfo.producingFederate.toString() : L"")
        << L"#regions: " << (theReceiveInfo.hasSentRegions ? theReceiveInfo.sentRegions.size() : 0)
        << L" #parameters: " << theParameterValues.size() << endl;

    printReceivedInteractionData(theInteraction, theParameterValues);
}

void MyFederateAmbassador::removeObjectInstance(
    ObjectInstanceHandle theObject,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    SupplementalRemoveInfo theRemoveInfo)
    throw (FederateInternalError)
{
    const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    wcout << L"\nremoveObjectInstance: "
        << myData.objectInstanceMap[theObject] << L"("
        << theObject.toString() << L") "
        << DtToWString(tag) << L" "
        << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theRemoveInfo.hasProducingFederate ? L"produced by " : L" ")
        << (theRemoveInfo.hasProducingFederate ? theRemoveInfo.producingFederate.toString() : L"")
        << endl;
}

void MyFederateAmbassador::removeObjectInstance(
    ObjectInstanceHandle theObject,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    LogicalTime const& theTime,
    OrderType receivedOrder,
    SupplementalRemoveInfo theRemoveInfo)
    throw (FederateInternalError)
{
    const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    wcout << L"\nremoveObjectInstance: "
        << myData.objectInstanceMap[theObject] << L"("
        << theObject.toString() << L") "
        << DtToWString(tag) << L" "
        << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << theTime.toString() << L" "
        << (receivedOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << (theRemoveInfo.hasProducingFederate ? L"produced by " : L" ")
        << (theRemoveInfo.hasProducingFederate ? theRemoveInfo.producingFederate.toString() : L"")
        << endl;
}

void MyFederateAmbassador::removeObjectInstance(
    ObjectInstanceHandle theObject,
    VariableLengthData const& theUserSuppliedTag,
    OrderType sentOrder,
    LogicalTime const& theTime,
    OrderType receivedOrder,
    MessageRetractionHandle theHandle,
    SupplementalRemoveInfo theRemoveInfo)
    throw (FederateInternalError)
{
    const char* tag = theUserSuppliedTag.size() ? (const char*)theUserSuppliedTag.data() : "";

    wcout << L"\nremoveObjectInstance: "
        << myData.objectInstanceMap[theObject] << L"("
        << theObject.toString() << L") "
        << DtToWString(tag) << L" "
        << (sentOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << theTime.toString() << L" " << (receivedOrder == RECEIVE ? L"RECEIVE " : L"TIMESTAMP ")
        << theHandle.toString()
        << (theRemoveInfo.hasProducingFederate ? L"produced by " : L" ")
        << (theRemoveInfo.hasProducingFederate ? theRemoveInfo.producingFederate.toString() : L"")
        << endl;
}
