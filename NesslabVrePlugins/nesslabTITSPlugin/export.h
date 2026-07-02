#pragma once

#ifdef _WIN32
#ifdef nesslab_TITS_EXPORTS
#define TITS_DLL __declspec ( dllexport )
#else
#define TITS_DLL __declspec ( dllimport )
#endif
#else
#define TITS_DLL
#endif


