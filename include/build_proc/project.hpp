#pragma once

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <mutex>
#include <toml++/toml.hpp>
#define SPDLOG_EOL ""
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "util.hpp"

namespace fs = std::filesystem;

std::mutex proj_mutex;

// keys to access arch in a map
class ArchSystKey {
  public:
  std::string arch, syst;
  ArchSystKey ( const std::string &_arch, const std::string &_syst )
  : arch ( _arch ), syst ( _syst ) {}

  ~ArchSystKey () {}

  bool equals ( const ArchSystKey &other ) const {
    return ( this->arch == other.arch ) && ( this->syst == other.syst );
  }
};

struct CompareASK {
  bool operator() ( const ArchSystKey &lk, const ArchSystKey &rk ) const {
    return lk.equals ( rk );
  }
};

std::string get_compiler ( const toml::table &local_config, const toml::table &global_config ) {
  if ( local_config[ "build" ][ "global" ][ "compiler" ] ) {
    const std::string compiler_ =
    *( local_config[ "build" ][ "global" ][ "compiler" ].value< std::string > () );
    spdlog::debug( " - located compiler \"{}\" from local.\n", compiler_.c_str () );
    return compiler_;

  } else if ( global_config[ "build" ][ "global" ][ "compiler" ] ) {
    const std::string compiler_ =
    *( global_config[ "build" ][ "global" ][ "compiler" ].value< std::string > () );
    spdlog::debug( " - located compiler \"{}\" from global.\n", compiler_.c_str () );
    return compiler_;

  } else {
    spdlog::error("No compiler provided.\n");
  }

  return "";
}

