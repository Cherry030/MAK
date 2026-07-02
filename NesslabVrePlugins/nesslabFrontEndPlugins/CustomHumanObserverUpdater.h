#pragma once

#include "export.h"
#include "vrePlayerStation/playerComponent.h"


#include "roles/human/vreHumanFrontend/humanControlLogic.h"//=> DtHumanControlLogic
#include "vrePlayerStation/playerStation.h"
#include "vrePlayerStation/playerStationApp.h"
#include "vrePlayerStation/playerStationTempNotification.h"
#include "vreUtil/initializer.h"
#include "vreUtil/logger.h"
#include <vlutil/vlUtil.h>
#include "vreHumanFrontend/humanObserverUpdater.h"

#include "vrvCore/DtObserver.h"
#include "vrvCore/DtObserverObject.h"
#include <makArchives/DtObserverStateRecord.h>


#include "vrePlayerStation/playerStationStateManager.h"

namespace makVre 
{
    class CustomHumanObserverUpdater : public makVre::DtHumanObserverUpdater
    {
    public:
        CustomHumanObserverUpdater();
        virtual ~CustomHumanObserverUpdater();


        bool initialize(DtPlayerStation* player, DtInitTable& config) override;
        void tick(double dt) override;
        void shutdown() override;
        
    };
}