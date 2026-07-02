//#define IS_NESSLAB
#include "HandposePlugin.h"
#include <vrvDiGuy\DtDiGuyArtPartLinkMapper.h>
#include <tdbutil\mathUtilities.h>

#include <sstream>
#include <iomanip>
#include "vrvDiGuy/DtDiGuyModelInstance.h"

#include "vrfobjcore/simObject.h"
#include <vrfobjcore\vrfMovingObjectStateRepository.h>

// Nesslab
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab//nesslabTcpClient.h"
#include <utilities/vreUtil/logger.h>


using namespace std;
using namespace makVrf;
using namespace makVrfEvents;
using namespace nesslab_common;

namespace ysu_to_vris_packets
{
#pragma pack(push, 1)
    struct HandposeLabelPacket
    {
        PacketHeader header;

        //BYTE handIndex = 0;              // 왼손 → 0 오른손 → 1
        //BYTE handLabel = 0;              // 수신호 라벨
        BYTE handLabel_L = 0;              // 왼손 수신호 라벨
        BYTE handLabel_R = 0;              // 오른손 수신호 라벨

        PacketTrailer trailer;

        HandposeLabelPacket()
        {
            header.deviceID = 0;        //IHRS( 0x56 )  SHRS( 0x66 )
            header.msgID = 0;           //IHRS( 0x59 )  SHRS( 0x6c )
            header.length = sizeof(HandposeLabelPacket); //0x09
        }
    };
#pragma pack(pop)
}

namespace nesslab_backend_plugins
{
    // Sender   : YSU → Sub VRIS
    //                     ↓
    // Receiver :         VRIS

    namespace
    {
        // 행동(Master) 장비 <----> Sub_VRIS <--TCP-- VRIS, Port = 4010(임의)
        nesslabTcpServer*       handposeTCPServer = nullptr;
        int                     handposePort = -1;
        ysu_to_vris_packets::HandposeLabelPacket     handposeDataPkt;

        DtDiGuyActionEvent leftHandActionEvent;
        DtDiGuyActionEvent rightHandActionEvent;

        vector<BYTE> handposeRecvBuffer;
        PacketHandlers packetHandlers;
    }

