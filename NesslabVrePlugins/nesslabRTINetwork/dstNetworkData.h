/******************************************************************************
** Copyright (c) 2019 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file exampleAccessoryTemplate.h
//! \brief Contains DLL export macros

#ifndef DSTNETWORKDATA_H_
#define DSTNETWORKDATA_H_

#ifdef _WIN32
#ifdef DSTNETWORKDATA_EXPORTS
#define DT_DLL_DSTNETWORKDATA __declspec ( dllexport )
#else
#define DT_DLL_DSTNETWORKDATA __declspec ( dllimport )
#endif
#else
#define DT_DLL_DSTNETWORKDATA
#endif

#endif