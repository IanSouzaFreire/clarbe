#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <toml++/toml.hpp>

#include "cmd_template.hpp"
#include "util.hpp"

// default values for flags
class Flags_t {
  public:
  bool cmd_window = true,
       build_both_iter = false,
       debug = false,
       release = false,
       build_wasm_files = true,
       build_main_files = true,
       build_dlls_files = true,
       arch = false,
       syst = false,
       use_zig = false;
  int optimization         = 0;
  std::string architecture = "";
  std::string system       = "";
};

#define FLAG_OP_L []( Flags_t & flags, const int place, const std::vector<std::string> &args ) -> int

std::map< std::string, std::function< int ( Flags_t &, const int, const std::vector<std::string> & ) > > flag_op = {
  { "--no-window", FLAG_OP_L {
    flags.cmd_window = false;
    return 0;
  }}, {
  "--optimize", FLAG_OP_L {
    flags.optimization = stoi ( args[ place + 1 ] );
    return 0;
  }}, {
  "--debug", FLAG_OP_L {
    flags.debug = true;
    return 0;
  }}, {
  "--release", FLAG_OP_L {
    flags.release = true;
    return 0;
  }}, {
  "--no-dll", FLAG_OP_L {
    flags.build_dlls_files = false;
    return 0;
  }}, {
  "--no-wasm", FLAG_OP_L {
    flags.build_wasm_files = false;
    return 0;
  }}, {
  "--no-main", FLAG_OP_L {
    flags.build_main_files = false;
    return 0;
  }}, {
  "--build-dr", FLAG_OP_L {
    flags.build_both_iter = true;
    return 0;
  }
}};


void print_help ( const std::vector<std::string>& args ) {
  spdlog::info("Available commands:\n");
  for ( const auto &iter : flag_op ) {
    spdlog::info("\t| {}\n", iter.first);
  }
}

void get_ext_to_ignore ( std::vector< std::string > &vec, toml::table &local_config, toml::table &global_config ) {
  if ( local_config[ "build" ][ "ignore" ] ) {
    const auto &ignore_dirs = *( local_config[ "build" ][ "ignore" ].as_array () );

    for ( const auto &node : ignore_dirs ) {
      const std::string to_ignore = node.as_string ()->get ();
      spdlog::debug( " - ignoring extension \"{}\" from local.\n", to_ignore.c_str () );
      vec.push_back ( to_ignore );
    }
  }

  if ( global_config[ "build" ][ "ignore" ] ) {
    const auto &ignore_dirs = *( global_config[ "build" ][ "ignore" ].as_array () );

    for ( const auto &node : ignore_dirs ) {
      if ( find_position_in_vec< std::string > ( vec, node.as_string ()->get () ) >= vec.size () ) {
        const std::string to_ignore = node.as_string ()->get ();
        spdlog::debug( " - ignoring extension \"{}\" from global.\n", to_ignore.c_str () );
        vec.push_back ( to_ignore );
      }
    }
  }
}

std::string get_pre ( toml::table &local_config, std::string &pkg_name ) {
  std::string pre = " ";

  if ( local_config[ "package" ][ "build_type" ] ) {
    std::string build_type = *( local_config[ "package" ][ "build_type" ].value< std::string > () );

    if ( build_type == "exec" ) {
      // build executable
    } else if ( build_type == "plugin" || build_type == "dll" ) {
      pre = "-shared";
      pkg_name += ".dll";
    } else if ( build_type == "wasm" ) {
      pkg_name += ".wasm";
    }
  }

  return pre;
}

std::string return_flags ( toml::table &local_config, Flags_t &flags ) {
  if ( local_config[ "zig" ][ "build" ][ "architectures" ] ) {
    flags.arch = true;
  } else {
    flags.arch = false;
  }

  if ( local_config[ "zig" ][ "build" ][ "systems" ] ) {
    flags.syst = true;
  } else {
    flags.syst = false;
  }

  flags.use_zig = flags.arch || flags.syst;

  std::string ret = "";

  if ( !flags.cmd_window ) {
    ret += " -mwindows";
  } else if ( local_config[ "build" ][ "features" ][ "command_line" ] ) {
    if ( *( local_config[ "build" ][ "features" ][ "command_line" ].value< bool > () ) == false ) {
      ret += " -mwindows";
    }
  }

  if ( flags.optimization == 0 ) {
    ret += " -O0";
  } else if ( flags.optimization == 1 ) {
    ret += " -O1";
  } else if ( flags.optimization == 2 ) {
    ret += " -O2";
  } else if ( flags.optimization == 3 ) {
    ret += " -O3";
  } else {
    spdlog::info("optimization level \'{}\' not found, using default...\n", flags.optimization);
  }

  // Define "DEBUG" to be used by user and global includes
  if ( flags.debug ) {
    ret += " -g -DDEBUG";
  } else if ( flags.release ) {
    ret += " -DRELEASE";
  } else {
    if ( local_config[ "build" ][ "profile" ] ) {
      const std::string profile = *( local_config[ "build" ][ "profile" ].value< std::string > () );

      if ( profile == "debug" || flags.debug ) {
        ret += " -g -DDEBUG " + local_config[ "profile" ][ "debug" ][ "add" ].value_or< std::string > ( " " );
        flags.debug = true;
      } else if ( profile == "release" || flags.release ) {
        ret += " -DRELEASE " + local_config[ "profile" ][ "debug" ][ "add" ].value_or< std::string > ( " " );
        flags.release = true;
      }
    }
  }

  if ( !flags.build_both_iter ) {
    if ( local_config[ "build" ][ "build_dr" ] ) {
      flags.build_both_iter = *( local_config[ "build" ][ "build_dr" ].value< bool > () );
    }
  }

  if ( local_config[ "build" ][ "extra" ] ) {
    ret += " " + local_config[ "build" ][ "extra" ].as_string ()->get ();
  }

  if ( local_config[ "build" ][ "global" ][ "libraries" ] ) {
    const auto &lib_flags_from_toml = *( local_config[ "build" ][ "global" ][ "libraries" ].as_array () );

    for ( const auto &node : lib_flags_from_toml ) {
      const std::string lib = node.as_string ()->get ();
      ret += " -l" + lib + " ";
    }
  }

  return ret;
}

std::string get_std ( toml::table &local_config, toml::table &global_config ) {
  std::string std = " ";

  if ( local_config[ "package" ][ "standard" ] ) {
    std = " -std=" + *( local_config[ "package" ][ "standard" ].value< std::string > () ) + " ";
  } else if ( global_config[ "package" ][ "standard" ] ) {
    std = " -std=" + *( global_config[ "package" ][ "standard" ].value< std::string > () ) + " ";
  }

  return std;
}
