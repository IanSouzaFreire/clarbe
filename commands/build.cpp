#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "cmd_template.hpp"
#include "consts.hpp"
#include "toml.hpp"

namespace fs = std::filesystem;

// default values for flags
class Flags_t {
 public:
  bool cmd_window = true;
  bool debug = false;
  int optimization = 0;
  std::string architecture = "";
};

#define FLAG_OP_L \
  [](Flags_t & flags, const int place, const args_t& args) -> int

std::map<std::string, std::function<int(Flags_t&, const int, const args_t&)>>
    flag_op = {{"--no_window", FLAG_OP_L{flags.cmd_window = false;
return 0;
}
}
, {"--optimize", FLAG_OP_L{flags.optimization = stoi(args[place + 1]);
return 0;
}
}
, {"--debug", FLAG_OP_L{flags.debug = true;
return 0;
}
}
, {
  "--architecture", FLAG_OP_L {
    flags.architecture = args[place + 1];
    return 0;
  }
}
}
;

std::string return_flags(const Flags_t&);
void handle_os_includes(std::string&, const std::string&, const Flags_t&);

MAIN_FUNC(const args_t& args) {
  if (args[1] == "help") {
    std::cout << "Availible commands:\n";

    for (std::map<std::string, std::function<int(Flags_t&, const int,
                                                 const args_t&)>>::iterator
             iter = flag_op.begin();
         iter != flag_op.end(); ++iter) {
      std::cout << "\t| " << iter->first << ";";
    }
  }

  if (!fs::exists("clarbe.toml")) {
    std::cout << "Project file not detected.\n";
    return 0;
  }

  toml::table local_config;
  toml::table global_config;

  try {
    local_config = toml::parse_file("clarbe.toml");
    global_config = toml::parse_file(clarbe_env + "/config.toml");
  } catch (const toml::parse_error& err) {
    std::cout << "Error parsing config file:\n" << err.description() << "\n";
    return 1;
  }

  std::string wasm_compiler = "";
  std::string compiler = "";
  std::string pkg_name = "";
  std::string used_flags = " ";
  std::string includes = "";
  std::string pre = "";
  std::string std = "";
  std::string main_out_objs = " ";
  Flags_t compilation_flags;

  if (local_config["package"]["name"]) {
    pkg_name = *(local_config["package"]["name"].value<std::string>());
  } else {
    std::cout << "package name not defined.\n";
    return 1;
  }

  if (local_config["build"]["compiler"]) {
    compiler = *(local_config["build"]["compiler"].value<std::string>());
  } else if (global_config["build"]["compiler"]) {
    compiler = *(global_config["build"]["compiler"].value<std::string>());
  } else {
    std::cout << "No compiler provided.\n";
    return 1;
  }

  if (local_config["build"]["wasm_compiler"]) {
    wasm_compiler =
        *(local_config["build"]["wasm_compiler"].value<std::string>());
  } else if (global_config["build"]["wasm_compiler"]) {
    wasm_compiler =
        *(global_config["build"]["wasm_compiler"].value<std::string>());
  } else if (local_config["local"]["wasms_dir"]) {
    std::cout << "Wasm compilation defined but no compiler.\n";
    return 1;
  }

  if (local_config["package"]["std"]) {
    std =
        " -std=" + *(local_config["package"]["std"].value<std::string>()) + " ";
  } else if (global_config["package"]["std"]) {
    std = " -std=" + *(global_config["package"]["std"].value<std::string>()) +
          " ";
  }

  if (local_config["build"]["libs"]) {
    const auto& lib_flags_from_toml =
        *(local_config["build"]["libs"].as_array());
    for (const auto& node : lib_flags_from_toml) {
      const std::string lib = node.as_string()->get();
      used_flags += "-l" + lib + " ";
    }
  }

  // recognize arguments from cli
  for (int i = 1; i < args.size(); i++) {
    if (flag_op.contains(args[i])) {
      flag_op.at(args[i])(compilation_flags, i, args);
    } else if (args[i] == "-add-") {
      for (int j = i + 1; j < args.size(); j++) {
        used_flags += args[j] + " ";
      }

      break;
    }
  }

  fs::create_directories("target/object");
  fs::create_directories("target/bin");

  if (local_config["uses"]["macro_file"]) {
    // use macros file
  }

  if (local_config["uses"]["build_file"]) {
    // use build file
  }

  const auto& source_directories = local_config["local"]["sources"].as_array();

  if (!source_directories) {
    std::cout << "Source directories not defined properly.\n";
  }

  if (local_config["local"]["includes"]) {
    const auto& include_directories =
        *(local_config["local"]["includes"].as_array());

    for (const auto& node : include_directories) {
      includes += "-I " + node.as_string()->get() + " ";
    }
  }

  if (local_config["local"]["os_includes"]) {
    const auto& include_directories =
        *(local_config["local"]["os_includes"].as_array());

    for (const auto& node : include_directories) {
      handle_os_includes(includes, node.as_string()->get(), compilation_flags);
    }
  }

  if (local_config["build"]["debug_info"]) {
    compilation_flags.debug =
        *(local_config["build"]["debug_info"].value<bool>());
  }

  if (local_config["package"]["build_type"]) {
    std::string build_type =
        *(local_config["package"]["build_type"].value<std::string>());

    if (build_type == "exec") {
      // build executable
    } else if (build_type == "plugin" || build_type == "dll") {
      pre = "-shared";
      pkg_name += ".dll";
    } else if (build_type == "wasm") {
      pkg_name += ".wasm";
    }
  }

  if (local_config["build"]["extra"]) {
    const std::string extra_commands =
        local_config["build"]["extra"].as_string()->get();
    used_flags += " " + extra_commands;
  }

  used_flags += return_flags(compilation_flags);

  // compile source to .obj files
  for (const auto& node : *source_directories) {
    const std::string dir = node.as_string()->get();
    for (const auto& entry : fs::directory_iterator(dir)) {
      const std::string path = entry.path().string();
      const std::string filename = entry.path().stem().string();
      std::system((compiler + " -c " + path + " -o target/object/" + filename +
                   ".o " + std + includes + used_flags)
                      .c_str());
      main_out_objs += "target/object/" + filename + ".o ";
    }
  }

  // Compile accompanying dll files
  if (local_config["local"]["dlls_dir"]) {
    fs::create_directories("target/dlls");
    const auto& dll_files_dir = *(local_config["local"]["dlls_dir"].as_array());

    for (const auto& node : dll_files_dir) {
      const std::string dir = node.as_string()->get();

      for (const auto& entry : fs::directory_iterator(dir)) {
        const std::string path = entry.path().string();
        const std::string filename = entry.path().stem().string();
        std::system((compiler + " -c " + path + " -o target/dlls/" + filename +
                     ".dll.o " + std + includes + used_flags)
                        .c_str());
        std::system((compiler + " -shared " + pre + " -o target/bin/" +
                     filename + ".dll target/dlls/" + filename + ".dll.o " +
                     std + includes + used_flags)
                        .c_str());
      }
    }
  }

  // Compile accompanying wasm files
  if (local_config["local"]["wasms_dir"]) {
    fs::create_directories("target/wasm_obj");
    const auto& dll_files_dir =
        *(local_config["local"]["wasms_dir"].as_array());

    for (const auto& node : dll_files_dir) {
      const std::string dir = node.as_string()->get();

      for (const auto& entry : fs::directory_iterator(dir)) {
        const std::string path = entry.path().string();
        const std::string filename = entry.path().stem().string();
        std::system((wasm_compiler + path + " -o target/wasm_obj/" + filename +
                     ".wasm.o " + std + includes + used_flags)
                        .c_str());
        std::system((wasm_compiler + " target/wasm_obj/" + filename +
                     ".wasm.o " + " -o target/bin/" + filename + ".wasm " +
                     std + includes + used_flags)
                        .c_str());
      }
    }
  }

  std::system((compiler + " " + pre + " -o target/bin/" + pkg_name +
               main_out_objs + std + " " + includes + used_flags)
                  .c_str());

  return 0;
}

