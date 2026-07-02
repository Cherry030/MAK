#pragma once

#include "vrfExtensions/vreVrfobjcore/vreMunitionLoader.h"
#include "Nesslab/nesslabCommon.h"

struct WeaponReloadPacket;


/*
        훈련용 총기(k2) 자동 재장전 방지 플러그인

메모
- 전역 변수(gReloadBlocked)로 제어
- MyVreVrfTurretedBalGunPSR에서 생성

*/


class AutoReloadBlocker : public makVre::DtVreMunitionLoader
{
public:

    AutoReloadBlocker(const DtString& rwName = DtString::nullString(),
        DtReaderWriterRegistry* parentRegistry = 0);


    AutoReloadBlocker(const DtVreMunitionLoader& orig,
        const DtString& rwName = DtString::nullString(),
        DtReaderWriterRegistry* parentRegistry = 0);


    virtual bool init(DtLocalObject* entity) override;


    virtual ~AutoReloadBlocker();


    // Overridden to lower the gun while loading and raise once done.
    virtual void tick(const DtSimMunition& correctMunition,
        DtDetonatorFuze correctFuze, DtTime currentTime,
        int roundsAvailable = 1) override;



private:
    bool initPlugin = false;
    bool isTrainingWeapon = false;

};