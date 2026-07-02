/******************************************************************************
** Copyright (c) 2019 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file vreAccessoryTemplate.h
//! \brief Contains DLL export macros (port of exampleAccessoryTemplate.h)

#ifndef VREACCESSORYTEMPLATE_H_
#define VREACCESSORYTEMPLATE_H_

#ifdef _WIN32
#ifdef VREACCESSORYTEMPLATE_EXPORTS
#define DT_DLL_VREACCESSORYTEMPLATE __declspec(dllexport)
#else
#define DT_DLL_VREACCESSORYTEMPLATE __declspec(dllimport)
#endif
#else
#define DT_DLL_VREACCESSORYTEMPLATE
#endif

#endif
