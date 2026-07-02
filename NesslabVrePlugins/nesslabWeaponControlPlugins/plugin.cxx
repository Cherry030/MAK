/*******************************************************************************
** Copyright (c) 2020 MAK Technologies
** All rights reserved.
*******************************************************************************/

//#include "exampleHumanDamageActuator.h"

#include <vreUtil/version.h>
#include <vrfcgf/vrfPluginExtension.h>
#include <vrfcgf/cgf.h>
#include <vrfcgf/factoryManager.h>
#include <vrfcgf/vrfCreator.h>
#include <vrfobjcore/simComponentTypes.h>

#include "GrenadeCollisionActuator.h"
#include "MyVreVrfTurretedBalGunPSR.h"
#include "MyHumanGunJoyActuator.h"


//Test
//#include "HumanGunJoyActuatorCmds.h"
//#include "ForceFireCmd.h"


using namespace makVre;

extern "C" {

    void DT_VRF_DLL_PLUGIN DtPluginInformation(DtVrfPluginInformation& info)
    {
        info.pluginName = "nesslabWeaponControlSim";
        info.pluginVersion = DtVreVersionNumber;
        info.pluginCreator = "Nesslab";
    }



   DT_VRF_DLL_PLUGIN bool DtInitializeVrfPlugin(DtCgf* cgf)
   {
       // Register VR-Engage component types with the factory

      return true;
   }

   // Post-initialization (optional)
   DT_VRF_DLL_PLUGIN bool DtPostInitializeVrfPlugin(DtCgf* cgf)
   {

       //총기 자동재장전 방지
       cgf->factoryManager()->processStateRepositoryFactory()->addCreatorFcn(makVre::DtVreVrfTurretedBalGunPSRType, MyVreVrfTurretedBalGunPSR::creator);

       //총기 Actuator 제어(탄 개수 제어, 재장전)
       cgf->factoryManager()->componentFactory()->addCreatorFcn(makVre::DtHumanGunJoyActuatorType, MyHumanGunJoyActuator::creator);

       //수류탄 충돌 제어(벽 충돌 에러 관련 플러그인) -> VRE 2.2 에서 에러 수정 완료
       //cgf->factoryManager()->componentFactory()->addCreatorFcn(DtFrictionlessGravityCollisionsActuatorType, nesslab_backend_plugins::GrenadeCollisionActuator::creator);


      return true;
   }

}


/*
{
    //테스트 진행중
    //msg 등록해서 수신받으면 handler에서 처리하는 방식
    //기본 구조
    // 1. msg 등록
    // 2. handler 등록
    // 3. msg 송신
    // 여기서는 격발 등록하는거 테스트 진행
    // msg 등록     => msgFactory.registerMessage<makVre::ForceFireMessage>();
    // handler 등록 => HumanGunJoyActuatorCmds.cpp 에서 handler 등록
    // msg 송신     =>            DtConsoleCommandManager::globalConsoleCommandManager()->addCommand("ForceFire", new DtForceFireCommand(cgf->simObjectManager()));
    // 다른 simPlugin(TerrainSoilPlugin.cpp)에서 지형 변경되면 격발msg 송신 테스트 해봤는데 잘 동작함

    // Additional setup after all plugins loaded.
    // Register our newly created messages.
    makVre::DtVreMessageFactory& msgFactory = makVre::DtVreMessageManager::instance().factory();
    msgFactory.registerMessage<makVre::ForceFireMessage>();



    //WARN[VR-Engage Sim] 08:12:15.508 DtVrfApp::processKeyboardInput: command 'ForceFire' failed



    //※※ addCommand해주면 사용하는 플러그인쪽에서 반드시 DtConsoleCommandManager::globalConsoleCommandManager()->removeCommand("ForceFire"); 해줘야함(없으면 에러)
    //ex)콘솔에 ForceFire 123e4567-e89b-12d3-a456-426614174000 입력하면 => DtForceFireCommand::execute(123e4567-e89b-12d3-a456-426614174000)
    //   함수로 호출하려면 		std::string fireCommand = "ForceFire " + entity()->uuid().uuidString(); DtConsoleCommandManager::globalConsoleCommandManager()->runCommand(fireCommand)
    //Command등록 addCommand(commandStr, command)
    DtConsoleCommandManager::globalConsoleCommandManager()->addCommand("ForceFire",
        new DtForceFireCommand(cgf->simObjectManager()));


    //cgf->factoryManager()->componentFactory()->print();
    //cgf->factoryManager()->processStateRepositoryFactory()->print();
}
*/