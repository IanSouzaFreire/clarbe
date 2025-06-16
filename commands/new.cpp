#include <filesystem>
#include <fstream>
#include <iostream>

#include "cmd_template.hpp"
#include "init.hpp"

namespace fs = std::filesystem;

HELP_FUNC() { return "new <project-name> [plugin|null]"; }

MAIN_FUNC(const args_t& args) {
  if (fs::is_directory(args[2])) {
    std::cout << "Directory " << args[2] << " already exists.\n";
    return 0;
  }

  fs::create_directory(args[2]);

  if (args[3] == "plugin") {
    create_plugin_template(args[2]);
    return 0;
  }

  add_content_to_folder(args[2]);

  return 0;
}
