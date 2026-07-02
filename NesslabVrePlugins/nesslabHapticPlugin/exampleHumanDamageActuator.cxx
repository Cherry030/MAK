/*******************************************************************************
** Copyright (c) 2020 MAK Technologies
** All rights reserved.
*******************************************************************************/

// Example includes
#include "exampleHumanDamageActuator.h"
#include <tdbutil\mathUtilities.h>

#include <vreUtil/assert.h>
#include <vreUtil/logger.h>

#include <vrfobjcore/detonationContext.h>
#include <vrfobjcore/humanStateRepository.h>
#include <vrfobjcore/attachedTerrain.h>
#include <vrfutil/kinematicTools.h>

using namespace makVrf;

namespace makVre
{
	// constructor
	DtExampleHumanDamageActuator::DtExampleHumanDamageActuator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc,
		DtReaderWriterRegistry* parentRegistry)
		: DtVreHumanDamageActuator(name, owner, simManager, desc, parentRegistry)
	{
		std::cout << "DtExampleHumanDamageActuator" << std::endl;
	}

	// destructor
	DtExampleHumanDamageActuator::~DtExampleHumanDamageActuator()
	{
		std::cout << "~DtExampleHumanDamageActuator" << std::endl;
	}

	// This returns a string from compTypes.h, and identifies the
	// type of component
	const char* DtExampleHumanDamageActuator::type() const
	{
		return DtExampleHumanDamageActuatorType;
	}






	bool DtExampleHumanDamageActuator::adjudicateCollateralDamage(const DtDetonationContext& detonationContext)
	{
		if (detonationContext.detonationInter().munitionType().domain() == 8)
		{
			//Weapon Type => Gun

			DtDetonationContext workingDetonationContext = detonationContext;
			DtDamageZoneInfo damageZoneInfo;
			if (getClosestDamageZone(workingDetonationContext, damageZoneInfo))
			{
				std::string bodyZoneName = damageZoneInfo.bodyZoneName;
			}
		}
		else
		{
			//Weapon Type => Bomb
			std::string bodyZoneName = "All";
		}

		return DtVreHumanDamageActuator::adjudicateCollateralDamage(detonationContext);

		/*
		//std::cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡTestㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << std::endl;

		//detonationContext;


		//std::cout << "adjudicateCollateralDamage" << std::endl;
		//std::cout << "isDirectFire() => " << detonationContext.isDirectFire() << std::endl;
		//std::cout << "eventId => " << detonationContext.eventId()  << std::endl;
		//std::cout << "surface => " << detonationContext.surface() << std::endl;
		//std::cout << "range => " << detonationContext.range()  << std::endl;

		//
		//detonationContext.munition();
		//detonationContext.detonationPowers();
		//detonationContext.detonationInter();

		//std::cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << std::endl;;

		std::cout << "detonationContext=> " << detonationContext.munition().debugText() << std::endl;
		std::cout << "detonationContext=> " << detonationContext.munition().munitionName() << std::endl;
		std::cout << "detonationContext=> " << detonationContext.munition().munitionType() << std::endl;
		std::cout << "detonationContext=> " << detonationContext.munition().name() << std::endl;



		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

		//1.폭발처리

		//if (!isPlayerControlled("Human")) return false;

		//실제 데미지 처리 전 Health
		DtRwInt Health        = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();
		//DtRwInt overallHealth = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();

		//실제 데미지 처리
		bool retVal = DtVreHumanDamageActuator::adjudicateCollateralDamage(detonationContext);
		std::cout << "retVal => " << retVal << std::endl;


		//데미지 처리 후 Health
		DtRwInt currentHealth = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();


		//데미지 처리(정확하게 해야하는지 여쭤보기)
		int attackDamage = Health - currentHealth;
		if (attackDamage == 0) return false;



		if (retVal)
		{
			DtDamageState damageState = myHumanLocalObjectFacade.nextFrameDamageState();

			//DtRwInt currentHealth = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();

			if (damageState != 0)
			{
				std::cout << "-----Indirect weapon hit-----" << "\n";
				std::cout
					<< "damage : " << attackDamage << "\n"
					<< "damageState(0~3) : " << damageState << "\n"
					<< "Health : " << currentHealth << std::endl;
				std::cout << "--------------" << std::endl;

				return retVal;
			}
		}

		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

		//2. 직접 피해

		DtHumanStateRepository* myHumanStateRepository = dynamic_cast<DtHumanStateRepository*>(entity()->internalState());
		const Coordinate_System* coordinateSystemPtr = terrainAttachedTo()->coordinateSystem();
		const DtVector& entityLocalLocation = entity()->localPosition();
		DtVector detonationEntityLocation = detonationContext.detonationInter().entityLocation();


		// Get the surface and impact angle of the hit on the entity's bounding box.
		// Surface name is one of the following:  "front", "left-side", "right-side", "top", "bottom", "rear"
		// Create a working copy of the incoming detonation context, because it will modified
		// and in this call.
		DtDetonationContext workingDetonationContext = detonationContext;
		getDirectFireSurfaceAndAngleOfIncidence(workingDetonationContext);



		// Get the damage data for the body zone that was hit.  
		DtDamageZoneInfo damageZoneInfo;
		if (getClosestDamageZone(workingDetonationContext, damageZoneInfo))
		{
			LOG_INFO("EXAMPLE") << std::setprecision(2) <<
				"Direct hit - " <<
				"surface:  " << workingDetonationContext.surface().c_str() << " " <<
				"angle of incidence:  " << DtRad2Deg(workingDetonationContext.angleOfIncidence()) << " "
				"body location:  " << workingDetonationContext.detonationInter().entityLocation() << " "
				"body zone:  " << damageZoneInfo.bodyZoneName << std::endl;



			//DtRwInt currentHealth = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();
			//std::cout << "adjudicateDirectDamage" << std::endl;
			//std::cout << "body zone:  " << damageZoneInfo.bodyZoneName << std::endl;
			//std::cout << "Health : " << currentHealth << std::endl;
			////std::cout << "Health : " << entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value() << "\n";

			DtRwInt Health = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();
			std::cout << "----- Direct hit -----" << "\n";
			std::cout
				<< "surface:  " << workingDetonationContext.surface().c_str() << "\t"
				<< "angle of incidence:  " << DtRad2Deg(workingDetonationContext.angleOfIncidence()) << "\t"
				<< "body location:  " << workingDetonationContext.detonationInter().entityLocation() << "\t"
				//head, torso-front, torso-left, torso-right,  torso-rear,   legs
				<< "body zone:  " << damageZoneInfo.bodyZoneName << "\n"

				//attacker_Id
				<< "attackerId_uuidString : " << workingDetonationContext.detonationInter().attackerId().uuidString() << "\t"

				//target_Id
				<< "targetId_uuidString : " << workingDetonationContext.detonationInter().targetId().uuidString() << "\t"

				//Damage_Body_Zone_Name
				<< "body zone : " << damageZoneInfo.bodyZoneName << "\t"

				//Entity_localPosition
				//<< "entityLocalLocation : " << entity()->localPosition() << "\n"

				//damage
				<< "damage : " << attackDamage << "\t"

				//Player Health
				//<< "\n" << "myOverallHealth : " << myOverallHealth->value()
				//<< "Health : " << entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value() << "\n"
				<< "Health : " << currentHealth << "\n"


				<< "--------------" << std::endl;
		}


		//bool retVal = DtVreHumanDamageActuator::adjudicateCollateralDamage(detonationContext);



		//std::cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << std::endl;;



		return retVal;
		*/

	}
	




	/*
	bool DtExampleHumanDamageActuator::adjudicateCollateralDamage(const DtDetonationContext& detonationContext)
	{

		std::cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << std::endl;

		detonationContext;


		std::cout << "adjudicateCollateralDamage" << std::endl;
		std::cout << "isDirectFire() => " << detonationContext.isDirectFire() << std::endl;

		detonationContext.detonationPowers();
		detonationContext.detonationInter();

		std::cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << std::endl;;


		//if (!isPlayerControlled("Human")) return false;

		DtRwInt overallHealth = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();

		//bool retVal = DtHumanDamageActuator::adjudicateCollateralDamage(detonationContext);
		bool retVal = DtVreHumanDamageActuator::adjudicateCollateralDamage(detonationContext);

		std::cout << "retVal => " << retVal << std::endl;


		if (retVal)
		{
			DtDamageState damageState = myHumanLocalObjectFacade.nextFrameDamageState();

			DtRwInt currentHealth = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();

			std::cout << "-----Dump-----" << "\n";
			std::cout << "Indirect weapon hit - " << "\t"
				<< "damage : " << overallHealth - currentHealth << "\t"
				<< "damageState(0~3) : " << damageState << "\t"
				<< "Health : " << currentHealth << std::endl;
			std::cout << "--------------" << std::endl;

		}


		return retVal;
	}
	*/




	/*
	bool DtExampleHumanDamageActuator::adjudicateDirectDamage(const DtDetonationContext& detonationContext)
	{

		std::cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << std::endl;
		std::cout << "detonationContext=> " << detonationContext.munition().debugText().c_str() << std::endl;
		std::cout << "adjudicateDirectDamage" << std::endl;
		std::cout << "detonationContext=> " << detonationContext.munition().munitionName().string() << std::endl;

		std::cout << "detonationContext2=> " << detonationContext.munition().munitionType().category()<< std::endl;
		std::cout << "detonationContext2=> " << detonationContext.munition().munitionType().string() << std::endl;
		std::cout << "detonationContext2=> " << detonationContext.munition().munitionType().country() << std::endl;
		std::cout << "detonationContext2=> " << detonationContext.munition().munitionType().subCategory() << std::endl;


		std::cout << "detonationContext=> " << detonationContext.munition().name().string() << std::endl;


		std::cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << std::endl;;


	   DtHumanStateRepository* myHumanStateRepository = dynamic_cast<DtHumanStateRepository*>(entity()->internalState());
	   const Coordinate_System* coordinateSystemPtr = terrainAttachedTo()->coordinateSystem();
	   const DtVector& entityLocalLocation = entity()->localPosition();
	   DtVector detonationEntityLocation = detonationContext.detonationInter().entityLocation();

	   // Get the surface and impact angle of the hit on the entity's bounding box.
	   // Surface name is one of the following:  "front", "left-side", "right-side", "top", "bottom", "rear"
	   // Create a working copy of the incoming detonation context, because it will modified
	   // and in this call.
	   DtDetonationContext workingDetonationContext = detonationContext;
	   getDirectFireSurfaceAndAngleOfIncidence(workingDetonationContext);

	   // Get the damage data for the body zone that was hit.
	   DtDamageZoneInfo damageZoneInfo;
	   if (getClosestDamageZone(workingDetonationContext, damageZoneInfo))
	   {
		  LOG_INFO("EXAMPLE") << std::setprecision(2) <<
			 "Direct hit - " <<
			 "surface:  " << workingDetonationContext.surface().c_str() << " " <<
			 "angle of incidence:  " << DtRad2Deg(workingDetonationContext.angleOfIncidence()) << " "
			 "body location:  " << workingDetonationContext.detonationInter().entityLocation() << " "
			 "body zone:  " << damageZoneInfo.bodyZoneName << std::endl;


		  std::cout << "body zone:  " << damageZoneInfo.bodyZoneName << std::endl;
		  std::cout << "Health : " << entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value() << "\n";

	   }




	   // Call down to base class to actually handle the incoming detonation and compute updated
	   // overall, mobility, and firepower health values.
	   //return DtVreHumanDamageActuator::adjudicateDirectDamage(detonationContext);
	   return DtVreHumanDamageActuator::adjudicateDirectDamage(detonationContext);
	}
	*/





	bool DtExampleHumanDamageActuator::getClosestDamageZone(const DtDetonationContext& detonationContext,
		DtDamageZoneInfo& damageZone)
	{
		DtHumanStateRepository* myHumanStateRepository = dynamic_cast<DtHumanStateRepository*>(entity()->internalState());

		// Location of hit in body coordinates of the entity.
		DtVector detonationEntityLocation = detonationContext.detonationInter().entityLocation();

		// Name of the surface on the entity's bounding volume that was hit.
		// One of the following:  "front", "left-side", "right-side", "top", "bottom", "rear".
		std::string surfaceName = std::string(detonationContext.surface().c_str());

		// Get the posture
		DtLifeformState posture = myHumanStateRepository->assignedPosture();

		// Look up the damage map for this posture
		DtDamageMap damageMap = myPostureDamageMap[posture];

		// The distance to current closest damage zone to the detonation.
		// The value is distance squared.
		// Initialize to max to indicate it has not been assigned yet.
		double currentClosestDistanceSquared = std::numeric_limits<double>::max();

		DtDamageZoneInfo* currentClosestDamageZoneInfo = nullptr;

		// Iterate through the different damage zones to find which zone was hit.
		for (auto& damageMapIter : damageMap)
		{
			DtDamageZoneInfo damageZoneInfo = damageMapIter.second;

			// determine the vector from the detonation location to the body zone center.
			DtVector distanceToZoneCenter = (damageZoneInfo.bodyZoneCenter - detonationEntityLocation);

			// For distance calculations, use the magnitude squared of the vector.
			double distZoneCenterSquared = distanceToZoneCenter.magnitudeSquared();

			if (distZoneCenterSquared <= currentClosestDistanceSquared)
			{
				// If this zone is independent of surface data, or it does use surface data and the surface is a match... 
				if (!damageZoneInfo.useSurfaceData || (damageZoneInfo.useSurfaceData && surfaceName == damageZoneInfo.surfaceName))
				{
					// Set this damage zone data to be the new closest one
					currentClosestDistanceSquared = distZoneCenterSquared;
					currentClosestDamageZoneInfo = &(damageMapIter.second);
				}
			}
		}

		if (currentClosestDamageZoneInfo)
		{
			damageZone = *currentClosestDamageZoneInfo;

			return true;
		}

		return false;
	}

	DtSimComponent* DtExampleHumanDamageActuator::creator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc,
		DtReaderWriterRegistry* parentRegistry)
	{
		//return MarkThreadSafe(new DtExampleHumanDamageActuator(name, owner, simManager, desc, parentRegistry));
		return new DtExampleHumanDamageActuator(name, owner, simManager, desc, parentRegistry);
	}

}