    enum LANG { KOR, ENG };
    string GetHandposeLableName(int idx, LANG language)
    {
        if (language == LANG::KOR)
        {
            switch (idx)
            {
            case 0:
                return "-";
            case 1:     
                return "1 : 일";
            case 2:   
                return "2 : 이";
            case 3:    
                return "3 : 삼";
            case 4:    
                return "4 : 사";
            case 5:    
                return "5 : 오";
            case 6:     
                return "6 : 주먹";
            case 7:    
                return "7 : 손바닥";
            case 8:    
                return "8 : 엄지 핀 손바닥";
            case 9:    
                return "9 : 엄지";
            case 10:
                return "10 : 약속";
            case 11:   
                return "11 : 검지, 약지";
            case 12: 
                return "12 : 그랩";
                /*
            case 1:
                return "산개";
            case 2:
                return "집합(Assemble)";
            case 3:
                return "집합(Join me)";
            case 4:
                return "증속";
            case 5:
                return "Wedge Formation";
            case 6:
                return "가스";
            case 7:
                return "착검";
            case 8:
                return "적 발견";
            case 9:
                return "신속하게";
            case 10:
                return "정지";
            case 11:
                return "무릎 앉아";
            case 12:
                return "포복으로 이동";
            case 13:
                return "Pace Count";
            case 14:
                return "통신병 앞으로";
            case 15:
                return "Head count";
            case 16:
                return "위험지역 도착";
            case 17:
                return "움직이지마!";
            case 18:
                return "정지! 사주경계";
            case 19:
                return "수신완료";
            case 20:
                return "사격개시/(기관총)사격속도 조절";
            case 21:
                return "방향 또는 고각 변경";
            case 22:
                return "사격 위치 이동/화력 조정";
            case 23:
                return "사격중지";
                */
            case 252:
                return "End Gesture";
            case 253:
                return "테스트 제스처(2-3-5-1)";
            case 254:
                return "Hold";
            case 255:
                return "Custom 제스처";
            default:
                return "Unknown";
            }
        }
        else
        {
            switch (idx)
            {
            case 0:
                return "-";
            case 1:     // 주먹
                return "dst_hand_1";    //dst_hand_freeze
            case 2:     // 1
                return "dst_hand_2";    //dst_hand_enemy_in_sight
            case 3:     // 2
                return "dst_hand_3";    //
            case 4:     // 3
                return "dst_hand_4";    //dst_hand_change_direction
            case 5:     // 4
                return "dst_hand_5";    //
            case 6:     // 5
                return "dst_hand_6";    //
            case 7:     // 손바닥
                return "dst_hand_7";    //dst_hand_head_count
            case 8:     // 엄지 핀 손바닥
                return "dst_hand_8";    //dst_hand_SLLS
            case 9:     // 엄지
                return "dst_hand_9";    //dst_hand_message_acknowledged
            case 10:    // 약속
                return "dst_hand_10";   //dst_hand_radio_operator_forward
            case 11:    // 검지 약지
                return "dst_hand_11";   //dst_hand_wedge_formation
            case 12:    // 그랩
                return "dst_hand_12";   //
                //for test
            case 252:
                return "End Gesture";
            case 253:
                return "test_diguy_hand";
            case 254:
                return "dst_hand_hold";
            case 255:
                return "custom_hand_wedge_formation";
                /*
            case 1:
                return "dst_hand_disperse";
            case 2:
                return "dst_hand_assemble";
            case 3:
                return "dst_hand_join_me";
            case 4:
                return "dst_hand_increase_speed";
            case 5:
                return "dst_hand_wedge_formation";
            case 6:
                return "dst_hand_chemical_attack";
            case 7:
                return "dst_hand_fix_bayonets";
            case 8:
                return "dst_hand_enemy_in_sight";
            case 9:
                return "dst_hand_quick_time";
            case 10:
                return "dst_hand_halt";
            case 11:
                return "dst_hand_take_a_knee";
            case 12:
                return "dst_hand_move_prone";
            case 13:
                return "dst_hand_pace_count";
            case 14:
                return "dst_hand_radio_operator_forward";
            case 15:
                return "dst_hand_head_count";
            case 16:
                return "dst_hand_danger_area";
            case 17:
                return "dst_hand_freeze";
            case 18:
                return "dst_hand_SLLS";
            case 19:
                return "dst_hand_message_acknowledged";
            case 20:
                return "dst_hand_commence_firing";
            case 21:
                return "dst_hand_change_direction";
            case 22:
                return "dst_hand_shift_fire";
            case 23:
                return "dst_hand_cease_firing";
            case 252:
                return "End Gesture";
            case 253:
                return "test_diguy_hand";
            case 254:
                return "dst_hand_hold";
            case 255:
                return "custom_hand_wedge_formation";
            */
            default:
                return "default";
            }
        }
    };


    makVrf::DtArticulatedPartStateRepository* myRightWrist = nullptr;
    ostringstream handRight;
    ostringstream point("point");
    ostringstream testGesture("test_diguy_hand_l");

    //kcl 251229
    ostringstream holdGesture_L("dst_hand_hold_l");
    ostringstream holdGesture_R("dst_hand_hold_r");
    ostringstream pointingGesture("diguy_signal_join_me");

    DtDiGuyActionEvent myRightHandAction;
    DtDiGuyActionEvent myPointAction;
    DtDiGuyActionEvent myTestGestureAction;

    //kcl 251229
    DtDiGuyActionEvent myHoldGestureAction_L;
    DtDiGuyActionEvent myHoldGestureAction_R;
    DtDiGuyActionEvent myPointingGestureAction;

    ostringstream latestGesture;
    DtDiGuyActionEvent cleanGestureAction;


    HandposePlugin* myOwner;

