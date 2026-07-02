#pragma once

#pragma warning(disable: 4251)
#pragma warning(disable: 4786)
#pragma warning(disable: 4290)


#include <RTI/RTI1516.h>
#include <RTI/NullFederateAmbassador.h>
#include <string>
#include <map>

class DtSimpleData
{
public:
    bool connectionLost;
    bool listFederationsReturned;
    bool nameReservationReturned;
    bool nameReservationSucceeded;

    std::map<rti1516e::ObjectClassHandle, std::wstring> objectClassMap;
    std::map<rti1516e::ObjectInstanceHandle, std::wstring> objectInstanceMap;

    DtSimpleData()
    {
        clear();
    }

    void clear()
    {
        connectionLost =
            listFederationsReturned =
            nameReservationReturned =
            nameReservationSucceeded = false;
        objectClassMap.clear();
        objectInstanceMap.clear();
    }
};

class MyFederateAmbassador : public rti1516e::NullFederateAmbassador
{
public:

    MyFederateAmbassador(DtSimpleData& data);

    virtual ~MyFederateAmbassador()
        throw ();

    // 4.4
    virtual void connectionLost(
        std::wstring const& faultDescription)
        throw (rti1516e::FederateInternalError);

    // 4.8
    virtual void reportFederationExecutions(
        rti1516e::FederationExecutionInformationVector const& theFederationExecutionInformationList)
        throw (rti1516e::FederateInternalError);

    // 6.3
    virtual void objectInstanceNameReservationSucceeded(
        std::wstring const& theObjectInstanceName)
        throw (rti1516e::FederateInternalError);

    virtual void objectInstanceNameReservationFailed(
        std::wstring const& theObjectInstanceName)
        throw (rti1516e::FederateInternalError);

    // 6.9
    virtual void discoverObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::ObjectClassHandle theObjectClass,
        std::wstring const& theObjectInstanceName)
        throw (rti1516e::FederateInternalError);

    virtual void discoverObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::ObjectClassHandle theObjectClass,
        std::wstring const& theObjectInstanceName,
        rti1516e::FederateHandle producingFederate)
        throw (rti1516e::FederateInternalError);

    // 6.11
    virtual void reflectAttributeValues(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleValueMap const& theAttributeValues,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::TransportationType theType,
        rti1516e::SupplementalReflectInfo theReflectInfo)
        throw (rti1516e::FederateInternalError);

    virtual void reflectAttributeValues(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleValueMap const& theAttributeValues,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::TransportationType theType,
        rti1516e::LogicalTime const& theTime,
        rti1516e::OrderType receivedOrder,
        rti1516e::SupplementalReflectInfo theReflectInfo)
        throw (rti1516e::FederateInternalError);

    virtual void reflectAttributeValues(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleValueMap const& theAttributeValues,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::TransportationType theType,
        rti1516e::LogicalTime const& theTime,
        rti1516e::OrderType receivedOrder,
        rti1516e::MessageRetractionHandle theHandle,
        rti1516e::SupplementalReflectInfo theReflectInfo)
        throw (rti1516e::FederateInternalError);

    // 6.13
    virtual void receiveInteraction(
        rti1516e::InteractionClassHandle theInteraction,
        rti1516e::ParameterHandleValueMap const& theParameterValues,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::TransportationType theType,
        rti1516e::SupplementalReceiveInfo theReceiveInfo)
        throw (rti1516e::FederateInternalError);

    virtual void receiveInteraction(
        rti1516e::InteractionClassHandle theInteraction,
        rti1516e::ParameterHandleValueMap const& theParameterValues,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::TransportationType theType,
        rti1516e::LogicalTime const& theTime,
        rti1516e::OrderType receivedOrder,
        rti1516e::SupplementalReceiveInfo theReceiveInfo)
        throw (rti1516e::FederateInternalError);

    virtual void receiveInteraction(
        rti1516e::InteractionClassHandle theInteraction,
        rti1516e::ParameterHandleValueMap const& theParameterValues,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::TransportationType theType,
        rti1516e::LogicalTime const& theTime,
        rti1516e::OrderType receivedOrder,
        rti1516e::MessageRetractionHandle theHandle,
        rti1516e::SupplementalReceiveInfo theReceiveInfo)
        throw (rti1516e::FederateInternalError);

    // 6.15
    virtual void removeObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::SupplementalRemoveInfo theRemoveInfo)
        throw (rti1516e::FederateInternalError);

    virtual void removeObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::LogicalTime const& theTime,
        rti1516e::OrderType receivedOrder,
        rti1516e::SupplementalRemoveInfo theRemoveInfo)
        throw (rti1516e::FederateInternalError);

    virtual void removeObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::LogicalTime const& theTime,
        rti1516e::OrderType receivedOrder,
        rti1516e::MessageRetractionHandle theHandle,
        rti1516e::SupplementalRemoveInfo theRemoveInfo)
        throw (rti1516e::FederateInternalError);

public:
    DtSimpleData& myData;
};

