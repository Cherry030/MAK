/*******************************************************************************
** Copyright (c) 2024 MAK Technologies
** All rights reserved.
*******************************************************************************/

#pragma once

#include "export.h"

#include "vrePlayerStation/playerComponent.h"

#include "rfidAccessory.h"


namespace nesslab_frontend_plugins
{
	constexpr auto DtRFIDPluginType = "DtRFID";

	class DtRFIDPlugin : public makVre::DtPlayerComponent
	{

	public:
		DtRFIDPlugin();
		virtual ~DtRFIDPlugin();

		virtual bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config);
		virtual void tick(double dt);
		virtual void shutdown();
		virtual const char* type() const override;

		virtual bool postInitialize() override;

	protected:

	};
}

namespace makVrv
{
	class DtDe;
}
;
RFID_DLL void init(makVrv::DtDe& de);
void loadAccessory(makVrv::DtDe* de);