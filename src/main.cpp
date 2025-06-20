#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "commands.hpp"
#include "consts.hpp"
#include "dll_handling.hpp"
#include "toml.hpp"

typedef int ( *CMD_func ) ( const std::vector< std::string > & );

void run_toml_before_cmd ();
void run_toml_after_cmd ();

int main ( int argc, char **argv ) {
  if ( clarbe_env == "null" ) {
    std::cout << "Environment variable 'CLARBE_HOME' not defined.\n";
    return 1;
  }

  std::vector< std::string > args;

  if ( argc < 2 ) {
    std::cout << "No arguments provided, try 'help'.\n";
    return 1;
  }

  // Starts at 1 to skip executable's path
  for ( int i = 1; i < argc; i++ ) {
    args.push_back ( std::string ( argv[ i ] ) );
  }

  // Check if it's a basic command
  if ( commands.contains ( args[ 0 ] ) ) {
    run_toml_before_cmd ();
    int ret = commands[ args[ 0 ] ]( args );
    run_toml_after_cmd ();
    return ret;
  }

  // Load the command's library
  std::string lib_path = clarbe_env + "/bin/" + args[ 0 ] + DLL_SUFFIX;
  CMD_func proc        = nullptr;

  open_dll ( CMD_func, lib_path.c_str (), proc, args[ 0 ], return 1 );

  run_toml_before_cmd ();
  int ret = proc ( args );
  run_toml_after_cmd ();

  close_dll;

  return ret;
}

void run_toml_before_cmd () {
  toml::table local_config;

  try {
    local_config = toml::parse_file ( "clarbe.toml" );
  } catch ( const toml::parse_error &err ) {
    ;
  }

  if ( local_config[ "build" ][ "run_before" ] ) {
    const auto &commands = *( local_config[ "build" ][ "run_before" ].as_array () );

    for ( const auto &node : commands ) {
      std::system ( node.as_string ()->get ().c_str () );
    }
  }
}

void run_toml_after_cmd () {
  toml::table local_config;

  try {
    local_config = toml::parse_file ( "clarbe.toml" );
  } catch ( const toml::parse_error &err ) {
    ;
  }

  if ( local_config[ "build" ][ "run_after" ] ) {
    const auto &commands = *( local_config[ "build" ][ "run_after" ].as_array () );

    for ( const auto &node : commands ) {
      std::system ( node.as_string ()->get ().c_str () );
    }
  }
}
