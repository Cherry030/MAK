#include "GrenadeCollisionActuator.h"
#include <vrfobjcore/simComponentTypes.h>


namespace nesslab_backend_plugins
{
	//수류탄 투척하면 생성
	GrenadeCollisionActuator::GrenadeCollisionActuator(const DtString& name, DtLocalObject* object,
		DtSimulationServices* simServices, DtComponentDescriptor* compDescriptor, DtReaderWriterRegistry* parentRegistry)
		:DtFrictionlessGravityCollisionsActuator(name, object, simServices, compDescriptor, parentRegistry)
	{
		std::cout << "[GrenadeCollisionActuator][Trace] Constructor \n";

	}

	GrenadeCollisionActuator::~GrenadeCollisionActuator()
	{
		std::cout << "[GrenadeCollisionActuator][Trace] Destructor \n";
	}

	const char* GrenadeCollisionActuator::type() const
	{
		return DtFrictionlessGravityCollisionsActuatorType;
	}


	//이 부분을 true로 변경
	bool GrenadeCollisionActuator::isFallingApplicable()
	{
		return true;
	}

	DtSimComponent* GrenadeCollisionActuator::creator(const DtString& name, DtLocalObject* object, DtSimulationServices* simServices, DtComponentDescriptor* compDescriptor, DtReaderWriterRegistry* parentRegistry)
	{
		return new GrenadeCollisionActuator(name, object, simServices, compDescriptor, parentRegistry);
	}
}