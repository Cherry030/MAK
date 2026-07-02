#include "SkBoundingVolume.h"
#include <iostream>


extern float gSkHeadPosZ;

SkBoundingVolume::SkBoundingVolume(DtLocalObject* owner) : DtHumanLocalObjectDecorator(owner) {}


SkBoundingVolume::~SkBoundingVolume() {}

//void SkBoundingVolume::tick() {}


//TODO: 누워있는 상태에서 BoundingVolume 높이 맞춰주기
void SkBoundingVolume::updateBoundingVolumeBasedOnPosture()
{
	/*
		Q&A 답변 정리
		//boundingVolume → 객체의 충돌 감지 및 공간 내 위치를 판별하는 데 사용됨.
		//myPostureUsedForBoundingVolume → 현재 적용된 자세.
		//determinePostureForBoundingVolume() → 현재 상태에서 새로운 자세를 결정하는 함수.
		//boundingVolume.setHalfLengthWidthHeight() → 바운딩 볼륨 크기를 변경하는 함수.
		//boundingVolume.setLatticeOffset() → 바운딩 볼륨의 위치를 조정하는 함수.
		//myLocalObject->setNextFrameBoundingVolume(boundingVolume) → 다음 프레임에서 적용할 바운딩 볼륨을 설정.

		body zone maps => standing and one for prone
			   => entity posture로 결정(기존)
	*/

	

	const DtVrfObjectParameters* params = myLocalObject->parameters();
	Dt3dBoundingVolume boundingVolume(*params->boundingVolume());
	double halfLength = boundingVolume.halfLength();//0.2
	double halfWidth  = boundingVolume.halfWidth();//0.3
	double halfHeight = boundingVolume.halfHeight();//0.91,  Height:1.82
	
	bool groundCenter = false;

	// In the case of ground center, need to adjust lattice offset to accommodate new bounding volume shape
	if (boundingVolume.latticeOffset().x() == 0.0
		&& boundingVolume.latticeOffset().y() == 0.0
		&& DtALMOST(boundingVolume.latticeOffset().z(), -boundingVolume.halfHeight(), 0.001))
	{
		groundCenter = true;
	}


	//0~ 1.82
	if (gSkHeadPosZ > 1.05)
	{

		//setHalfLengthWidthHeight() => 사용하면 offset기준 위아래로 half만큼 높이 증가 ex) 1값이 들어오면 위아래로 1씩 증가
		boundingVolume.setHalfLengthWidthHeight(halfWidth, halfWidth, gSkHeadPosZ * 0.5);

		if (groundCenter)
		{
			boundingVolume.setLatticeOffset(DtVector(0., 0., -gSkHeadPosZ * 0.5));
		}

	}
	else//임시 => gSkHeadPosZ가 1.05 밑으로내려가면 누워있다고 가정
	{

		boundingVolume.setHalfLengthWidthHeight(halfHeight, halfWidth, halfLength);//Z = 0.2

		if (groundCenter)
		{
			boundingVolume.setLatticeOffset(DtVector(0., 0., -boundingVolume.halfHeight()));
		}
	}

	myLocalObject->setNextFrameBoundingVolume(boundingVolume);
}