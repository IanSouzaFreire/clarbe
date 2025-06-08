#ifndef INIT_HPP
#define INIT_HPP

#include <filesystem>
#include <fstream>
#include <iostream>

#include "cmd_template.hpp"

namespace fs = std::filesystem;

void add_content_to_folder(const std::string& path) {
  fs::create_directory(path + "/src");
  fs::create_directory(path + "/inc");

  std::ofstream source_file(path + "/src/main.c");
  source_file << "#include <stdio.h>" << '\n'
              << '\n'
              << "int main(int argc, char **argv) {" << '\n'
              << "  fprintf(stdout, \"Hello world!\");" << '\n'
              << "  return 0;" << '\n'
              << "}" << '\n';
  source_file.flush();
  source_file.close();

  std::ofstream config_file(path + "/clarbe.toml");
  config_file
      << "# For more information, visit "
         "https://github.com/IanSouzaFreire/clarbe/wiki/Build-configuration"
      << '\n'
      << "[package]" << '\n'
      << "name = \"" << fs::path(path).stem().string() << "\"" << '\n'
      << "std = \"c99\"" << '\n'
      << "build_type = \"exec\"" << '\n'
      << "version = \"0.0.1\"" << '\n'
      << '\n'
      << "[local]" << '\n'
      << "sources = [\"src\"]" << '\n'
      << "includes = [\"inc\"]" << '\n';
  config_file.flush();
  config_file.close();
}

void create_plugin_template(const std::string& path) {
  fs::create_directory(path + "/src");
  fs::create_directory(path + "/inc");

  std::ofstream source_file(path + "/src/main.cpp");
  source_file << "#include \"cmd_template.hpp\"" << '\n'
              << "#include <iostream>" << '\n'
              << '\n'
              << "MAIN_FUNC(const args_t& args) {" << '\n'
              << "  std::cout << \"Your plugin goes here!\\n\";" << '\n'
              << "  return 0;" << '\n'
              << "}" << '\n';
  source_file.flush();
  source_file.close();

  std::ofstream inc_file(path + "/inc/cmd_template.hpp");
  inc_file << "#ifndef CMD_TEMPLATE_HPP" << '\n'
           << "#define CMD_TEMPLATE_HPP" << '\n'
           << "#include <string>" << '\n'
           << "#include <vector>" << '\n'
           << "#define EXPORT_FN extern \"C\" __declspec(dllexport)" << '\n'
           << "#define MAIN_FUNC EXPORT_FN int proc" << '\n'
           << "typedef std::vector<std::string> args_t;" << '\n'
           << "#endif" << '\n';
  inc_file.flush();
  inc_file.close();

  fs::path name(path);

  std::ofstream config_file(path + "/clarbe.toml");
  config_file << "[package]" << '\n'
              << "name = \"" << name.stem().string() << "\"" << '\n'
              << "std = \"c++26\"" << '\n'
              << "build_type = \"plugin\"" << '\n'
              << "version = \"0.0.1\"" << '\n'
              << '\n'
              << "[build]" << '\n'
              << "compiler = \"g++\"" << '\n'
              << '\n'
              << "[local]" << '\n'
              << "sources = [\"src\"]" << '\n'
              << "includes = [\"inc\"]" << '\n';
  config_file.flush();
  config_file.close();
}

#endif