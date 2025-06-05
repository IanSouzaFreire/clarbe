#ifndef DLL_HANDLING_HPP
#define DLL_HANDLING_HPP

#include <dlfcn.h>
#define LIB_SUFFIX ".so"

#define open_dll                                                             \
  void* dll_file = dlopen(lib_path.c_str(), RTLD_LAZY);                      \
  if (!dll_file) {                                                           \
    std::cout << "Could not find command '" << args[0] << "': " << dlerror() \
              << "\n";                                                       \
    return 1;                                                                \
  }                                                                          \
  CMD_func proc = (CMD_func)dlsym(dll_file, "proc");                         \
  if (!proc) {                                                               \
    std::cout << "Could not locate main function in command: " << dlerror()  \
              << "\n";                                                       \
    dlclose(dll_file);                                                       \
    return 1;                                                                \
  }

#define close_dll dlclose(dll_file)

#endif