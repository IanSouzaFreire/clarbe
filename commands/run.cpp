#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "cmd_template.hpp"
#include "toml.hpp"

namespace fs = std::filesystem;

MAIN_FUNC(const args_t& args) {
  if (!fs::is_directory("target/bin")) {
    std::cout << "Project not built.\n";
    return 0;
  }

  toml::table local_config = toml::parse_file("clarbe.toml");

  std::string argv = " ";

  if (args[1] == "-va-") {
    for (int i = 2; i < args.size(); i++) {
      argv += args[i] + " ";
    }
  }

  std::system(("target\\bin\\" + *(local_config["package"]["name"].value<std::string>()) + argv).c_str());

  return 0;
}