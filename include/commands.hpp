#pragma once

#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

#include "cmd_template.hpp"
#include "consts.hpp"
#include "addons.hpp"

#if __has_include( "config_param.h" )
#include "config_param.h"
#else
#define CLARBE_LOCAL_SOFTWARE_VERSION "tmp-build"
#endif

namespace fs = std::filesystem;

#define CMD_FUNC []( const std::vector<std::string>& args ) -> int

std::map< std::string, std::function< int ( const std::vector<std::string>& ) > > commands = {
  {"help", CMD_FUNC {
    spdlog::info("Available commands:\n");
    for ( const auto &[ name, func ] : commands ) {
      spdlog::info("\t| {}\n", name);
    }

    std::string bin_path = clarbe_env + "/bin";
    spdlog::debug("binary files location: \"{}\"\n", bin_path);
    for ( const auto &entry : fs::directory_iterator ( bin_path ) ) {
      if ( entry.is_regular_file () && entry.path ().extension () == ".zip" ) {
        const auto path = entry.path ().string ();

        try {
          AddonFile addon(path);
          
          const auto ret =  std::string(addon.execute<const char*>("fhelp"));
          if (ret == "NONE") continue;

          spdlog::info("\t| {}\n", ret);
          
        } catch (const std::exception& e) {
          spdlog::error("Exception: {}", e.what());
          return 1;
        }
      }
    }
    return 0;
  }}, {
  "version", CMD_FUNC {
    spdlog::info("{}\n", CLARBE_LOCAL_SOFTWARE_VERSION);
    return 0;
  }}, {
  "clean", CMD_FUNC {
    fs::remove_all ( args[ 1 ] );
    return 0;
  }
}};
