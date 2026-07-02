/*******************************************************************************
** Copyright (c) 2020 MAK Technologies
** All rights reserved.
*******************************************************************************/

// exampleHumanArtPartActuator.h

// DtExampleHumanArtPartActuator; acutator that demonstrates publishing
// human character articulated parts representing joints on a DI-Guy
// character.  It rotates the left shoulder of a given character. 
#pragma once

#include <vrfobjcore/actuatorComponent.h>
#include <vrfobjcore/platformLocalObjectFacade.h>
#include "vrfMsgTransport/diGuyActionEvent.h"

//251001
#include "TCPServer.h"
#include <vrvCore/DtDe.h>
#include "vrfutil/rwDIGuyCharacterInfo.h"

//260119
#include "Nesslab/nesslabCommon.h"

using namespace makVrfEvents;
//전방 선언./////////////////////.
namespace ysu_to_vris_packets 
{
    struct HandposeLabelPacket;
}

class DtVrfMovingObjectStateRepository;


/*
// 수신호 라벨 인덱스 //==============================================================================================
namespace nesslab_enums
{
    enum HandGestureLabel
    {
        Hold = 1,
        Pointing,
        CustomTest,

        //TODO : 추후에 List 확립 시, 추가 정의

        Count   //enum 개수 확인용
    };

    enum DeviceID
    {
        //VR-Engage
        Vris = 0x71,

        Ipms = 0x53,
        Spms = 0x67,

        Ifes = 0x55,
        Sfes = 0x69,

        Iars = 0x54,
        Sars = 0x68,

        Ihrs = 0x56,
        Shrs = 0x66,

        Stms = 0x01,
        Mtms = 0x02,

        Hagh = 0x81,

        //RFID
        Rfid = 0xca,

    };

    enum class HandposeMsgID
    {
        InfraHandposeInfoMsgID = 0x59,
        SemiInfraHandposeInfoMsgID = 0x6c,
    };
}
//==================================================================================================================

namespace plugin_common
{
// 기본 구조 패킷 //================================================================================================
    constexpr uint8_t PACKET_STX = 0x02;
    constexpr uint8_t PACKET_ETX = 0x03;

#pragma pack(push, 1)            // → 구조체 멤버들을 1byte 단위로 정렬해서 패딩 없이 붙여서 저장 (Stack에 저장)
    struct PacketHeader
    {
        std::uint8_t        stx = PACKET_STX;
        std::uint8_t        deviceID = 0;
        std::uint8_t        msgID = 0;
        std::uint8_t        msgType = 0;
        std::uint16_t       length = 0;
    };

    struct PacketTail
    {
        std::uint8_t        etx = PACKET_ETX;
    };
#pragma pack(pop)                // → 이전에 저장해둔 정렬 규칙을 복원 (이대로 꺼내겠다)

    static const uint8_t PACKET_HEADER_SIZE = sizeof(PacketHeader);

    template<class Frame>
    inline std::size_t PayloadSize() {
        return sizeof(Frame) - sizeof(PacketHeader) - sizeof(PacketTail);
    }
//==================================================================================================================
    
// 패킷 구조 //=====================================================================================================
    struct PacketHandler {
        std::uint8_t    deviceID;
        std::uint8_t    msgID;
        void* frameStorage;// Parsed frame data will be written here before calling onEvent
        std::uint16_t   payloadLen;//payload Length
        std::uint16_t   frameSize;//sizeof(Frame)

        std::function<void()> onEvent;//Callback Event
    };

    using PacketHandlers = std::vector<PacketHandler>;
//==================================================================================================================
}
*/

namespace makVre
{
    //constexpr char DtExampleHumanArtPartActuatorType[] = "vre-example-human-art-part-actuator";
    constexpr char HANDPOSE_PLUGIN_TYPE[] = "vre-human-handpose-actuator";
}

namespace nesslab_backend_plugins
{
    //Type 경로 → C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\platforms => Skeleton_with_Humanpose.ope
    //const char HANDPOSE_PLUGIN_TYPE[] = "vre-human-handpose-actuator";

   class HandposePlugin : public DtActuatorComponent
   {

   public:

      //! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
      //! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
       HandposePlugin(const DtString& name,
         DtLocalObject* owner,
         DtSimulationServices* simManager,
         DtComponentDescriptor* desc = 0,
         DtReaderWriterRegistry* parentRegistry = 0);

      //! default constructor; not implemented
       HandposePlugin() = delete;

      //! copy constructor; not implemented
       HandposePlugin(const HandposePlugin& orig) = delete;

      //! assignment operator; not implemented
      const HandposePlugin& operator=(const HandposePlugin& orig) = delete;

      //! destructor
      virtual ~HandposePlugin();

   public:
       virtual bool init() override;

       //! \copydoc DtSimComponent::type()
       //! \return DtExampleHumanArtPartActuatorType
       virtual const char* type() const override;

       // Looks up the left shoulder articulated part and updates the elevation angle and rate on
       // that part.
       virtual void tick() override;

       //! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
       //! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
       //! \return New instance of this class.
       static DtSimComponent* creator(const DtString& name,
           DtLocalObject* owner,
           DtSimulationServices* simManager,
           DtComponentDescriptor* desc = 0,
           DtReaderWriterRegistry* parentRegistry = 0);

       //void HandleHandposeLabel(int handIndex, int newHandposeLabel);
       void HandleHandposeLabel(int newHandposeLabel_L, int newHandposeLabel_R);

   protected:

       //! Member to allow for access to platform specific data
       DtPlatformLocalObjectFacade myPlatformLocalObjectFacade;

       makVrf::DtArticulatedPartStateRepository* myLeftShoulder;
       makVrf::DtArticulatedPartStateRepository* myRightShoulder;
       makVrf::DtArticulatedPartStateRepository* myLeftWrist;
       

       double myAnimationInterval;
       double myTimeInInterval;

       DtVrfMovingObjectStateRepository* myLocalState;

   private:
   //public :

       void InitPlugin();

       //Callback Function
       void OnDataReceived(const string& clientAddr, const uint8_t* data, int len);
       void OnHandposeLabelPkt(const ysu_to_vris_packets::HandposeLabelPacket& packet);

       int latestHandposeLabel_L = -1;
       int latestHandposeLabel_R = -1;

       void SetGesture(int handIndex, string strGesture);

       /*
      void InitRightHandAction();
      void setRightHandAction(int index);

      void InitPointAction();
      void setPointAction();

      void InitGestureAction(DtDiGuyActionEvent* currentGesture);
      void setEndGestureAction();
      void setTestGestureAction();

      void setHoldGestureAction(bool isLeft);
      void setPointingGestureAction();
      void setEndHoldGestureAction();

      void ParseCommand(std::string recvString);

       DtRwInt GetHitPropertyValue();
       DtRwInt GetMissPropertyValue();
       DtRwInt GetHandposePropertyValue();
       void SetHandposePropertyValue(DtRwInt leftHandpose);
       */
       thread netThread;
   };
   /*
   void TCPServerConnected(string clientIP);
   void OnDataReceived(BYTE* data, int len);

   void ParseReceiveBuffer(BYTE* data);
   */
}
