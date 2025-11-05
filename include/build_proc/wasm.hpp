#pragma once

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <toml++/toml.hpp>
#define SPDLOG_EOL ""
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "build_proc/flags.hpp"
#include "util.hpp"

namespace fs = std::filesystem;

std::mutex wasm_mutex;

std::string get_wasm_compiler ( const toml::table &local_config, const toml::table &global_config, Flags_t &flags ) {
  if ( local_config[ "build" ][ "global" ][ "wasm_compiler" ] ) {
    const std::string compiler_ =
    *( local_config[ "build" ][ "global" ][ "wasm_compiler" ].value< std::string > () );
    spdlog::debug( " - located compiler \"{}\" from local.\n", compiler_.c_str () );
    return compiler_;

  } else if ( global_config[ "build" ][ "global" ][ "wasm_compiler" ] ) {
    const std::string compiler_ =
    *( global_config[ "build" ][ "global" ][ "wasm_compiler" ].value< std::string > () );
    spdlog::debug( " - located compiler \"{}\" from global.\n", compiler_.c_str () );
    return compiler_;

  } else if ( local_config[ "build" ][ "local" ][ "wasms_dir" ] ) {
    spdlog::error("Wasm compilation defined but no compiler.\n");
  } else {
    flags.build_wasm_files = false;
  }

  return "";
}

void wasm_build_task ( const std::string &wasm_compiler,
                       const std::string &path,
                       const std::string &std,
                       const std::string &includes,
                       const std::string &used_flags,
                       const std::string &filename ) {
  spdlog::debug("Starting WASM build task for: {}\n", filename);
  
  wasm_mutex.lock();
  const std::string object_cmd{ wasm_compiler + " " + path + " -o target/wasms/" + filename +
                                ".wasm.o " + std + includes + used_flags };
  spdlog::trace("Compiling WASM object: {}\n", object_cmd);
  int obj_result = std::system ( object_cmd.c_str () );
  spdlog::info("{} obj : {}\n", filename, obj_result);
  wasm_mutex.unlock();

  wasm_mutex.lock();
  const std::string bin_cmd{ wasm_compiler + " target/wasms/" + filename +
                             ".wasm.o -o target/bin/" + filename + ".wasm " + std + used_flags };
  spdlog::trace("Linking WASM binary: {}\n", bin_cmd);
  int bin_result = std::system ( bin_cmd.c_str () );
  spdlog::info("{} bin : {}\n", filename, bin_result);
  wasm_mutex.unlock();
  
  spdlog::debug("Completed WASM build task for: {}\n", filename);
}

void build_wasm ( const toml::table &local_config,
                  const std::string &wasm_compiler,
                  const std::string &std,
                  const std::string &includes,
                  const std::string &used_flags,
                  const Flags_t &flags,
                  const std::vector< std::string > &ignore ) {
  if ( !local_config[ "build" ][ "local" ][ "wasms_dir" ] ) {
    spdlog::trace("No WASM directory configured, skipping WASM build\n");
    return;
  }

  spdlog::debug("Starting WASM build process\n");
  
  fs::create_directories ( "target/wasms" );
  spdlog::trace("Created target/wasms directory\n");
  
  const auto &wasm_files_dir = *( local_config[ "build" ][ "local" ][ "wasms_dir" ].as_array () );
  std::vector< std::future< void > > tasks{};

  for ( const auto &node : wasm_files_dir ) {
    const std::string dir = node.as_string ()->get ();
    spdlog::trace("Processing WASM directory: {}\n", dir);

    for ( const auto &entry : fs::directory_iterator ( dir ) ) {
      const std::string path     = entry.path ().string ();
      const std::string filename = entry.path ().stem ().string ();

      if ( std::find ( ignore.begin (), ignore.end (), extension_of ( path ) ) != ignore.end () ) {
        spdlog::trace("Ignoring WASM file: {}\n", path);
        continue;
      }

      spdlog::debug("Launching async task for WASM file: {}\n", filename);
      if (flags.debug) {
        tasks.emplace_back (
        std::async ( std::launch::async, wasm_build_task, wasm_compiler, path, std, includes, used_flags, filename + "-debug" ) );
      } else {
        tasks.emplace_back (
        std::async ( std::launch::async, wasm_build_task, wasm_compiler, path, std, includes, used_flags, filename ) );
      }
    }
  }

  spdlog::debug("Waiting for {} WASM build tasks to complete\n", tasks.size());
  for ( auto &task : tasks ) {
    task.get ();
  }

  spdlog::debug("All WASM build tasks completed\n");
}