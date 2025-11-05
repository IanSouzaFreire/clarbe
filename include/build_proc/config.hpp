#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <toml++/toml.hpp>
#define SPDLOG_EOL ""
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "consts.hpp"

namespace fs = std::filesystem;

bool load_config ( toml::table &local_config, toml::table &global_config ) {
  try {
    local_config = toml::parse_file ( "clarbe.toml" );
  } catch ( const toml::parse_error &err ) {
    spdlog::error("Error parsing local config file:\n{}\n", err.description ());
    return false;
  }

  try {
    global_config = toml::parse_file ( clarbe_env + "/config.toml" );
  } catch ( const toml::parse_error &err ) {
    spdlog::error("Error parsing global config file:\n{}\n", err.description ());

    if ( fs::exists ( clarbe_env + "/config.toml" ) ) {
      spdlog::debug( " - created empty file in desired location.\n" );
      std::ofstream tmp ( clarbe_env + "/config.toml", std::ios::out );
      tmp.flush ();
      tmp.close ();
      return true;
    }

    return false;
  }

  return true;
}
