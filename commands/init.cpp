#include "init.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "cmd_template.hpp"

namespace fs = std::filesystem;

MAIN_FUNC(const args_t& args) {
  if (!fs::is_empty(fs::current_path())) {
    std::cout << "Current directory is not empty.\n";
    return 0;
  }

  if (args[1] == "plugin") {
    create_plugin_template(fs::current_path().generic_string());
    return 0;
  }

  add_content_to_folder(fs::current_path().generic_string());

  return 0;
}
