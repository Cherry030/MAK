#include "MyVreVrfTurretedBalGunPSR.h"


//Constructor => Engage 오브젝트 생성시
MyVreVrfTurretedBalGunPSR::MyVreVrfTurretedBalGunPSR(DtLocalObject* vrfObject,
	const DtString& rwName,
	DtReaderWriterRegistry* parentRegistry)
	:DtVreVrfTurretedBalGunPSR(vrfObject,rwName,parentRegistry)
{
	std::cout << "[MyVreVrfTurretedBalGunPSR][Trace] Constructor " << std::endl;

	myMunitionLoader.init(vrfObject);
}


MyVreVrfTurretedBalGunPSR::MyVreVrfTurretedBalGunPSR(const DtVreVrfTurretedBalGunPSR& orig, 
	const DtString& rwName, 
	DtReaderWriterRegistry* parentRegistry)
	:DtVreVrfTurretedBalGunPSR(orig, rwName, parentRegistry)
{
	std::cout << "[MyVreVrfTurretedBalGunPSR][Trace] Constructor " << std::endl;
}


MyVreVrfTurretedBalGunPSR::~MyVreVrfTurretedBalGunPSR()
{
	std::cout << "[MyVreVrfTurretedBalGunPSR][Trace] Destructor " << std::endl;
}

DtVrfProcessStateRepository* MyVreVrfTurretedBalGunPSR::creator(const DtString& name,
	DtLocalObject* vrfObject, DtReaderWriterRegistry* parentRegistry)
{
	return new MyVreVrfTurretedBalGunPSR(vrfObject, name, parentRegistry);
}


const DtVrfMunitionLoader& MyVreVrfTurretedBalGunPSR::munitionLoader() const
{
	//std::cout << "[MyVreVrfTurretedBalGunPSR][Trace] munitionLoader " << std::endl;
	return myMunitionLoader;
}


DtVrfMunitionLoader& MyVreVrfTurretedBalGunPSR::munitionLoader()
{
	//std::cout << "[MyVreVrfTurretedBalGunPSR][Trace] munitionLoader " << std::endl;
	return myMunitionLoader;
}


DtString MyVreVrfTurretedBalGunPSR::type() const
{
	//return VRE_VRF_TURRETED_BALGUN_PSR;
	return makVre::DtVreVrfTurretedBalGunPSRType;
}