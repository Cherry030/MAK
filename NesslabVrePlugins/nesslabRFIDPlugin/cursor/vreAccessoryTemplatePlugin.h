/******************************************************************************
** Copyright (c) 2019 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file vreAccessoryTemplatePlugin.h
//! \brief Port of exampleAccessoryTemplatePlugin.h for VR-Engage

#pragma once

#include "vreAccessoryTemplate.h"

#define DT_DE_PLUGIN_EXPORT_MACRO DT_DLL_VREACCESSORYTEMPLATE
#include <vrvCore/exportPlugin.h>

namespace makVrv
{
	class DtDe;
}

namespace makVre
{
	class DtPlayerStationApp;
}

DT_DLL_VREACCESSORYTEMPLATE void init(makVrv::DtDe& de);
void loadAccessory(makVrv::DtDe* de);

extern "C"
{
	DT_DLL_VREACCESSORYTEMPLATE bool initPlayerStationModule(makVre::DtPlayerStationApp* app);
}
