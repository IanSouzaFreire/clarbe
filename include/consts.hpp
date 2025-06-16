#ifndef CONSTS_HPP
#define CONSTS_HPP

#include <string>

const char* getenv_or(const char* env, const char* err) noexcept {
  char* tmp;
  return (tmp = std::getenv(env)) != NULL ? tmp : err;
}

const std::string clarbe_env(getenv_or("CLARBE_HOME", "null"));
const std::string temp_env(getenv_or("TEMP", getenv_or("TMP", "null")));
#if defined(__linux__)
const std::string user_env(getenv_or("HOME", "null"));
#elif defined(_WIN32)
const std::string user_env(getenv_or("USERPROFILE", "null"));
#endif

#endif