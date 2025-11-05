#pragma once

#include <string>
#include <vector>

#if defined( __linux__ )
#define EXPORT_FN extern "C"
#elif defined( _WIN32 )
#define EXPORT_FN extern "C" __declspec ( dllexport )
#endif

#define DEFINE_HELP_MESSAGE(msg) EXPORT_FN const char *fhelp(){ return(msg); }

#define MAIN_FUNC(args) EXPORT_FN int proc ( char** args )
