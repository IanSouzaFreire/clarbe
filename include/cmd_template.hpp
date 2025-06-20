#ifndef CMD_TEMPLATE_HPP
#define CMD_TEMPLATE_HPP

#include <string>
#include <vector>

#if defined( __linux__ )
#define EXPORT_FN extern "C"
#elif defined( _WIN32 )
#define EXPORT_FN extern "C" __declspec ( dllexport )
#endif

#define HELP_FUNC EXPORT_FN char *fhelp // add support for custom help messages later

#define MAIN_FUNC EXPORT_FN int proc

typedef std::vector< std::string > args_t;

#endif