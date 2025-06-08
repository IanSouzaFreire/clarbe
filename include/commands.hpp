#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <filesystem>
#include <functional>
#include <iostream>
#include <map>

#include "cmd_template.hpp"

namespace fs = std::filesystem;

#define CMD_FUNC [](const args_t& args) -> int

std::map<std::string, std::function<int(const args_t&)>> commands = {
    {"help",
     CMD_FUNC{const char help_msg[] =
                  "Software usage: \"clarbe <action> [args]?\"\n"
                  "\n"
                  "clarbe new <project-name>\n"
                  "     | init\n"
                  "     | build\n"
                  "     | help\n"
                  "     | clean\n"
                  "     | run\n"
                  "--------------------------------";

std::cout << help_msg << '\n';

return 0;
}
}
, {"version", CMD_FUNC{const char version[] = "0.0.2";

std::cout << version << '\n';
return 0;
}
}
, {
  "clean", CMD_FUNC {
    fs::remove_all(args[1]);
    return 0;
  }
}
}
;

#endif