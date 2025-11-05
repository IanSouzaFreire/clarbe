#include <filesystem>
#include <fstream>
#include <iostream>

#include "cmd_template.hpp"
#include "init.hpp"
#include "util.hpp"

namespace fs = std::filesystem;

DEFINE_HELP_MESSAGE("init [plugin|null]")

MAIN_FUNC ( args_raw ) {
  auto args = char_arr_to_vector ( args_raw, 1 );

  if ( !fs::is_empty ( fs::current_path () ) ) {
    std::cout << "Current directory is not empty.\n";
    return 0;
  }

  if ( args.size () > 1 ) {
    if ( args[ 1 ] == "plugin" ) {
      create_plugin_template ( fs::current_path ().generic_string () );
      return 0;
    }
  }

  add_content_to_folder ( fs::current_path ().generic_string () );

  return 0;
}
