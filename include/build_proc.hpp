#ifndef BUILD_PROC_HPP
#define BUILD_PROC_HPP

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "cmd_template.hpp"
#include "consts.hpp"
#include "toml.hpp"
#include "dll_handling.hpp"

namespace fs = std::filesystem;

// default values for flags
class Flags_t {
  public:
  bool cmd_window          = true;
  bool debug               = false;
  bool release             = false;
  int optimization         = 0;
  std::string architecture = "";
};

#define FLAG_OP_L []( Flags_t & flags, const int place, const args_t &args ) -> int

std::map< std::string, std::function< int ( Flags_t &, const int, const args_t & ) > > flag_op = {
  { "--no_window", FLAG_OP_L{ flags.cmd_window = false;
return 0;
}
}
, { "--optimize", FLAG_OP_L{ flags.optimization = stoi ( args[ place + 1 ] );
return 0;
}
}
, { "--debug", FLAG_OP_L{ flags.debug = true;
return 0;
}
}
, { "--release", FLAG_OP_L{ flags.release = true;
return 0;
}
}
, {
  "--architecture", FLAG_OP_L {
    flags.architecture = args[ place + 1 ];
    return 0;
  }
}
}
;

void print_help ( const args_t &args ) {
  std::cout << "Available commands:\n";
  for ( const auto &iter : flag_op ) {
    std::cout << "\t| " << iter.first << "\n";
  }
}

bool load_config ( toml::table &local_config, toml::table &global_config ) {
  try {
    local_config  = toml::parse_file ( "clarbe.toml" );
  } catch ( const toml::parse_error &err ) {
    std::cout << "Error parsing local config file:\n" << err.description () << "\n";
    return false;
  }

  try {
    global_config = toml::parse_file ( clarbe_env + "/config.toml" );
  } catch ( const toml::parse_error &err ) {
    std::cout << "Error parsing global config file:\n" << err.description () << "\n";
    
    if ( fs::exists( clarbe_env + "/config.toml" ) ) {
      std::ofstream tmp( clarbe_env + "/config.toml", std::ios::out );
      tmp.flush();
      tmp.close();
      return true;
    }

    return false;
  }
  
  return true;
}

std::string get_compiler ( const toml::table &local_config, const toml::table &global_config ) {
  if ( local_config[ "build" ][ "global" ][ "compiler" ] ) {
    return *( local_config[ "build" ][ "global" ][ "compiler" ].value< std::string > () );

  } else if ( global_config[ "build" ][ "global" ][ "compiler" ] ) {
    return *( global_config[ "build" ][ "global" ][ "compiler" ].value< std::string > () );

  } else {
    std::cout << "No compiler provided.\n";
  }

  return "";
}

