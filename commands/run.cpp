#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "cmd_template.hpp"
#include "toml.hpp"
#include "util.hpp"

namespace fs = std::filesystem;

HELP_FUNC () { return "run [flags] -va- <argv>"; }

MAIN_FUNC ( const args_t &args ) {
  if ( !fs::is_directory ( "target/bin" ) ) {
    std::cout << "Project not built.\n";
    return 0;
  }

  toml::table local_config = toml::parse_file ( "clarbe.toml" );

  std::string argv    = " ";
  const size_t va_pos = find_position_in_vec< std::string > ( args, "-va-" );

  if ( va_pos < args.size () ) {
    for ( size_t i = va_pos; i < args.size (); i++ ) {
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