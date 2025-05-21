#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <filesystem>
#include <functional>
#include <map>
#include <iostream>

#include "cmd_template.hpp"

namespace fs = std::filesystem;

#define CMD_FUNC [](const args_t& args) -> int

std::map<std::string, std::function<int(const args_t&)>> commands = {
    {"help",
     CMD_FUNC{const char help_msg[] =
                  "Software usage: \"clarbe <action> [args]?\"\n"
                  "\n"
                  "clarbe new <project-name>\n"
                  "     | add [<creator>_<name>]\n"
                  "     | remove [<creator>_<name>]\n"
                  "     | config <placement> [<configuration>=<value>]\n"
                  "     | init\n"
                  "     | build\n"
                  "     | help\n"
                  "     | clean\n"
                  "     | check\n"
                  "     | run\n"
                  "--------------------------------";

std::cout << help_msg << '\n';

return 0;
}
}
, {"version", CMD_FUNC{const char version[] = "0.0.1";

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