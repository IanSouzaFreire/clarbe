#pragma once

#include <string>
#include <toml++/toml.hpp>
#define SPDLOG_EOL ""
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void handle_os_includes ( std::string &incs, const std::string &path, const Flags_t &flags ) {
  if ( flags.syst ) {
    return;
  }

#if defined( __linux__ )
  incs += "-I " + path + "/linux ";
#elif defined( _WIN32 )
  incs += "-I " + path + "/windows ";
#elif defined( __sun )
  incs += "-I " + path + "/sun ";
#elif defined( __FreeBSD__ )
  incs += "-I " + path + "/freebsd ";
#elif defined( __NetBSD__ )
  incs += "-I " + path + "/netbsd ";
#elif defined( __OpenBSD__ )
  incs += "-I " + path + "/openbsd ";
#elif defined( __APPLE__ )
  incs += "-I " + path + "/apple ";
#elif defined( __hpux )
  incs += "-I " + path + "/hpux ";
#elif defined( __osf__ )
  incs += "-I " + path + "/osf ";
#elif defined( __sgi )
  incs += "-I " + path + "/sgi ";
#elif defined( _AIX )
  incs += "-I " + path + "/AIX ";
#else
#error Platform not supported
#endif
}

std::string get_includes ( toml::table &local_config, Flags_t &compilation_flags ) {
  std::string includes = " ";

  if ( local_config[ "build" ][ "local" ][ "includes" ] ) {
    const auto &include_directories = *( local_config[ "build" ][ "local" ][ "includes" ].as_array () );

    for ( const auto &node : include_directories ) {
      const std::string dir_str{node.as_string ()->get ()};
      includes += "-I " + dir_str + " ";
      spdlog::debug(" - including directory \"{}\".\n", dir_str.c_str());
    }
  }

  if ( local_config[ "build" ][ "local" ][ "os_includes" ] ) {
    const auto &include_directories = *( local_config[ "build" ][ "local" ][ "os_includes" ].as_array () );

    for ( const auto &node : include_directories ) {
      handle_os_includes ( includes, node.as_string ()->get (), compilation_flags );
    }
  }

  if ( local_config[ "build" ][ "global" ][ "dependencies" ] ) {
    const auto &include_directories = *( local_config[ "build" ][ "global" ][ "dependencies" ].as_array () );

    for ( const auto &node : include_directories ) {
      handle_os_includes ( includes, node.as_string ()->get (), compilation_flags );
    }
  }

  if ( compilation_flags.debug == true && local_config[ "profile" ][ "debug" ][ "includes" ] ) {
    const auto &include_directories = *( local_config[ "profile" ][ "debug" ][ "includes" ].as_array () );

    for ( const auto &node : include_directories ) {
      const std::string dir_str{node.as_string ()->get ()};
      includes += "-I " + dir_str + " ";
      spdlog::debug(" - including directory from debug \"{}\".\n", dir_str.c_str());
    }
  } else if ( compilation_flags.release == true &&
              local_config[ "profile" ][ "release" ][ "includes" ] ) {
    const auto &include_directories = *( local_config[ "profile" ][ "release" ][ "includes" ].as_array () );

    for ( const auto &node : include_directories ) {
      const std::string dir_str{node.as_string ()->get ()};
      includes += "-I " + dir_str + " ";
      spdlog::debug(" - including directory from release \"{}\".\n", dir_str.c_str());
    }
  }

  return includes;
}