    //Constructor => Enage Human 스켈레톤 캐릭터 생성하면 실행
    HandposePlugin::HandposePlugin(const DtString& name,
      DtLocalObject* owner,
      DtSimulationServices* simManager,
      DtComponentDescriptor* desc,
      DtReaderWriterRegistry* parentRegistry)
      : DtActuatorComponent(name, owner, simManager, desc, parentRegistry)
      , myPlatformLocalObjectFacade(owner)
      , myLeftShoulder(0)
      , myRightShoulder(0)
        , myLeftWrist(0)
      , myAnimationInterval(4)
      , myTimeInInterval(0)
   {
        cout << "[HandposePlugin][Trace] Constructor " << endl;
   }

    // destructor => Human 아닌 오브젝트를 생성하거나 다른 인간 캐릭터 생성하면 실행
    HandposePlugin::~HandposePlugin()
   {
        cout << "[HandposePlugin][Trace] Destructor " << entity()->uuid().string() << endl;

        if (netThread.joinable())
            netThread.join();

        if (handposeTCPServer != nullptr)
        {
            handposeTCPServer->ServerStop();
            delete handposeTCPServer;
            handposeTCPServer = nullptr;
        }
   }

   bool HandposePlugin::init()
   {
       cout << "[HandposePlugin][Trace] Initialize " << endl;
      if (!DtActuatorComponent::init())
      {
         return false;
      }

      myOwner = this;
      myLocalState = localStateRepository()->as<DtVrfMovingObjectStateRepository>();

      InitPlugin();
      
      return true;
   }

   const char* HandposePlugin::type() const
   {
      return makVre::HANDPOSE_PLUGIN_TYPE;
   }

