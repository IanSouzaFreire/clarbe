#pragma once

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <toml++/toml.hpp>
#define SPDLOG_EOL ""
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "build_proc/flags.hpp"
#include "addons.hpp"
#include "util.hpp"

namespace fs = std::filesystem;

std::mutex dll_mutex;

void dll_build_task ( const std::string &compiler,
                      const std::string &path,
                      const std::string &std,
                      const std::string &includes,
                      const std::string &used_flags,
                      const std::string &filename,
                      const std::string &pre ) {
  spdlog::debug("Starting DLL build task for: {}\n", filename);
  
  dll_mutex.lock();
  const std::string object_cmd{ compiler + " -fPIC -c " + path + " -o target/dlls/" + filename +
                                '.' + dll_ext + ".o " + std + includes + used_flags };
  spdlog::trace("Compiling DLL object: {}\n", object_cmd);
  int obj_result = std::system ( object_cmd.c_str () );
  spdlog::info("{} obj : {}\n", filename, obj_result);
  dll_mutex.unlock();

  dll_mutex.lock();
  const std::string bin_cmd{ compiler + " -shared " + pre + " -o target/bin/" + filename + '.' + dll_ext +
                             " target/dlls/" + filename + '.' + dll_ext + ".o " + std + used_flags };
  spdlog::trace("Linking DLL binary: {}\n", bin_cmd);
  int bin_result = std::system ( bin_cmd.c_str () );
  spdlog::info("{} bin : {}\n", filename, bin_result);
  dll_mutex.unlock();
  
  spdlog::debug("Completed DLL build task for: {}\n", filename);
}

void build_dlls ( const toml::table &local_config,
                  const std::string &compiler,
                  const std::string &std,
                  const std::string &includes,
                  const std::string &used_flags,
                  const std::string &pre,
                  const Flags_t &flags,
                  const std::vector< std::string > &ignore ) {
  if ( !local_config[ "build" ][ "local" ][ "dlls_dir" ] ) {
    spdlog::trace("No DLL directory configured, skipping DLL build\n");
    return;
  }

  spdlog::debug("Starting DLL build process\n");
  
  fs::create_directories ( "target/dlls" );
  spdlog::trace("Created target/dlls directory\n");
  
  const auto &dll_files_dir = *( local_config[ "build" ][ "local" ][ "dlls_dir" ].as_array () );
  std::vector< std::future< void > > tasks{};

  for ( const auto &node : dll_files_dir ) {
    const std::string dir = node.as_string ()->get ();
    spdlog::trace("Processing DLL directory: {}\n", dir);

    for ( const auto &entry : fs::directory_iterator ( dir ) ) {
      const std::string path     = entry.path ().string ();
      const std::string filename = entry.path ().stem ().string ();

      if ( std::find ( ignore.begin (), ignore.end (), extension_of ( path ) ) != ignore.end () ) {
        spdlog::trace("Ignoring DLL file: {}\n", path);
        continue;
      }

      spdlog::debug("Launching async task for DLL file: {}\n", filename);
      if (flags.debug) {
        tasks.emplace_back (
        std::async ( std::launch::async, dll_build_task, compiler, path, std, includes, used_flags, filename + "-debug", pre ) );
      } else {
        tasks.emplace_back (
        std::async ( std::launch::async, dll_build_task, compiler, path, std, includes, used_flags, filename, pre ) );
      }
    }
  }

  spdlog::debug("Waiting for {} DLL build tasks to complete\n", tasks.size());
  for ( auto &task : tasks ) {
    task.get ();
  }

  spdlog::debug("All DLL build tasks completed\n");
}