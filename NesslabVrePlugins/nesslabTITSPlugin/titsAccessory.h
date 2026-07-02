/******************************************************************************
** Copyright (c) 2019 MAK Technologies, Inc.
** All rights reserved.
******************************************************************************/

//! \file rfidAccessory.h
//! \brief Contains DLL export macros

#ifndef EXAMPLEACCESSORYTEMPLATE_H_
#define EXAMPLEACCESSORYTEMPLATE_H_

#ifdef _WIN32
#ifdef EXAMPLEACCESSORYTEMPLATE_EXPORTS
#define DT_DLL_EXAMPLEACCESSORYTEMPLATE __declspec ( dllexport )
#else
#define DT_DLL_EXAMPLEACCESSORYTEMPLATE __declspec ( dllimport )
#endif
#else
#define DT_DLL_EXAMPLEACCESSORYTEMPLATE
#endif

#endif
