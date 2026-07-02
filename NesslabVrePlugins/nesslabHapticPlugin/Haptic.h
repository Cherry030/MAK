#pragma once

#include "vreVrfmodel/vreHumanDamageActuator.h"

/*
링커 라이브러리 추가
vreMessageManager.lib
vreMessages.lib
*/
#include "vreMessageManager/vreMessageManager.h"
#include "vreMessageManager/vreMessage.h"




/*
					햅틱 플러그인(BE)

	메모
	- VRIS에서 캐릭터가 피격당했을때 피격 부위와 피격 정보들을 수집하는 플러그인

*/


namespace nesslab_backend_plugins {


	class HapticPlugin : public makVre::DtVreHumanDamageActuator
	{
	public:

		//! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
		//! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
		HapticPlugin(const DtString& name,
			DtLocalObject* owner,
			DtSimulationServices* simManager,
			DtComponentDescriptor* desc = 0,
			DtReaderWriterRegistry* parentRegistry = 0);

		//! default constructor; not implemented
		HapticPlugin() = delete;

		//! copy constructor; not implemented
		HapticPlugin(const HapticPlugin& orig) = delete;

		//! assignment operator; not implemented
		const HapticPlugin& operator=(const HapticPlugin& orig) = delete;

		//! destructor
		virtual ~HapticPlugin();

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



	private:

		void InitPlugin();

		//VRIS -> Dsms
		void SendDataToDsms(const BYTE hitBodyZone, const BYTE attackerID[16], const BYTE damage, const int health);

		//VRIS -> Haptic
		void SendDataToHaptic(const BYTE hitBodyZone, const BYTE damageType, const BYTE	damage, const BYTE damageState);

		void OnDsmsDisconnected();
		void HandleDsmsDisconnectedThr();

		std::uint8_t FormatHitZone(const std::string hitBodyZoneName);
		std::uint8_t FormatDamageState(const int healthAfterHit);//EncodeDamageState
		std::thread netThread;



		//! Handles incoming CustomMessage from the simulation backend.
		makVre::DtVreMessageResult HandleStopPluginMessage(makVre::DtVreMessage* msg);
		bool stopRequested = false;

	};
}