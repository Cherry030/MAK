#pragma once


#include <vreVrfmodel/vreSimComponent.h>
#include <vrfobjcore/actuatorComponent.h>
#include <vrfobjcore/platformLocalObjectFacade.h>
#include "vrfobjcore/humanDecorator.h"
#include "vrfutil/kinematicTools.h"
#include <iostream>


class SkBoundingVolume : public DtHumanLocalObjectDecorator
{
public:

	SkBoundingVolume(DtLocalObject* owner);
	~SkBoundingVolume();

	virtual void updateBoundingVolumeBasedOnPosture() override;
	
};

