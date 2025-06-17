# C.L.A.R.B.E.
###### Or: C Language Assistant for Running and Building Ecosystems.

### Community: <a href="https://discord.gg/Twhv4KU3qf"><img alt="Discord" src="https://img.shields.io/discord/1326716525461245962"></a>

Is a compiler / cargo-like package and project manager for the C programming language.
I were tired of reading incredible C/C++ committee proposal papers which were not accepted in the language, so I created this manager for the lovers of C to try adding things they'd love to see in the language.

## Overview

the project's compromised of two parts, the compiler and project manager.
hence the compiler is not yet functional, the project manager will be a wrapper around other compilers such as clang or gcc

## Dependencies
 - toml++
 - curl

## Build dependencies
 - C/C++ compiler(gcc, clang, etc...)
 - git

## Installation
 - Create a environment variable 'CLARBE_HOME' and add 'CLARBE_HOME/bin' to path
 - Download the zipped executable, place the contents in 'CLARBE_HOME/bin'
 > if you're compiling with ```build.sh``` please note that you need to define the included OS specific manually
 - now run in terminal ```clarbe version```

## Usage
 - use ```clarbe new <project_name>``` or go to an empty directory and use ```clarbe init```
 - in the project directory there should be an include and source folders, as well as a "clarbe.toml" file.
 - (Optional): If you want to change the compiler used, write:
 > ```toml
 > [build]
 > compiler="compiler_name"
 > ```
 - Now write what you wish for the source code and run ```clarbe build```, then ```clarbe run```
 - XoXo: Sleep deprived programmer.
