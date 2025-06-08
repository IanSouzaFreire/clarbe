#ifndef DLL_HANDLING_HPP
#define DLL_HANDLING_HPP

#include <windows.h>
#define DLL_SUFFIX ".dll"

#define open_dll                                                  \
  HMODULE dll_file = LoadLibrary(lib_path.c_str());               \
  if (!dll_file) {                                                \
    std::cout << "Could not find command '" << args[0] << "'.\n"; \
    return 1;                                                     \
  }                                                               \
  CMD_func proc = (CMD_func)GetProcAddress(dll_file, "proc");     \
  if (!proc) {                                                    \
    std::cout << "Could not locate main function in command.\n";  \
    FreeLibrary(dll_file);                                        \
    return 1;                                                     \
  }

#define close_dll FreeLibrary(dll_file)

#endif