   void HandposePlugin::InitPlugin()
   {
       //Initialize buffer
       handposeRecvBuffer.clear();
       packetHandlers.clear();

       //Get ConfigFile Data
       {
           //handpose
           handposePort = GetPrivateProfileInt("YSU_Handpose", "handposePort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

           if (handposePort == INI_INT_NOT_FOUND_DEFAULT)
               LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[HandposePlugin][Error] Failed to read data from INI file. Using default value.\n";


           std::cout << "[HandposePlugin][Debug] hapticPort: " << handposePort << std::endl;
       }

       //Store packet data(파싱, 콜백 처리용)
       {
           //인프라 수신호 패킷
           RegisterPacketHandler<ysu_to_vris_packets::HandposeLabelPacket>(packetHandlers,
                                 (int)DeviceID::Ihrs,
                                 (uint8_t)HandposeMsgID::InfraHandposeInfoMsgID,
                                 handposeDataPkt,
                                 [this](const ysu_to_vris_packets::HandposeLabelPacket& packet)
                                 { OnHandposeLabelPkt(packet); });

           //세미인프라 수신호 패킷
           RegisterPacketHandler<ysu_to_vris_packets::HandposeLabelPacket>(packetHandlers,
                                (int)DeviceID::Shrs,
                                (uint8_t)HandposeMsgID::SemiInfraHandposeInfoMsgID,
                                handposeDataPkt,
                                [this](const ysu_to_vris_packets::HandposeLabelPacket& packet)
                                { OnHandposeLabelPkt(packet); });
       }

       //Create and start TCP/UDP
       {
           netThread = thread([this]()
               {
                   handposeTCPServer = new nesslabTcpServer(handposePort);
                   handposeTCPServer->SetOnDataReceived([this](const string& client, const uint8_t* data, int len)
                                                        {OnDataReceived(client, data, len); });
                   handposeTCPServer->ServerStart();
               });
       }
   }

   /*
   void HandposePlugin::OnHandposeLabelPkt(const ysu_to_vris_packets::HandposeLabelPacket& packet)
   {
       //마지막 라벨과 다를때만 처리 → Handpose는 중복된게 들어와도 처리해야함
       cout << "OnHandposeLabel Packet : "  << (int)packet.handIndex << ", " << (int)packet.handLabel << endl;
       // Left
       if (packet.handIndex == 0)//if (packet.handIndex == 0 && latestHandposeLabel_L != packet.handLabel)
       {
           latestHandposeLabel_L = packet.handLabel;
           HandleHandposeLabel(packet.handIndex, packet.handLabel);
       }

       //Right
       if (packet.handIndex == 1)//if (packet.handIndex == 1 && latestHandposeLabel_R != packet.handLabel)
       {
           latestHandposeLabel_R = packet.handLabel;
           HandleHandposeLabel(packet.handIndex, packet.handLabel);
       }
   }
   */
   void HandposePlugin::OnHandposeLabelPkt(const ysu_to_vris_packets::HandposeLabelPacket& packet)
   {
       //마지막 라벨과 다를때만 처리 → Handpose는 중복된게 들어와도 처리해야함

       cout << "OnHandposeLabel Packet : " << (int)packet.handLabel_L << ", " << (int)packet.handLabel_R << endl;
       // Left
       if (packet.handLabel_L == 0)//if (packet.handIndex == 0 && latestHandposeLabel_L != packet.handLabel)
       {
           latestHandposeLabel_L = packet.handLabel_L;
       }

       //Right
       if (packet.handLabel_R == 0)//if (packet.handIndex == 1 && latestHandposeLabel_R != packet.handLabel)
       {
           latestHandposeLabel_R = packet.handLabel_R;
       }

       HandleHandposeLabel(packet.handLabel_L, packet.handLabel_R);
   }


   void HandposePlugin::OnDataReceived(const string& clientAddr, const uint8_t* data, int len)
   {
       handposeRecvBuffer.insert(handposeRecvBuffer.end(), data, data + len);
       ParseReceiveBuffer(packetHandlers, handposeRecvBuffer);
   }
   /*
   void HandposePlugin::HandleHandposeLabel(int handIndex, int newHandposeLabel)
   {
       //HandposeLableIndex label = static_cast<HandposeLableIndex>(newHandposeLabel);
       cout << "[수신호 데이터][TRACE] " << GetHandposeLableName(newHandposeLabel, LANG::KOR) << endl;

       ostringstream labelStream("");
       labelStream << GetHandposeLableName(newHandposeLabel, LANG::ENG)
                   << ((handIndex == 0) ? "_l" : "_r");

       string strGesture = labelStream.str();
       cout << "[수신호 데이터][TRACE] ENG " << strGesture << endl;
       
       SetGesture(handIndex, strGesture);
   }
   */
   void HandposePlugin::HandleHandposeLabel(int newHandposeLabel_L, int newHandposeLabel_R)
   {
       //HandposeLableIndex label = static_cast<HandposeLableIndex>(newHandposeLabel);
       cout << "[수신호 데이터][TRACE] 왼손 : " << GetHandposeLableName(newHandposeLabel_L, LANG::KOR) << "  오른손 : " << GetHandposeLableName(newHandposeLabel_R, LANG::KOR) << endl;

       if (newHandposeLabel_L > 0)
       {
           ostringstream labelStream_L("");
           labelStream_L << GetHandposeLableName(newHandposeLabel_L, LANG::ENG) << "_l";
           string strGesture_L = labelStream_L.str();
           cout << "[수신호 데이터][TRACE] ENG 왼손 : " << strGesture_L;

           SetGesture(0, strGesture_L);
       }

       if (newHandposeLabel_R > 0)
       {
           ostringstream labelStream_R("");
           labelStream_R << GetHandposeLableName(newHandposeLabel_R, LANG::ENG) << "_r";
           string strGesture_R = labelStream_R.str();
           cout << "  오른손 : " << strGesture_R << endl;

           SetGesture(1, strGesture_R);
       }
   }

   void HandposePlugin::SetGesture(int handIndex, string strGesture)
   {
       DtDiGuyActionEvent actionEvent;
       if (strGesture == "End Gesture_l" || strGesture == "End Gesture_r")
       {
           strGesture = "test_diguy_hand";
           actionEvent.setAction(DtDiGuyActionEvent::StartGesture);
           actionEvent.setEntityId(entity()->uuid());
           actionEvent.setReps(0);
           actionEvent.setOverallDuration(0);
           ostringstream tmp(strGesture);
           actionEvent.setGesture(tmp.str());
       }
       else
       {
           actionEvent.setAction(DtDiGuyActionEvent::StartGesture);
           actionEvent.setEntityId(entity()->uuid());
           actionEvent.setReps(1);
           actionEvent.setOverallDuration(60);
           //actionEvent.setOverallDuration(60);
           //actionEvent.setGesture(strGesture);
           ostringstream tmp(strGesture);
           actionEvent.setGesture(tmp.str());
       }
       /*
       actionEvent.setAction(DtDiGuyActionEvent::StartGesture);
       actionEvent.setEntityId(entity()->uuid());
       actionEvent.setReps(1);
       actionEvent.setOverallDuration(60);
       //actionEvent.setGesture(strGesture);
       ostringstream tmp(strGesture);
       actionEvent.setGesture(tmp.str());
       */
       simulationServices()->addEvent(actionEvent);

       (handIndex == 0 ? leftHandActionEvent : rightHandActionEvent) = actionEvent;

       cout << "[HandposePlugin][Trace] Send Set Gesture Event : " << strGesture << endl;
   }


   DtSimComponent* HandposePlugin::creator(const DtString& name,
      DtLocalObject* owner,
      DtSimulationServices* simManager,
      DtComponentDescriptor* desc,
      DtReaderWriterRegistry* parentRegistry)
   {
      return new HandposePlugin(name, owner, simManager, desc, parentRegistry);
   }


   double myPartAngle = -1.57;
   double myPartAngleRate = 0;

   void HandposePlugin::tick()
   {
      if (!simulationServices())
      {
         objectConsoleError()("Uninitialized simulation services detected by component: %s\n",
            name().c_str());
         return;
      }
      ///*
      if (DtIsZero<double>(dT()))
      {
         // When the simulation is paused, stop the rotation of the elbow.
         if (myLeftShoulder)
         {
            myLeftShoulder->setAzimuthRate(0);
            myLeftShoulder->setElevationRate(0);
            myLeftShoulder->setRotationRate(0);
         }

         if (myRightShoulder)
         {
             myRightShoulder->setAzimuthRate(0);
             myRightShoulder->setElevationRate(0);
             myRightShoulder->setRotationRate(0);
         }

         if (myLeftWrist)
         {
             myLeftWrist->setAzimuthRate(0);
             myLeftWrist->setElevationRate(0);
             myLeftWrist->setRotationRate(0);
         }

         return;
      }

      if (!myLeftShoulder)
          myLeftShoulder = myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartShoulderLeft));      