std::string get_wasm_compiler ( const toml::table &local_config, const toml::table &global_config ) {
  if ( local_config[ "build" ][ "global" ][ "wasm_compiler" ] ) {
    return *( local_config[ "build" ][ "global" ][ "wasm_compiler" ].value< std::string > () );

  } else if ( global_config[ "build" ][ "global" ][ "wasm_compiler" ] ) {
    return *( global_config[ "build" ][ "global" ][ "wasm_compiler" ].value< std::string > () );

  } else if ( local_config[ "build" ][ "local" ][ "wasms_dir" ] ) {
    std::cout << "Wasm compilation defined but no compiler.\n";
  }

  return "";
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

void build_main_project ( const toml::table &local_config,
                          const std::string &compiler,
                          const std::string &std,
                          const std::string &includes,
                          const std::string &used_flags,
                          const std::string &pkg_name,
                          const std::string &pre,
                          const Flags_t &flags ) {
  std::string main_out_objs = " ";

  const auto &source_directories = *( local_config[ "build" ][ "local" ][ "sources" ].as_array () );

  fs::create_directories ( "target/object" );
  fs::create_directories ( "target/bin" );

  // compile source to .obj files
  for ( const auto &node : source_directories ) {
    const std::string dir = node.as_string ()->get ();
    for ( const auto &entry : fs::directory_iterator ( dir ) ) {
      const std::string path     = entry.path ().string ();
      const std::string filename = entry.path ().stem ().string ();
      std::system ( ( compiler + " -c " + path + " -o target/object/" + filename + ".o " + std + includes + used_flags )
                    .c_str () );
      main_out_objs += "target/object/" + filename + ".o ";
    }
  }

  if ( flags.debug == true && local_config[ "profile" ][ "debug" ][ "sources" ] ) {
    const auto &debug_src_dirs = *( local_config[ "profile" ][ "debug" ][ "sources" ].as_array () );

    for ( const auto &node : debug_src_dirs ) {
      const std::string dir = node.as_string ()->get ();
      for ( const auto &entry : fs::directory_iterator ( dir ) ) {
        const std::string path     = entry.path ().string ();
        const std::string filename = entry.path ().stem ().string ();
        std::system ( ( compiler + " -c " + path + " -o target/object/" + filename + ".o " + std + includes + used_flags )
                      .c_str () );
        main_out_objs += "target/object/" + filename + ".o ";
      }
    }
  } else if ( flags.release == true && local_config[ "profile" ][ "release" ][ "sources" ] ) {
    const auto &release_src_dirs = *( local_config[ "profile" ][ "release" ][ "sources" ].as_array () );

    for ( const auto &node : release_src_dirs ) {
      const std::string dir = node.as_string ()->get ();
      for ( const auto &entry : fs::directory_iterator ( dir ) ) {
        const std::string path     = entry.path ().string ();
        const std::string filename = entry.path ().stem ().string ();
        std::system ( ( compiler + " -c " + path + " -o target/object/" + filename + ".o " + std + includes + used_flags )
                      .c_str () );
        main_out_objs += "target/object/" + filename + ".o ";
      }
    }
  }

  std::system ( ( compiler + " " + pre + " -o target/bin/" + pkg_name + main_out_objs + std + " " + includes + used_flags )
                .c_str () );
}

void build_dlls ( const toml::table &local_config,
                  const std::string &compiler,
                  const std::string &std,
                  const std::string &includes,
                  const std::string &used_flags,
                  const std::string &pre,
                  const Flags_t &flags ) {
  if ( local_config[ "build" ][ "local" ][ "dlls_dir" ] ) {
    fs::create_directories ( "target/dlls" );
    const auto &dll_files_dir = *( local_config[ "build" ][ "local" ][ "dlls_dir" ].as_array () );

    for ( const auto &node : dll_files_dir ) {
      const std::string dir = node.as_string ()->get ();

      for ( const auto &entry : fs::directory_iterator ( dir ) ) {
        const std::string path     = entry.path ().string ();
        const std::string filename = entry.path ().stem ().string ();
        std::system ( ( compiler + " -fPIC -c " + path + " -o target/dlls/" + filename + DLL_SUFFIX + ".o " + std + includes + used_flags )
                      .c_str () );
        std::system ( ( compiler + " -shared " + pre + " -o target/bin/" + filename +
                        DLL_SUFFIX + " target/dlls/" + filename + DLL_SUFFIX + ".o " + std + includes + used_flags )
                      .c_str () );
      }
    }
  }
}

void build_wasm ( const toml::table &local_config,
                  const std::string &wasm_compiler,
                  const std::string &std,
                  const std::string &includes,
                  const std::string &used_flags,
                  const Flags_t &flags ) {
  if ( local_config[ "build" ][ "local" ][ "wasms_dir" ] ) {
    fs::create_directories ( "target/wasm_obj" );
    const auto &wasm_files_dir = *( local_config[ "build" ][ "local" ][ "wasms_dir" ].as_array () );

    for ( const auto &node : wasm_files_dir ) {
      const std::string dir = node.as_string ()->get ();
      for ( const auto &entry : fs::directory_iterator ( dir ) ) {
        const std::string path     = entry.path ().string ();
        const std::string filename = entry.path ().stem ().string ();
        std::system ( ( wasm_compiler + " " + path + " -o target/wasm_obj/" + filename +
                        ".wasm.o " + std + includes + used_flags )
                      .c_str () );
        std::system ( ( wasm_compiler + " target/wasm_obj/" + filename + ".wasm.o -o target/bin/" +
                        filename + ".wasm " + std + includes + used_flags )
                      .c_str () );
      }
    }
  }
}

std::string return_flags ( toml::table &local_config, Flags_t &flags ) {
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
    std::cout << "optimization level \'" << flags.optimization << "\' not found, using default...\n";
  }

  // Define "DEBUG" to be used by user and global includes
  if ( flags.debug == true ) {
    ret += " -g -DDEBUG";
  } else if ( flags.release == true ) {
    ret += " -DRELEASE";
  } else {
    if ( local_config[ "build" ][ "profile" ] ) {
      const std::string profile = *( local_config[ "build" ][ "profile" ].value< std::string > () );

      if ( profile == "debug" ) {
        ret += " -g -DDEBUG " + local_config[ "profile" ][ "debug" ][ "add" ].value_or< std::string > ( " " );
        flags.debug = true;
      } else if ( profile == "release" ) {
        ret += " -DRELEASE " + local_config[ "profile" ][ "debug" ][ "add" ].value_or< std::string > ( " " );
        flags.release = true;
      }
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

void handle_os_includes ( std::string &incs, const std::string &path, const Flags_t &flags ) {
  // if architecture is given, it'll search "/OS{arch}"; if not, "/OS".
  // the software will never work with wine using this approach :(
#if defined( __linux__ )
  incs += "-I " + path + "/linux" + flags.architecture + " ";
#elif defined( _WIN32 )
  incs += "-I " + path + "/win" + flags.architecture + " ";
#elif defined( __sun )
  incs += "-I " + path + "/sun" + flags.architecture + " ";
#elif defined( __FreeBSD__ )
  incs += "-I " + path + "/freebsd" + flags.architecture + " ";
#elif defined( __NetBSD__ )
  incs += "-I " + path + "/netbsd" + flags.architecture + " ";
#elif defined( __OpenBSD__ )
  incs += "-I " + path + "/openbsd" + flags.architecture + " ";
#elif defined( __APPLE__ )
  incs += "-I " + path + "/apple" + flags.architecture + " ";
#elif defined( __hpux )
  incs += "-I " + path + "/hpux" + flags.architecture + " ";
#elif defined( __osf__ )
  incs += "-I " + path + "/osf" + flags.architecture + " ";
#elif defined( __sgi )
  incs += "-I " + path + "/sgi" + flags.architecture + " ";
#elif defined( _AIX )
  incs += "-I " + path + "/AIX" + flags.architecture + " ";
#else
#error Platform not supported
#endif
}

std::string get_includes ( toml::table &local_config, Flags_t &compilation_flags ) {
  std::string includes = " ";

  if ( local_config[ "build" ][ "local" ][ "includes" ] ) {
    const auto &include_directories = *( local_config[ "build" ][ "local" ][ "includes" ].as_array () );

    for ( const auto &node : include_directories ) {
      includes += "-I " + node.as_string ()->get () + " ";
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
      includes += "-I " + node.as_string ()->get () + " ";
    }
  } else if ( compilation_flags.release == true &&
              local_config[ "profile" ][ "release" ][ "includes" ] ) {
    const auto &include_directories = *( local_config[ "profile" ][ "release" ][ "includes" ].as_array () );

    for ( const auto &node : include_directories ) {
      includes += "-I " + node.as_string ()->get () + " ";
    }
  }

  return includes;
}

void generate_include_file ( const toml::table &local_config,
                             const toml::table &global_config,
                             std::string &includes,
                             Flags_t &flags ) {
  fs::create_directories ( "target/crb_inc" ); // clarbe includes

  includes += " -I target/crb_inc ";
  std::ofstream include_file ( "target/crb_inc/config_param.h", std::fstream::trunc );
  include_file << "#ifndef CLARBE_GEN_INCLUDE_FILE\n"
               << "#define CLARBE_GEN_INCLUDE_FILE\n"
               << "#define CLARBE_FLAG_CMD_WINDOW " << flags.cmd_window << '\n'
               << "#define CLARBE_FLAG_DEBUG " << flags.debug << '\n'
               << "#define CLARBE_FLAG_RELEASE " << flags.release << '\n'
               << "#define CLARBE_FLAG_OPTIMIZATION " << flags.optimization << '\n'
               << "#define CLARBE_LOCAL_SOFTWARE_VERSION \""
               << *( local_config[ "package" ][ "version" ].value< std::string > () ) << "\"\n"
               << "#define CLARBE_PACKAGE_NAME \""
               << *( local_config[ "package" ][ "name" ].value< std::string > () ) << "\"\n"
               << "#define CLARBE_PACKAGE_TYPE \""
               << *( local_config[ "package" ][ "build_type" ].value< std::string > () ) << "\"\n"
               << "#endif";
  include_file.flush ();
  include_file.close ();
}

#endif