/*******************************************************************************
** Copyright (c) 2020 MAK Technologies
** All rights reserved.
*******************************************************************************/

// exampleHumanArtPartActuator.h

// DtExampleHumanDamageActuator; acutator that demonstrates publishing
// human character articulated parts representing joints on a DI-Guy
// character.  It rotates the left shoulder of a given character. 
#pragma once

//#include "../../include/vrfExtensions/vrevrfmodel/vreHumanDamageActuator.h"
#include "vrfExtensions/vreVrfmodel/vreHumanDamageActuator.h"



namespace makVre
{
   const char DtExampleHumanDamageActuatorType[] = "vre-example-human-art-part-actuator";

   class DtExampleHumanDamageActuator : public makVre::DtVreHumanDamageActuator
   {

   public:

      //! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
      //! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
      DtExampleHumanDamageActuator(const DtString& name,
         DtLocalObject* owner,
         DtSimulationServices* simManager,
         DtComponentDescriptor* desc = 0,
         DtReaderWriterRegistry* parentRegistry = 0);

      //! default constructor; not implemented
      DtExampleHumanDamageActuator() = delete;

      //! copy constructor; not implemented
      DtExampleHumanDamageActuator(const DtExampleHumanDamageActuator& orig) = delete;

      //! assignment operator; not implemented
      const DtExampleHumanDamageActuator& operator=(const DtExampleHumanDamageActuator& orig) = delete;

      //! destructor
      virtual ~DtExampleHumanDamageActuator();

   public:

      //! \copydoc DtSimComponent::type()
      //! \return DtExampleHumanDamageActuatorType
      virtual const char* type() const override;



      //! Determines damage from a direct impact on the entity
      //! Overriding to determine where on the entity the direct hit occurred.
      //virtual bool adjudicateDirectDamage(const DtDetonationContext& detonationContext);

      //! Processes a detonation for this entity, and decrements the hit points
      //! on the entity if appropriate.
      //! Determines collateral damage from an impact near the entity
      virtual bool adjudicateCollateralDamage(const DtDetonationContext& detonationContext);



      //! Get the damage zone information given an incoming direct fire hit.  
      //! Fills out damageZone data structure information about the closest damage zone affected. 
      virtual bool getClosestDamageZone(const DtDetonationContext& detonationContext, DtDamageZoneInfo& damageZone);

      //! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
      //! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
      //! \return New instance of this class.
      static DtSimComponent* creator(const DtString& name,
         DtLocalObject* owner,
         DtSimulationServices* simManager,
         DtComponentDescriptor* desc = 0,
         DtReaderWriterRegistry* parentRegistry = 0);      
   };
}