      if (!myLeftShoulder)
          return;

      if(!myLeftWrist)
          myLeftWrist = myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartWristLeft));
      

      if (!myRightShoulder)
          myRightShoulder = myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartShoulderRight));
      if (!myRightShoulder)
          return;
      //*/
      /*
      // Animate the elevation of the joint sinusoidally from minAngle to maxAngle over myAnimationInterval.
      double maxAngle = M_PI_2;
      myTimeInInterval += dT();
      if (myTimeInInterval > myAnimationInterval) {
         myTimeInInterval = 0;
      }

      double theta = myTimeInInterval / myAnimationInterval * 2 * M_PI;

      // Compute a new angle and angular rate for the part this tick.
      double newPartAngle = maxAngle * sin(theta);
      double newPartAngleRate = (4 * maxAngle / myAnimationInterval) * cos(theta);
      */
      ///*
      // Update the art part.
      myLeftShoulder->setAzimuth(0);
      myLeftShoulder->setAzimuthRate(0);
      myLeftShoulder->setElevation(myPartAngle);
      myLeftShoulder->setElevationRate(myPartAngleRate);
      myLeftShoulder->setRotation(0);
      myLeftShoulder->setRotationRate(0);

      myRightShoulder->setAzimuth(0);
      myRightShoulder->setAzimuthRate(0);
      myRightShoulder->setElevation(myPartAngle);
      myRightShoulder->setElevationRate(myPartAngleRate);
      myRightShoulder->setRotation(0);
      myRightShoulder->setRotationRate(0);

      myLeftWrist->setAzimuth(0);
      myLeftWrist->setAzimuthRate(0);
      myLeftWrist->setElevation(myPartAngle);
      myLeftWrist->setElevationRate(myPartAngleRate);
      myLeftWrist->setRotation(0);
      myLeftWrist->setRotationRate(0);
      //*/
      return;
   }


