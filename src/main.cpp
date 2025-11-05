#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <toml++/toml.hpp>
#define SPDLOG_EOL ""
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "commands.hpp"
#include "consts.hpp"
#include "util.hpp"

void run_toml_before_cmd ();
void run_toml_after_cmd ();

int main ( int argc, char **argv ) {
  spdlog::set_pattern("%v");

  if ( clarbe_env == "null" ) {
    spdlog::info("Environment variable 'CLARBE_HOME' not defined.\n");
    return 1;
  }

  if ( argc < 2 ) {
    spdlog::info("No arguments provided, try 'help'.\n");
    return 1;
  }

  auto args = char_arr_to_vector(argv, 1);

  if ( find_position_in_vec<std::string>(args, "--trace" ) != args.size() ) {
    spdlog::set_level(spdlog::level::trace);
  } else if ( find_position_in_vec<std::string>(args, "--verbose" ) != args.size() ) {
    spdlog::set_level(spdlog::level::debug);
  } else if ( find_position_in_vec<std::string>(args, "--silent" ) != args.size() ) {
    spdlog::set_level(spdlog::level::off);
  }

  // Check if it's a basic command
  if ( commands.contains ( args[ 0 ] ) ) {
    run_toml_before_cmd ();
    int ret = commands[ args[ 0 ] ]( args );
    run_toml_after_cmd ();
    return ret;
  }

  const std::string lib_path = clarbe_env + "/bin/" + args[ 0 ] + ".zip";
  auto addon = AddonFile(lib_path);

  run_toml_before_cmd ();
  int ret = addon.execute<int, char**>("proc", argv);
  run_toml_after_cmd ();

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
