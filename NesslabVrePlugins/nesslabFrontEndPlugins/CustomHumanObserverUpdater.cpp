#include "CustomHumanObserverUpdater.h"

#include "vreHumanFrontend/humanObserverUpdater.h"


using namespace makVre;

CustomHumanObserverUpdater::CustomHumanObserverUpdater()
    : DtHumanObserverUpdater()
{
    
}


CustomHumanObserverUpdater::~CustomHumanObserverUpdater()
{
    // 커스텀 정리
}

bool CustomHumanObserverUpdater::initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config)
{
    //stateManager
    if (player->app()->stateManager().containsStateType("ENGAGED_STATE"))
    {
        std::cout << "ENGAGED_STATE => True" << std::endl;
    }
    else
    {
        std::cout << "ENGAGED_STATE => False" << std::endl;

        std::cout << "Try init " << std::endl;
        if (!DtHumanObserverUpdater::initialize(player, config))
        {
            std::cout << "ObserverUpdater init failed" << std::endl;
            return false;
        }

    }

    


    

    //std::cout << "Try init " << std::endl;
    //if (!DtHumanObserverUpdater::initialize(player, config))
    //{
    //    std::cout << "ObserverUpdater init failed" << std::endl;
    //    return false;

    //}




    // 추가 초기화
    return true;
}

void CustomHumanObserverUpdater::tick(double dt)
{
    std::cout << "Tick \n";
;    //std::cout << "Tick " << myObserver->name() << std::endl;
    //std::cout << myObserver->observerCameraOrientation() << std::endl;

    //makVre::DtHumanObserverUpdater::tick(dt);
    //myObserver->setAttachType(makVrv::DtObserverObject::AttachTypeTether);
}

void CustomHumanObserverUpdater::shutdown()
{
    // 부모 정리 로직 먼저
    DtHumanObserverUpdater::shutdown();

    // 추가 정리
}

//DtVector CustomHumanObserverUpdater::getCameraOffset()
//{
//    // 예: 부모 구현을 참고하거나 완전히 재정의
//    DtVector baseOffset = DtHumanObserverUpdater::getCameraOffset();
//    // baseOffset.x += 1.0;  // 오프셋 살짝 조정
//    return baseOffset;
//}

//void CustomHumanObserverUpdater::myCustomLogic()
//{
//    // 사용자 정의 동작
//}