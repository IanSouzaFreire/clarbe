#ifndef DLL_HANDLING_HPP
#define DLL_HANDLING_HPP

#include <windows.h>
#define DLL_SUFFIX ".dll"

#define open_dll( T, path, proc, cmd, in_err )                \
  HMODULE dll_file = LoadLibrary ( path );                    \
  if ( !dll_file ) {                                          \
    std::cout << "Could not find command '" << cmd << "'.\n"; \
    in_err;                                                   \
  }                                                           \
  proc = (T)GetProcAddress ( dll_file, #proc );               \
  if ( !proc ) {                                              \
    std::cout << "Could not locate function in command.\n";   \
    FreeLibrary ( dll_file );                                 \
    in_err;                                                   \
  }

#define close_dll FreeLibrary ( dll_file )

#endif