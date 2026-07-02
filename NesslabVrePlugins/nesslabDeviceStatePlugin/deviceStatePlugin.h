/*******************************************************************************
** Copyright (c) 2024 MAK Technologies
** All rights reserved.
*******************************************************************************/

#pragma once

#include "export.h"

#include "vrePlayerStation/playerComponent.h"

#include "deviceStateAccessory.h"
#include <vrvCore/DtAccessoryManager.h>


namespace nesslab_frontend_plugins
{
	constexpr auto DtDeviceStatePluginType = "DtDevState";

	class DtDeviceStatePlugin : public makVre::DtPlayerComponent
	{

	public:
		DtDeviceStatePlugin();
		virtual ~DtDeviceStatePlugin();

		virtual bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config);
		virtual void tick(double dt);
		virtual void shutdown();
		virtual const char* type() const override;

		virtual bool postInitialize() override;

	protected:
		makVre::DtAttributeHandle myTreadmillTypeHandle;
		makVre::DtAttributeHandle myTreadmillStateHandle;
		makVre::DtAttributeHandle myManipulatorStateHandle;

		makVre::DtAttributeHandle myHelmetAttributeHandle;
		makVre::DtAttributeHandle myBodyAttributeHandle;
		makVre::DtAttributeHandle myHandAttributeHandle;

		virtual void OnTreadmillTypeChanged(const int& val);
		virtual void OnTreadmillStateChanged(const int& val);
		virtual void OnManipulatorStateChanged(const int& val);

		virtual void OnHelmetStateChanged(const int& state);
		virtual void OnBodytStateChanged(const int& state);
		virtual void OnHandStateChanged(const int& state);
		
		std::uint8_t myTreadmillType;
		std::uint8_t myTreadmillState;
		std::uint8_t myManipulatorState;

		std::uint8_t myHelmetPatch;
		std::uint8_t myBodyPatch;
		std::uint8_t myHandPatch;
	};
}

namespace makVrv
{
	class DtDe;
}
;