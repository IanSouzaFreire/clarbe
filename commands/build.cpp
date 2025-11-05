#include "build_proc.hpp"
#include "util.hpp"

DEFINE_HELP_MESSAGE("build [flags] -add- [extra]")

MAIN_FUNC ( args_raw ) {
  spdlog::set_pattern("%v");

  auto args = char_arr_to_vector ( args_raw, 1 );

  if ( find_position_in_vec<std::string>(args, "--trace" ) != args.size() ) {
    spdlog::set_level(spdlog::level::trace);
  } else if ( find_position_in_vec<std::string>(args, "--verbose" ) != args.size() ) {
    spdlog::set_level(spdlog::level::debug);
  } else if ( find_position_in_vec<std::string>(args, "--silent" ) != args.size() ) {
    spdlog::set_level(spdlog::level::off);
  }

  if ( args.size() > 1 && args.at( 1 ) == "help" ) {
    print_help ( args );
    return 0;
  }
  
  if ( !fs::exists ( "clarbe.toml" ) ) {
    spdlog::info("Project file not detected.\n");
    return 0;
  }

  toml::table local_config;
  toml::table global_config;

  spdlog::debug( "loading config files.\n" );

  if ( !load_config ( local_config, global_config ) ) {
    spdlog::error( " - error while loading. Exit\n" );
    return 1;
  }

  spdlog::trace( "get ignored extensions.\n" );
  std::vector< std::string > ignore{};
  get_ext_to_ignore ( ignore, local_config, global_config );

  spdlog::trace( "get package name.\n" );
  std::string pkg_name = *( local_config[ "package" ][ "name" ].value< std::string > () );
  Flags_t compilation_flags;

  spdlog::trace( "get flags used.\n" );
  std::string used_flags = return_flags ( local_config, compilation_flags );

  spdlog::trace( "get wasm compiler.\n" );
  std::string wasm_compiler = get_wasm_compiler ( local_config, global_config, compilation_flags );

  spdlog::trace( "get compiler.\n" );
  std::string compiler = get_compiler ( local_config, global_config );

  spdlog::trace( "get include directories.\n" );
  std::string includes = get_includes ( local_config, compilation_flags );

  spdlog::trace( "get specific flags.\n" );
  std::string pre = get_pre ( local_config, pkg_name );

  spdlog::trace( "get standard to compile with.\n" );
  std::string std = get_std ( local_config, global_config );

  spdlog::trace( "recognizing extra input.\n" );
  const size_t add_pos = find_position_in_vec< std::string > ( args, "-add-" );

  // "-add-" exists
  if ( add_pos < args.size () ) {
    for ( size_t i = add_pos + 1; i < args.size (); i++ ) {
      spdlog::debug( " - adding \"{}\" as compiler flag.\n", args[ i ].c_str () );
      used_flags += args[ i ] + " ";
    }
  }

  // run flags
  for ( size_t i = 1; i < add_pos; i++ ) {
    if ( flag_op.contains ( args[ i ] ) ) {
      spdlog::debug( " - running flag \"{}\".\n", args[ i ].c_str () );
      flag_op.at ( args[ i ] ) ( compilation_flags, i, args );
    } else {
      spdlog::debug( " - flag \"{}\" not recognized.\n", args[ i ].c_str () );
    }
  }

  spdlog::trace( "generating include file.\n" );
  generate_include_file ( local_config, global_config, includes, compilation_flags );

  if ( compilation_flags.build_main_files ) {
    spdlog::trace( "start build main.\n" );
    if ( compilation_flags.build_both_iter ) {
      compilation_flags.debug = true;
      compilation_flags.release = false;
      build_main_project ( local_config, compiler, std, includes, used_flags, pkg_name, pre, compilation_flags, ignore );
      compilation_flags.debug = false;
      compilation_flags.release = true;
      build_main_project ( local_config, compiler, std, includes, used_flags, pkg_name, pre, compilation_flags, ignore );
    } else {
      build_main_project ( local_config, compiler, std, includes, used_flags, pkg_name, pre, compilation_flags, ignore );
    }
  } else {
    spdlog::trace( "build main skipped.\n" );
  }

  if ( compilation_flags.build_dlls_files ) {
    spdlog::trace( "start build dll.\n" );
    if ( compilation_flags.build_both_iter ) {
      compilation_flags.debug = true;
      compilation_flags.release = false;
      build_dlls ( local_config, compiler, std, includes, used_flags, pre, compilation_flags, ignore );
      compilation_flags.debug = false;
      compilation_flags.release = true;
      build_dlls ( local_config, compiler, std, includes, used_flags, pre, compilation_flags, ignore );
    } else {
      build_dlls ( local_config, compiler, std, includes, used_flags, pre, compilation_flags, ignore );
    }
  } else {
    spdlog::trace( "build dll skipped.\n" );
  }

  if ( compilation_flags.build_wasm_files ) {
    spdlog::trace( "start build wasm.\n" );
    if ( compilation_flags.build_both_iter ) {
      compilation_flags.debug = true;
      compilation_flags.release = false;
      build_wasm ( local_config, wasm_compiler, std, includes, used_flags, compilation_flags, ignore );
      compilation_flags.debug = false;
      compilation_flags.release = true;
      build_wasm ( local_config, wasm_compiler, std, includes, used_flags, compilation_flags, ignore );
    } else {
      build_wasm ( local_config, wasm_compiler, std, includes, used_flags, compilation_flags, ignore );
    }
  } else {
    spdlog::trace( "build wasm skipped.\n" );
  }

  return 0;
}
