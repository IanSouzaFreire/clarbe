#include <filesystem>
#include <fstream>
#include <iostream>

#include "cmd_template.hpp"

namespace fs = std::filesystem;

void add_content_to_folder(const std::string& path);
void create_plugin_template(const std::string& path);

MAIN_FUNC(const args_t& args) {
  if (!fs::is_empty(fs::current_path())) {
    std::cout << "Current directory is not empty.\n";
    return 0;
  }

  if (args[1] == "plugin") {
    create_plugin_template(fs::current_path().generic_string());
    return 0;
  }

  add_content_to_folder(fs::current_path().generic_string());

  return 0;
}

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
  config_file << "[package]" << '\n'
              << "name = \"" << path << "\"" << '\n'
              << "std = \"c99\"" << '\n'
              << "build_type = \"executable\"" << '\n'
              << "version = \"0.0.1\"" << '\n'
              << '\n'
              << "[locals]" << '\n'
              << "sources = [\"src\"]" << '\n'
              << "includes = [\"inc\"]" << '\n'
              << '\n'
              << "[uses]" << '\n'
              << "macro_file = \"false\"" << '\n'
              << "build_file = \"false\"" << '\n'
              << '\n'
              << "[dependencies]" << '\n'
              << "stdio = \"latest\"" << '\n';
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
              << "compiler = \"clang++\"" << '\n'
              << "[local]" << '\n'
              << "sources = [\"src\"]" << '\n'
              << "includes = [\"inc\"]" << '\n'
              << '\n'
              << "[uses]" << '\n'
              << "macro_file = \"false\"" << '\n'
              << "build_file = \"false\"" << '\n'
              << '\n'
              << "[dependencies]" << '\n'
              << "cppstd = \"latest\"" << '\n';
  config_file.flush();
  config_file.close();
}