void build_main_project ( const toml::table &local_config,
                          const std::string &compiler,
                          const std::string &std,
                          const std::string &includes,
                          const std::string &used_flags,
                          const std::string &pkg_name,
                          const std::string &pre,
                          const Flags_t &flags,
                          const std::vector< std::string > &ignore ) {
  spdlog::debug("Starting build_main_project for package: {}\n", pkg_name);
  
  const auto &source_directories = *( local_config[ "build" ][ "local" ][ "sources" ].as_array () );

  fs::create_directories ( "target/objects" );
  fs::create_directories ( "target/bin" );
  spdlog::trace("Created target directories\n");

  // Add debug suffix if in debug mode
  const std::string debug_suffix = flags.debug ? "-debug" : "";

  std::map< ArchSystKey, std::string, CompareASK > main_out_objs{};

  std::stack< std::string > directories{};

  // Push all source directories onto the stack
  for ( const auto &node : source_directories ) {
    directories.push ( node.as_string ()->get () );
    spdlog::trace("Added source directory: {}\n", node.as_string ()->get ());
  }

  if ( flags.debug == true && local_config[ "profile" ][ "debug" ][ "sources" ] ) {
    const auto &debug_src_dirs = *( local_config[ "profile" ][ "debug" ][ "sources" ].as_array () );
    spdlog::debug("Adding debug profile source directories\n");

    for ( const auto &node : debug_src_dirs ) {
      directories.push ( node.as_string ()->get () );
      spdlog::trace("Added debug source directory: {}\n", node.as_string ()->get ());
    }
  } else if ( flags.release == true && local_config[ "profile" ][ "release" ][ "sources" ] ) {
    const auto &release_src_dirs = *( local_config[ "profile" ][ "release" ][ "sources" ].as_array () );
    spdlog::debug("Adding release profile source directories\n");

    for ( const auto &node : release_src_dirs ) {
      directories.push ( node.as_string ()->get () );
      spdlog::trace("Added release source directory: {}\n", node.as_string ()->get ());
    }
  }

  // push all directories inside or compile to object
  while ( !directories.empty () ) {
    const std::string dir = directories.top ();
    directories.pop ();
    spdlog::trace("Processing directory: {}\n", dir);

    for ( const auto &entry : fs::directory_iterator ( dir ) ) {
      const std::string path     = entry.path ().string ();
      const std::string filename = entry.path ().stem ().string ();

      // Check if the file extension is in the ignore list
      if ( std::find ( ignore.begin (), ignore.end (), extension_of ( path ) ) != ignore.end () ) {
        spdlog::trace("Ignoring file: {}\n", path);
        continue;
      }

      // If it's a directory, add to stack
      if ( fs::is_directory ( entry.status () ) ) {
        directories.push ( path );
        spdlog::trace("Found subdirectory: {}\n", path);
      } else {
        spdlog::debug("Compiling source file: {}\n", filename);
        
        if ( flags.use_zig ) {
          if ( flags.arch && flags.syst ) {
            const auto &arch_list = *( local_config[ "zig" ][ "build" ][ "architectures" ].as_array () );
            const auto &syst_list = *( local_config[ "zig" ][ "build" ][ "systems" ].as_array () );

            if ( !local_config[ "build" ][ "local" ][ "os_includes" ] ) {
              spdlog::error("\"build.local.os_includes\" needs to be defined to use \"zig.build.systems\"\n");
              return;
            }

            const auto &os_includes = *( local_config[ "build" ][ "local" ][ "os_includes" ].as_array () );

            for ( const auto &node_arch : arch_list ) {
              const std::string arch_str{ node_arch.as_string ()->get () };

              for ( const auto &node_syst : syst_list ) {
                const std::string syst_str{ node_syst.as_string ()->get () };
                std::string inc_w_sys = includes;

                for ( const auto &node : os_includes ) {
                  inc_w_sys += " -I " + node.as_string ()->get () + "/" + syst_str + " ";
                }

                fs::create_directories ( "target/objects/" + arch_str + "/" + syst_str );

                const std::string full_command{compiler + " -c " + path + " -o target/objects/" + arch_str + "/" + syst_str + "/" + filename + debug_suffix + "." + path[ 0 ] + ".o " + std + inc_w_sys + used_flags};
                
                proj_mutex.lock();
                int result = std::system ( full_command.c_str () );
                spdlog::info("{} {}-{} obj : {}\n", filename, arch_str, syst_str, result);
                proj_mutex.unlock();
                
                main_out_objs[ ArchSystKey ( arch_str, syst_str ) ] +=
                " target/objects/" + arch_str + "/" + syst_str + "/" + filename + debug_suffix + "." + path[ 0 ] + ".o ";
              }
            }
          } else if ( flags.arch ) {
            const auto &arch_list = *( local_config[ "zig" ][ "build" ][ "architectures" ].as_array () );

            for ( const auto &node_arch : arch_list ) {
              const std::string arch_str{ node_arch.as_string ()->get () };

              fs::create_directories ( "target/objects/" + arch_str );

              const std::string full_command{compiler + " -c " + path + " -o target/objects/" + arch_str + "/" + filename + debug_suffix + "." + path[ 0 ] + ".o " + std + includes + used_flags};
              
              proj_mutex.lock();
              int result = std::system ( full_command.c_str () );
              spdlog::info("{} {} obj : {}\n", filename, arch_str, result);
              proj_mutex.unlock();
              
              main_out_objs[ ArchSystKey ( arch_str, "null" ) ] +=
              " target/objects/" + arch_str + "/" + filename + debug_suffix + "." + path[ 0 ] + ".o ";
            }
          } else if ( flags.syst ) {
            const auto &syst_list = *( local_config[ "zig" ][ "build" ][ "systems" ].as_array () );

            if ( !local_config[ "build" ][ "local" ][ "os_includes" ] ) {
              spdlog::error("\"build.local.os_includes\" needs to be defined to use \"zig.build.systems\"\n");
              return;
            }

            const auto &os_includes = *( local_config[ "build" ][ "local" ][ "os_includes" ].as_array () );

            for ( const auto &node_syst : syst_list ) {
              const std::string syst_str{ node_syst.as_string ()->get () };
              std::string inc_w_sys = includes;

              for ( const auto &node : os_includes ) {
                inc_w_sys += " -I " + node.as_string ()->get () + "/" + syst_str + " ";
              }

              fs::create_directories ( "target/objects/" + syst_str );

              const std::string full_command{compiler + " -c " + path + " -o target/objects/" + syst_str + "/" + filename + debug_suffix + "." + path[ 0 ] + ".o " + std + inc_w_sys + used_flags};
              
              proj_mutex.lock();
              int result = std::system ( full_command.c_str () );
              spdlog::info("{} {} obj : {}\n", filename, syst_str, result);
              proj_mutex.unlock();
              
              main_out_objs[ ArchSystKey ( "null", syst_str ) ] +=
              " target/objects/" + syst_str + "/" + filename + debug_suffix + "." + path[ 0 ] + ".o ";
            }
          }
        } else {
          const std::string full_command{compiler + " -c " + path + " -o target/objects/" + filename + debug_suffix + "." + path[ 0 ] + ".o " + std + includes + used_flags};
          
          proj_mutex.lock();
          int result = std::system ( full_command.c_str () );
          spdlog::info("{} obj : {}\n", filename, result);
          proj_mutex.unlock();
          
          main_out_objs[ ArchSystKey ( "null", "null" ) ] +=
          " target/objects/" + filename + debug_suffix + "." + path[ 0 ] + ".o ";
        }
      }
    }
  }

  std::string out_file_name{ pkg_name };

  if ( local_config[ "build" ][ "output_name" ] ) {
    out_file_name = *( local_config[ "build" ][ "output_name" ].value< std::string > () );
    spdlog::debug("Using custom output name: {}\n", out_file_name);
  }

  out_file_name += debug_suffix + " ";

  spdlog::debug("Starting compilation phase\n");
  for ( const auto &[ key, out_objects ] : main_out_objs ) {
    std::string binpath{ "target/bin/" };
    std::string targetf{ "" };

    if ( key.arch == "null" && key.syst == "null" ) {
      spdlog::trace("Building for native OS\n");
    } else if ( key.arch == "null" ) {
      spdlog::trace("Building for \"{}\"\n", key.syst);
      binpath += key.syst + "/";
      fs::create_directories ( binpath );
    } else if ( key.syst == "null" ) {
      spdlog::trace("Building for \"{}\"\n", key.arch);
      binpath += key.arch + "/";
      fs::create_directories ( binpath );
    } else {
      spdlog::trace("Building for \"{}-{}\"\n", key.arch, key.syst);
      targetf = " -target " + key.arch + "-" + key.syst;
      binpath += key.arch + "/" + key.syst + "/";
      fs::create_directories ( binpath );
    }

    const std::string full_comp_command{compiler + " " + pre + " -o " + binpath + out_file_name + out_objects + std + " " + used_flags + targetf};
    
    proj_mutex.lock();
    int result = std::system ( full_comp_command.c_str () );
    spdlog::info("{} {} bin : {}\n", out_file_name, targetf, result);
    proj_mutex.unlock();
  }
  
  spdlog::debug("Build completed for package: {}\n", pkg_name);
}