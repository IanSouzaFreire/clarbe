#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "commands.hpp"
#include "consts.hpp"
#include "toml.hpp"

#if defined(__linux__)
#include <dlfcn.h>
#define LIB_SUFFIX ".so"
#elif defined(_WIN32)
#include <windows.h>
#define LIB_SUFFIX ".dll"
#else
#error "not supported"
#endif

typedef int (*CMD_func)(const std::vector<std::string>&);

void run_toml_before_cmd();
void run_toml_after_cmd();

int main(int argc, char** argv) {
  if (clarbe_env == "null") {
    std::cout << "Environment variable 'CLARBE_HOME' not defined.\n";
    return 1;
  }

  std::vector<std::string> args;

  if (argc == 1) {
    std::cout << "No arguments provided, try 'help'.\n";
    return 1;
  }

  // Starts at 1 to skip executable's path
  for (int i = 1; i < argc; i++) {
    args.push_back(std::string(argv[i]));
  }

  // Check if it's a basic command
  if (commands.contains(args[0])) {
    run_toml_before_cmd();
    int ret = commands[args[0]](args);
    run_toml_after_cmd();
    return ret;
  }

  // Load the command's library
  std::string lib_path = clarbe_env + "/bin/" + args[0] + LIB_SUFFIX;

#if defined(__linux__)
  void* dll_file = dlopen(lib_path.c_str(), RTLD_LAZY);
  if (!dll_file) {
    std::cout << "Could not find command '" << args[0] << "': " << dlerror() << "\n";
    return 1;
  }
  CMD_func proc = (CMD_func)dlsym(dll_file, "proc");
  if (!proc) {
    std::cout << "Could not locate main function in command: " << dlerror() << "\n";
    dlclose(dll_file);
    return 1;
  }
#elif defined(_WIN32)
  HMODULE dll_file = LoadLibrary(lib_path.c_str());
  if (!dll_file) {
    std::cout << "Could not find command '" << args[0] << "'.\n";
    return 1;
  }
  CMD_func proc = (CMD_func)GetProcAddress(dll_file, "proc");
  if (!proc) {
    std::cout << "Could not locate main function in command.\n";
    FreeLibrary(dll_file);
    return 1;
  }
#endif

  run_toml_before_cmd();
  int ret = proc(args);
  run_toml_after_cmd();

#if defined(__linux__)
  dlclose(dll_file);
#elif defined(_WIN32)
  FreeLibrary(dll_file);
#endif

  return ret;
}

void run_toml_before_cmd() {
  toml::table local_config;

  try {
    local_config = toml::parse_file("clarbe.toml");
  } catch (const toml::parse_error& err) {
    std::cout << "Error parsing config file:\n" << err.description() << '\n';
  }

  if (local_config["build"]["run_before"]) {
    const auto& commands = *(local_config["build"]["run_before"].as_array());

    for (const auto& node : commands) {
      std::system(node.as_string()->get().c_str());
    }
  }
}

void run_toml_after_cmd() {
  toml::table local_config;

  try {
    local_config = toml::parse_file("clarbe.toml");
  } catch (const toml::parse_error& err) {
    std::cout << "Error parsing config file:\n" << err.description() << '\n';
  }

  if (local_config["build"]["run_after"]) {
    const auto& commands = *(local_config["build"]["run_after"].as_array());

    for (const auto& node : commands) {
      std::system(node.as_string()->get().c_str());
    }
  }
}
