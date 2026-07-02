#pragma once

#ifdef _WIN32
#ifdef NESSLAB_FRONTEND_PLUGINS_EXPORTS
#define NESSLAB_FRONTEND_DLL __declspec ( dllexport )
#else
#define NESSLAB_FRONTEND_DLL __declspec ( dllimport )
#endif
#else
#define NESSLAB_FRONTEND_DLL
#endif


