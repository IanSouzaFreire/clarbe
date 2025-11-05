#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "cmd_template.hpp"
#include "init.hpp"
#include "util.hpp"

namespace fs = std::filesystem;

DEFINE_HELP_MESSAGE("new <project-name> [flags]")

MAIN_FUNC ( args_raw ) {
  auto args = char_arr_to_vector ( args_raw, 1 );
  
  if ( args.size () < 2 ) {
    std::cout << "Please provide a name.\n";
    return 1;
  }

  if ( std::find ( args.begin (), args.end (), "--override" ) != args.end () ) {
    fs::remove_all ( args[ 1 ] );
  } else if ( fs::is_directory ( args[ 1 ] ) ) {
    std::cout << "Directory " << args[ 1 ] << " already exists.\n";
    return 1;
  }

  fs::create_directory ( args[ 1 ] );

  if ( std::find ( args.begin (), args.end (), "--plugin" ) != args.end () ) {
    create_plugin_template ( args[ 1 ] );
    return 0;
  }

  add_content_to_folder ( args[ 1 ] );

  return 0;
}
