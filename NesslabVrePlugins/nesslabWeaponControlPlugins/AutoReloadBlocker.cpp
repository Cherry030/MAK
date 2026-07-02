#include "AutoReloadBlocker.h"
#include "Nesslab/nesslabTcpServer.h"
#include <vreUtil/logger.h>

using namespace nesslab_common;

extern bool gReloadBlocked;

extern const char* gK2MunitionType;



AutoReloadBlocker::AutoReloadBlocker(const DtString& rwName,
	DtReaderWriterRegistry* parentRegistry)
	: makVre::DtVreMunitionLoader(rwName, parentRegistry)
{
	std::cout << "[AutoReloadBlocker][Trace] Constructor \n";
}

AutoReloadBlocker::AutoReloadBlocker(const DtVreMunitionLoader& orig, const DtString& rwName, DtReaderWriterRegistry* parentRegistry)
	:makVre::DtVreMunitionLoader(orig, rwName, parentRegistry)
{
	std::cout << "[AutoReloadBlocker][Trace] Constructor \n";
}

bool AutoReloadBlocker::init(DtLocalObject* entity)
{
	if (!DtVreMunitionLoader::init(entity))
	{
		return false;
	}
	std::cout << "[AutoReloadBlocker][Trace] init Plugin  \n";

	return true;
}

AutoReloadBlocker::~AutoReloadBlocker()
{
	std::cout << "[AutoReloadBlocker][Trace] Destructor \n";

}

/*
모의총기 플러그인(FE)에서 재장전 안하는 이유
- MAK에서 재장전을 처리하면 총알수를 0으로 변경시킨다음 최대로 변경
-> 탄수가 0이면 재장전방지 플러그인(BE)에서 재장전 처리 막아서 안됨
*/
void AutoReloadBlocker::tick(const DtSimMunition& correctMunition,
	DtDetonatorFuze correctFuze,
	DtTime currentTime,
	int roundsAvailable)
{

	//M16_MUNITION_TYPE 
	//gK2MunitionType

	//한번만 실행
	if (!initPlugin)
	{

		//k2 소총만 자동재장전 막기(권총은 전자탄창x)
		//VR-Forces 5.2d Simulation Object Editor에서 타입확인가능
		//K2-556mm -> 2:8:225:2:1:1:2
		//K5-9mm   -> 2:8:225:2:3:0:1
		if (correctMunition.munitionType() == gK2MunitionType)
		{
			//std::cout << "[AutoReloadBlocker][Debug] name => "				<< correctMunition.name()			<< std::endl;
			//std::cout << "[AutoReloadBlocker][Debug] munitionType => "		<< correctMunition.munitionType()	<< std::endl;
			//std::cout << "[AutoReloadBlocker][Debug] myRoundsPerMagazine => "	<< myRoundsPerMagazine				<< std::endl;
			//std::cout << "[AutoReloadBlocker][Debug] roundsAvailable => "		<< roundsAvailable					<< std::endl;
			//std::cout << "[AutoReloadBlocker][Debug] isTrainingWeapon: "		<< std::boolalpha					<< isTrainingWeapon << std::endl;
			
			isTrainingWeapon = true;
		}

		initPlugin = true;
	}



	//훈련용 무기(k2)인지 확인
	if (isTrainingWeapon)
	{

		if (myRoundsLeftInMagazine == 0)
		{
		 
			//자동 재장전 방지
			if (gReloadBlocked)
				return;

			//MyHumanGunJoyActuator.h에서 재장전 신호 받으면 gReloadBlocked = false
			//한틱에 재장전 안되서 여기서 tick 몇 번 처리하고 장전되면 gReloadBlocked = true처리
			makVre::DtVreMunitionLoader::tick(correctMunition, correctFuze, currentTime, roundsAvailable);
			return;
		}

		if (!gReloadBlocked)
			gReloadBlocked = true;
	}




	makVre::DtVreMunitionLoader::tick(correctMunition, correctFuze, currentTime, roundsAvailable);
}