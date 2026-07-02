/*******************************************************************************
** Copyright (c) 2018 MAK Technologies, Inc.
** All rights reserved.
*******************************************************************************/
//#define KCLTEST 1
//#if DtHLA

#include "configFomMap.h"

#include <vl/stateEncoderFactory.h>
#include <vl/stateDecoderFactory.h>
#include <vl/fom.h>
#include <vl/fomMapper.h>
#include <vrfExtObjects/extEntityStateRepository.h>
#include <vrfExtObjects/extendedAttributesInitializer.h>
#include <vrfExtObjects/vrfEntityPublisher.h>
#include <vrfExtObjects/reflectedExtEntityList.h>

#include <vl/exerciseConn.h>
#include <vl/exerciseConnInitializer.h>
#include <vl/reflectedEntityList.h>
#include <vl/entityStateRepository.h>
#include <vl/reflectedEntity.h>
#include <vl/fireInteraction.h>
#include <vl/topoView.h>
#include <vl/fomMapper.h>
#include <vl/environmentProcessRepository.h>
#include <vl/reflectedEnvironmentProcess.h>
#include <vl/reflectedEnvironmentProcessList.h>
#include <vl/signalInteraction.h>
#include <vl/setDataInteractionHLA.h>

#include <vlutil/vlProcessControl.h>
#include <vlutil/vlMiniDumper.h>
#include <vlutil/vlStringUtil.h>
#include <vlutil/vlSimpleKeyboard.h>
#include <vl/setDataInteractionHLA.h>

//[ Fom Interaction Class]
// > Send
#include "extTraineeSensor.h"
#include "extTraineeSensorEncoder.h"
#include "extTraineeSensorDecoder.h"
/*
#include "extLogInInfo.h"
#include "extLogInInfoEncoder.h"
#include "extLogInInfoDecoder.h"
*/

// < Receive
#include "extTrainingCtrl.h"
#include "extTrainingCtrlEncoder.h"
#include "extTrainingCtrlDecoder.h"

#include "TCPClass.h"

DtExerciseConn* m_pExCon;
const DtString theFomClass = "AlternateEntity"; 

TCPClass* titsTcpClass = nullptr;
std::thread* pDataProcessingThread = nullptr;

void OnTraineeSensorCB(extTraineeSensor* inter, void* usr)
{
    SetConsoleOutputCP(949);
    std::cout << "\n[KCL] Trainee State Data" << std::endl;
    std::cout << "[KCL] Publish Type       " << GetString_Publisher(inter->PublisherType()) << std::endl;
    std::cout << "[KCL] Device ID           " << inter->deviceID() << std::endl;
    //std::cout <<  "[KCL] Trainee ID         " << inter->TraineeID() << std::endl;
    std::cout << "[KCL] Heartbeat          " << inter->Heartbeat()<< std::endl;
    std::cout << "[KCL] Concentration      " << inter->Concentrativeness() << std::endl;
    std::cout << "[KCL] Stress             " << inter->Stress() << std::endl;
    std::cout << "[KCL] Fatigue            " << inter->Fatigue() << std::endl;
}
/*
void OnLogInInfoCB(extLogInInfo* inter, void* usr)
{
    SetConsoleOutputCP(949);
    std::cout << "\n[KCL] Send Login Info" << std::endl;
    std::cout << "[KCL] Publisher Type      " << inter->publisherType() << std::endl;
    std::cout << "[KCL] Device ID           " << inter->deviceID() << std::endl;
    std::cout << "[KCL] Service Number      " << GetString_DtNetLogInInfo10(inter->serviceNumber()) << std::endl;
    std::cout << "[KCL] Login Name          " << GetString_DtNetLogInInfo10(inter->logInName()) << std::endl;
    std::cout << "[KCL] Affiliation         " << GetString_DtNetLogInInfo10(inter->affiliation()) << std::endl;
}
*/
void OnTrainingCtrlCB(extTrainingCtrl* inter, void* usr)
{
    SetConsoleOutputCP(949);
    std::cout << "\n[KCL] Receive Training Control Message" << std::endl;
    std::cout << "[KCL] RealWorldTime      " << inter->getRealWorldTime() << std::endl;
    std::cout << "[KCL] SimulationTime          " << inter->getSimulationTime() << std::endl;
    std::cout << "[KCL] ControlMessage         " << inter->getControlMessage() << std::endl;
    std::cout << "       : [0]종료 [1]준비 [2]시작 [3]일시정지" << std::endl;
}

std::string GetString_Publisher(unsigned short type)
{
    if (type == 0)
        return "Subject 1";
    else if (type == 1)
        return "Subject 2";
    else if (type == 2)
        return "Subject 3";
    else
        return "Unknown Subject" + type;
}

