#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <toml++/toml.hpp>

#include "cmd_template.hpp"
#include "util.hpp"

namespace fs = std::filesystem;

DEFINE_HELP_MESSAGE("run [flags] -va- <argv>")

MAIN_FUNC ( args_raw ) {
  if ( !fs::is_directory ( "target/bin" ) ) {
    std::cout << "Project not built.\n";
    return 0;
  }
  
  toml::table local_config = toml::parse_file ( "clarbe.toml" );
  
  auto args           = char_arr_to_vector ( args_raw, 1 );
  std::string argv    = " ";
  const size_t va_pos = find_position_in_vec< std::string > ( args, "-va-" );

  if ( va_pos < args.size () ) {
    for ( size_t i = va_pos + 1; i < args.size (); i++ ) {
      argv += args[ i ] + " ";
    }
  }

  std::system ( (
#if defined( __linux__ )
                "./target/bin/"
#elif defined( _WIN32 )
                "target\\bin\\"
#endif
                + *( local_config[ "package" ][ "name" ].value< std::string > () ) + argv )
                .c_str () );
  return 0;
}