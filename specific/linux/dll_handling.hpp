#ifndef DLL_HANDLING_HPP
#define DLL_HANDLING_HPP

#include <dlfcn.h>
#define DLL_SUFFIX ".so"

#define open_dll(T, path, proc, cmd, in_err)                             \
  void* dll_file = dlopen(path, RTLD_LAZY);                              \
  if (!dll_file) {                                                       \
    std::cout << "Could not find command '" << cmd << "': " << dlerror() \
              << "\n";                                                   \
    in_err;                                                              \
  }                                                                      \
  proc = (T)dlsym(dll_file, #proc);                                      \
  if (!proc) {                                                           \
    std::cout << "Could not locate function in command: " << dlerror()   \
              << "\n";                                                   \
    dlclose(dll_file);                                                   \
    in_err;                                                              \
  }

#define close_dll dlclose(dll_file)

#endif