#if def IS_NESSLAB
   vector<BYTE> handGestureBuffer = {};
   void HandposePlugin::OnDataReceived(BYTE* data, int len)
   //void HandposePlugin::ParseReceiveBuffer(BYTE* data)
   void ParseReceiveBuffer(BYTE* data)
   {
       int index = data[0];

       if (index == 255)//ff
       {
           myOwner->InitPointAction();
           myOwner->setPointAction();
       }
       else if (index == 250)//fa
       {
           myOwner->InitGestureAction(&myHoldGestureAction_R);
           myOwner->setHoldGestureAction(false);
           myOwner->InitGestureAction(&myHoldGestureAction_L);
           myOwner->setHoldGestureAction(true);
       }
       else if (index == 251)//fb
       {
           myOwner->InitGestureAction(&myHoldGestureAction_R);
           myOwner->setHoldGestureAction(false);
       }
       else if (index == 252)//fc
       {
           myOwner->InitGestureAction(&myHoldGestureAction_L);
           myOwner->setHoldGestureAction(true);

           myOwner->SetHandposePropertyValue(2);
       }
       else if (index == 253)//fd
       {
           myOwner->InitGestureAction(&myPointingGestureAction);
           myOwner->setPointingGestureAction();
       }
       else if (index == 254)//fe
       {
           myOwner->InitGestureAction(&myTestGestureAction);
           myOwner->setTestGestureAction();
       }
       else if (index == 1)
       {
           cout << "[INFO] Hit Value : " << myOwner->GetHitPropertyValue() << endl;
       }
       else if (index == 2)
       {
           cout << "[INFO] Miss Value : " << myOwner->GetMissPropertyValue() << endl;
       }
       else if (index == 3)
       {
           cout << "[INFO] Hand Pose Value : " << myOwner->GetHandposePropertyValue() << endl;
       }
       else if (index == 4)
       {
           myOwner->setEndGestureAction(); 
       }
       else if (index == 5)
       {
           myOwner->setEndHoldGestureAction();
       }
       else
       {
           myOwner->InitRightHandAction();
           myOwner->setRightHandAction(index);
       }
       /*
       if (index == 255)//ff
       {
           InitPointAction();
           setPointAction();
       }
       else if (index == 250)//fa
       {
           InitGestureAction(&myHoldGestureAction_R);
           setHoldGestureAction(false);
           InitGestureAction(&myHoldGestureAction_L);
           setHoldGestureAction(true);
       }
       else if (index == 251)//fb
       {
           InitGestureAction(&myHoldGestureAction_R);
           setHoldGestureAction(false);
       }
       else if (index == 252)//fc
       {
           InitGestureAction(&myHoldGestureAction_L);
           setHoldGestureAction(true);

           SetHandposePropertyValue(2);
       }
       else if (index == 253)//fd
       {
           InitGestureAction(&myPointingGestureAction);
           setPointingGestureAction();
       }
       else if (index == 254)//fe
       {
           InitGestureAction(&myTestGestureAction);
           setTestGestureAction();
       }
       else if (index == 1)
       {
           cout << "[INFO] Hit Value : " << GetHitPropertyValue() << endl;
       }
       else if (index == 2)
       {
           cout << "[INFO] Miss Value : " << GetMissPropertyValue() << endl;
       }
       else if (index == 3)
       {
           cout << "[INFO] Hand Pose Value : " << GetHandposePropertyValue() << endl;
       }
       else if (index == 4)
       {
           setEndGestureAction();
       }
       else
       {
           myOwner->InitRightHandAction();
           myOwner->setRightHandAction(index);
       }
       */
   }


   DtRwInt HandposePlugin::GetHitPropertyValue()
   {
       DtRwInt val;

       boost::optional<int> optVal = entity()->nextFrameStateProperties().findPropertyValue<int>("TargetsHit");
       if (optVal)
       {
           cout << "[ INFO ] findPropertyValue <TargetsHit> : " << *optVal << endl;
           val = optVal.value();
       }
       return val;
   }

   DtRwInt HandposePlugin::GetMissPropertyValue()
   {
       DtRwInt val;

       boost::optional<int> optVal = entity()->nextFrameStateProperties().findPropertyValue<int>("TargetsMissed");
       if (optVal)
       {
           cout << "[ INFO ] findPropertyValue <TargetsMissed> : " << *optVal << endl;
           val = optVal.value();
       }
       /*
       DtRwInt* missProperty = entity()->nextFrameStateProperties().findProperty<DtRwInt>("TargetsMissed");

       if (!missProperty->invalid())
           val = missProperty->value();
       */
       return val;
   }

   DtRwInt HandposePlugin::GetHandposePropertyValue()
   {
       DtRwInt val;

       boost::optional<int> optVal = entity()->nextFrameStateProperties().findPropertyValue<int>("Handpose");
       if (optVal)
       {
           cout << "[ INFO ] findPropertyValue <Handpose> : " << *optVal << endl;
           val = optVal.value();
       }

       return val;
   }

   void HandposePlugin::SetHandposePropertyValue(DtRwInt leftHandpose)
   {
       DtRwInt* handposeProperty = entity()->nextFrameStateProperties().findProperty<DtRwInt>("Handpose");

       if (handposeProperty)
       {
           handposeProperty->setValue(leftHandpose);
           std::cout << "[INFO] Set Handpose index : " << leftHandpose << std::endl;
       }
   }

   void HandposePlugin::InitRightHandAction()
   {
       cout << "[INFO] Init Right Hand Action " << endl;

       myRightHandAction.setAction(DtDiGuyActionEvent::StartGesture);
       myRightHandAction.setEntityId(entity()->uuid());
       myRightHandAction.setReps(1);
       myRightHandAction.setOverallDuration(60);
       myRightHandAction.setOverallDuration(60);
   }
   
   void HandposePlugin::setRightHandAction(int index)
   {
       handRight.str("");
       handRight << "diguy_hand_l_" << std::setw(2) << std::setfill('0') << index;

       myRightHandAction.setGesture(handRight.str());				

       simulationServices()->addEvent(myRightHandAction);

       cout << "[INFO] Set Right Hand Action : " << handRight.str() << endl;
   }

   /*-------------------------------------------------------------------------------------------------------------------------*/

   void HandposePlugin::InitPointAction()
   {
       cout << "[INFO] Init Point Action " << endl;

       myPointAction.setAction(DtDiGuyActionEvent::StartGesture);
       myPointAction.setEntityId(entity()->uuid());
       myPointAction.setReps(1);
       myPointAction.setOverallDuration(60);
   }

   void HandposePlugin::setPointAction()
   {
       myPointAction.setGesture(point.str());	

       latestGesture.str("");
       latestGesture << point.str();

       simulationServices()->addEvent(myPointAction);

       cout << "[INFO] Set Point Action : " << point.str() << endl;
   }

   /*-------------------------------------------------------------------------------------------------------------------------*/

   void HandposePlugin::InitGestureAction(DtDiGuyActionEvent* currentGesture)
   {
       cout << "[INFO] Init Gesture : " << currentGesture->gesture() << endl;

       currentGesture->setAction(DtDiGuyActionEvent::StartGesture);
       currentGesture->setEntityId(entity()->uuid());
       currentGesture->setReps(1);
       currentGesture->setOverallDuration(60.0f);
   }

   void HandposePlugin::setEndGestureAction()
   {
       cleanGestureAction.setAction(DtDiGuyActionEvent::EndGesture);
       cleanGestureAction.setEntityId(entity()->uuid());
       cleanGestureAction.setRampdownTime(0);
       cout << "[INFO] End Gesture rampdown ( current : " << cleanGestureAction.gesture() << " )" << endl;
       cleanGestureAction.setGesture(latestGesture.str());
       cout << "[INFO] End Gesture rampdown - setGesture(  " << latestGesture.str() << " )" << endl;
       simulationServices()->addEvent(cleanGestureAction);
   }

   void HandposePlugin::setEndHoldGestureAction()
   {
       myHoldGestureAction_L.setAction(DtDiGuyActionEvent::EndGesture);
       myHoldGestureAction_L.setEntityId(entity()->uuid());
       myHoldGestureAction_L.setRampdownTime(0);
       myHoldGestureAction_L.setGesture(holdGesture_L.str());
       cout << "[INFO] End Gesture - setGesture(  " << holdGesture_L.str() << " )" << endl;
       simulationServices()->addEvent(myHoldGestureAction_L);
   }

   void HandposePlugin::setTestGestureAction()
   {
       myTestGestureAction.setGesture(testGesture.str());	

       latestGesture.str("");
       latestGesture << testGesture.str();


       simulationServices()->addEvent(myTestGestureAction);

       cout << "[INFO] action : " << myTestGestureAction.action() << endl;
       cout << "[INFO] reps : " << myTestGestureAction.reps() << endl;
       cout << "[INFO] rampdownTime : " << myTestGestureAction.rampdownTime() << endl;
       cout << "[INFO] overallDuration : " << myTestGestureAction.overallDuration() << endl;
       
       cout << "[INFO] Custom Test Gesture : " << testGesture.str() << endl;
   }

   void HandposePlugin::setHoldGestureAction(bool isLeft)
   {   
       if (isLeft)
       {
           myHoldGestureAction_L.setGesture(holdGesture_L.str());

           latestGesture.str("");
           latestGesture << holdGesture_L.str();

           simulationServices()->addEvent(myHoldGestureAction_L);

           cout << "[INFO] Hold Gesture (왼) : " << holdGesture_L.str() << endl;
       }
       else
       {
           myHoldGestureAction_R.setGesture(holdGesture_R.str());

           latestGesture.str("");
           latestGesture << holdGesture_R.str();

           simulationServices()->addEvent(myHoldGestureAction_R);

           cout << "[INFO] Hold Gesture (오) : " << holdGesture_R.str() << endl;
       }
   }

   void HandposePlugin::setPointingGestureAction()
   {
       myPointingGestureAction.setGesture(pointingGesture.str());

       latestGesture.str("");
       latestGesture << pointingGesture.str();

       simulationServices()->addEvent(myPointingGestureAction);

       cout << "[INFO] Pointing Gesture : " << pointingGesture.str() << endl;
   }
   
       
   /*-------------------------------------------------------------------------------------------------------------------------*/

   void HandposePlugin::ParseCommand(std::string recvString)
   {
       switch (recvString[0])
       {

       case 'c':
           cout << "[INFO] Character Name : [" << myOwner->myLocalState->diguyCharacterName() << "]" << endl;
           break;
           //
       case 'a':
           cout << "[INFO] Appearance Name : [" << myOwner->myLocalState->diguyAppearance() << "]" << endl;
           break;
       case 't':
           //myOwner->myLocalState->diguyGraphicsLink::set_shape_switch_override(29);
           break;
       default:
           break;
       }
   }


   //void HandposePlugin::TCPServerConnected(string clientIP)
   void TCPServerConnected(string clientIP)
   {
       cout << "[INFO] Client Connected [" << clientIP << "]" << endl;
   }
#endif
}
