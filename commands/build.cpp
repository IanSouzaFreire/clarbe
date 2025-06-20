#include "build_proc.hpp"

HELP_FUNC () { return "build [flags]"; }

MAIN_FUNC ( const args_t &args ) {
  if ( args[ 1 ] == "help" ) {
    print_help ( args );
    return 0;
  }

  if ( !fs::exists ( "clarbe.toml" ) ) {
    std::cout << "Project file not detected.\n";
    return 0;
  }

  toml::table local_config;
  toml::table global_config;

  if ( !load_config ( local_config, global_config ) ) {
    return 1;
  }

  std::string wasm_compiler = get_wasm_compiler ( local_config, global_config );
  std::string compiler      = get_compiler ( local_config, global_config );
  std::string pkg_name      = *( local_config[ "package" ][ "name" ].value< std::string > () );
  Flags_t compilation_flags;
  std::string used_flags = return_flags ( local_config, compilation_flags );
  std::string includes   = get_includes ( local_config, compilation_flags );
  std::string pre        = get_pre ( local_config, pkg_name );
  std::string std        = get_std ( local_config, global_config );

  // recognize arguments from cli
  for ( int i = 1; i < args.size (); i++ ) {
    if ( flag_op.contains ( args[ i ] ) ) {
      flag_op.at ( args[ i ] ) ( compilation_flags, i, args );
    } else if ( args[ i ] == "-add-" ) {
      for ( int j = i + 1; j < args.size (); j++ ) {
        used_flags += args[ j ] + " ";
      }

      break;
    }
  }

  generate_include_file ( local_config, global_config, includes, compilation_flags );

  build_main_project ( local_config, compiler, std, includes, used_flags, pkg_name, pre, compilation_flags );
  build_dlls ( local_config, compiler, std, includes, used_flags, pre, compilation_flags );
  build_wasm ( local_config, wasm_compiler, std, includes, used_flags, compilation_flags );

  return 0;
}