std::string GetString_DeviceStatus(unsigned char state)
{
    if (state == 0x00)
        return "Not Connected";
    else if (state == 0x01)
        return "Normal";
    else if (state == 0x02)
        return "Error";
    else
        return "Unknown State ";// +state;
}

std::string  GetString_DtNetLogInInfo10(DtNetLogInInfo10 info)
{
    size_t len = strnlen(reinterpret_cast<const char*>(info.DtText), 10);
    std::string str(reinterpret_cast<const char*>(info.DtText), len);

    return str;
}
/*
DtNetLogInInfo10  GetDtNetLogInInfo10_String(const std::string& str)
{
    DtNetLogInInfo10 info{};

    size_t len = str.size();
    if (len > 10) len = 10;

    memcpy(info.DtText, str.data(), len);

    return info;
}
*/

void configFomMapper(DtExerciseConn* exConn, bool runExtPropInit)
{
    DtFom* fom = exConn->fom();
    DtFomMapper* mapper = exConn->fomMapper();
    DtInfo << "[KCL] Start Config Fom Mapper\n";
    m_pExCon = exConn;

    DtExerciseConn::InitializationStatus status = DtExerciseConn::DtINIT_SUCCESS;
    DtClock* clock = exConn->clock();

    DtInfo << "[KCL] [" << clock->simTime() << ",  " << " 0] Initializing\n";
    exConn->setExecutionStatus(vrlstatus_initializing);
    exConn->drainInput(0.1);

    DtInfo << "[KCL] Exercise connection : " << runExtPropInit << "\n";

    if (status != 0)
        DtInfo << "[KCL] Error creating exercise connection.\n";


    //extTraineeSensor::addCallback(exConn, OnTraineeSensorCB, NULL);
    //DtInfo << "[KCL] Add Trainee State Callback\n";

    extTrainingCtrl::addCallback(exConn, OnTrainingCtrlCB, NULL);
    DtInfo << "[KCL] Add Training Control Callback\n";

    std::cout << "[" << clock->simTime() << ", " << " 0] Executing " << std::endl;
    exConn->setExecutionStatus(vrlstatus_executing);
    exConn->drainInput(0.1);

    DtExtendedAttributesInitializerPtr init(new DtEntityExtendedAttributesInitializer);
    init->addSupportedClass(theFomClass);


    DtVrfEntityPublisher::setInitializer(init);
    DtReflectedExtEntityList::setInitializer(init);

    if (runExtPropInit)
    {
        init->initialize(exConn);
    }

    titsTcpClass = new TCPClass;
    std::cout << "[INFO] TITS TCP 접속 시도중...\n";
    titsTcpClass->Start();
    if (!pDataProcessingThread)
    {
        pDataProcessingThread = new std::thread(&ProcessIncomingData);
    }
}

void unconfigFomMapper(DtExerciseConn* exConn)
{
    DtFom* fom = exConn->fom();
    DtFomMapper* mapper = exConn->fomMapper();

    DtDebug << "[KCL] End Config Fom Mapper\n";


    // Remove the Extended Attributes Initializer when the plugin is removed.
    DtVrfEntityPublisher::setInitializer(DtExtendedAttributesInitializerPtr());
    DtReflectedExtEntityList::setInitializer(DtExtendedAttributesInitializerPtr());
}

void SendTraineeSensor(unsigned short usPublisherType, unsigned short usDeviceID, byte heartbeat, byte concentration, byte stress, byte fatigue)
{
    extTraineeSensor inter;

    inter.setPublisherType(DtNetU16(usPublisherType));
    inter.setDeviceID(DtNetU16(usDeviceID));
    inter.setHeartbeat(heartbeat);
    inter.setConcentrativeness(concentration);
    inter.setStress(stress);
    inter.setFatigue(fatigue);

    m_pExCon->sendStamped(inter);
}
/*
void SendLoginInfo(unsigned short usPublisherType, unsigned short usDeviceID, string strServiceNumber, string strLoginName, string strAffiliation)
{
    extLogInInfo inter;

    inter.setPublisherType(DtNetU16(usPublisherType));
    inter.setDeviceID(DtNetU16(usDeviceID));

    DtNetLogInInfo10 serviceNumber;
    memcpy(serviceNumber.DtText, strServiceNumber.c_str(), 10);

    inter.setServiceNumber(GetDtNetLogInInfo10_String(strServiceNumber));
    inter.setLogInName(GetDtNetLogInInfo10_String(strLoginName));
    inter.setAffiliation(GetDtNetLogInInfo10_String(strAffiliation));

    m_pExCon->sendStamped(inter);
}
*/

void ProcessIncomingData()
{
    while (true)
    {
        if (titsTcpClass->HasData())
        {
            std::vector<unsigned char> data = titsTcpClass->GetNextData();

            if (!data.empty())
            {
                if (data[0] == 1) 
                {
                    SendTraineeSensor(1, 1, 120, 3, 2, 1);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
//#endif