std::string return_flags(const Flags_t& flags) {
  std::string ret = "";

  if (!flags.cmd_window) {
    ret += " -mwindows ";
  }

  if (flags.optimization == 0) {
    ret += " -O0 ";
  } else if (flags.optimization == 1) {
    ret += " -O1 ";
  } else if (flags.optimization == 2) {
    ret += " -O2 ";
  } else if (flags.optimization == 3) {
    ret += " -O3 ";
  } else {
    std::cout << "optimization level \'" << flags.optimization
              << "\' not found, using default...\n";
  }

  if (flags.debug == true) {
    ret += " -g ";
  }

  return ret;
}

void handle_os_includes(std::string& incs, const std::string& path,
                        const Flags_t& flags) {
  // if architecture is given, it'll search "/OS{arch}"; if not, "/OS".
  // the software will never work with wine using this approach
#if defined(__linux__)
  incs += "-I " + path + "/linux" + flags.architecture + " ";
#elif defined(_WIN32)
  incs += "-I " + path + "/win" + flags.architecture + " ";
#elif defined(__sun)
  incs += "-I " + path + "/sun" + flags.architecture + " ";
#elif defined(__FreeBSD__)
  incs += "-I " + path + "/freebsd" + flags.architecture + " ";
#elif defined(__NetBSD__)
  incs += "-I " + path + "/netbsd" + flags.architecture + " ";
#elif defined(__OpenBSD__)
  incs += "-I " + path + "/openbsd" + flags.architecture + " ";
#elif defined(__APPLE__)
  incs += "-I " + path + "/apple" + flags.architecture + " ";
#elif defined(__hpux)
  incs += "-I " + path + "/hpux" + flags.architecture + " ";
#elif defined(__osf__)
  incs += "-I " + path + "/osf" + flags.architecture + " ";
#elif defined(__sgi)
  incs += "-I " + path + "/sgi" + flags.architecture + " ";
#elif defined(_AIX)
  incs += "-I " + path + "/AIX" + flags.architecture + " ";
#else
#error Platform not supported
#endif
}