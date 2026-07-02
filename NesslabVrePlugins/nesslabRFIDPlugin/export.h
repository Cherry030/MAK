/*******************************************************************************
** Copyright (c) 2024 MAK Technologies
** All rights reserved.
*******************************************************************************/

#pragma once

#ifdef _WIN32
#ifdef nesslabRFID_EXPORTS
#define RFID_DLL __declspec ( dllexport )
#else
#define RFID_DLL __declspec ( dllimport )
#endif
#else
#define RFID_DLL
#endif
