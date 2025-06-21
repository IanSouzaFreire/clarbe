#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

#include "cmd_template.hpp"
#include "config_param.h"
#include "consts.hpp"
#include "dll_handling.hpp"

namespace fs = std::filesystem;

#define CMD_FUNC []( const args_t &args ) -> int

typedef char *( *fhelp_func ) ();

std::map< std::string, std::function< int ( const args_t & ) > > commands = {
  { "help", CMD_FUNC{ std::cout << "Available commands:\n";
for ( const auto &[ name, func ] : commands ) {
  std::cout << "  " << name << "\n";
}

std::string bin_path = clarbe_env + "/bin";
for ( const auto &entry : fs::directory_iterator ( bin_path ) ) {
  if ( entry.is_regular_file () && entry.path ().extension () == DLL_SUFFIX ) {
    std::string lib_path = entry.path ().string ();

    fhelp_func fhelp = nullptr;

    open_dll ( fhelp_func, lib_path.c_str (), fhelp, fs::path ( entry ).stem ().string (), continue );

    char *help_message = fhelp ();
    if ( help_message ) {
      std::cout << "  " << help_message << "\n";
    } else {
      std::cerr << "fhelp returned nullptr for " << lib_path << "\n";
    }

    close_dll;
  }
}
return 0;
}
}
, { "version", CMD_FUNC{ std::cout << CLARBE_LOCAL_SOFTWARE_VERSION << '\n';
return 0;
}
}
, {
  "clean", CMD_FUNC {
    fs::remove_all ( args[ 1 ] );
    return 0;
  }
}
}
;

#endif