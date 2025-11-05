# C.L.A.R.B.E.
###### Or: C Language Assistant for Running and Building Ecosystems.

### Community: <a href="https://discord.gg/Twhv4KU3qf"><img alt="Discord" src="https://img.shields.io/discord/1326716525461245962"></a>

Is a compiler wrapper / cargo-like package and project manager for the C and C++ programming language.

## Dependencies
 - toml++
 - curl

## Build dependencies
 - C/C++ compiler(gcc, clang, etc...)
 - git

## Installation
 - Create a environment variable 'CLARBE_HOME' and add 'CLARBE_HOME/bin' to path
 - Download the zipped executable and DLLs, place the contents in 'CLARBE_HOME/bin'
 > if you're compiling with ```build.sh``` please note that you need to define the included OS specific manually as "../specific/OS"
 - now run in terminal ```clarbe version```

## Usage
 - use ```clarbe new <project_name>``` or go to an empty directory and use ```clarbe init```
 - in the project directory there should be an include and source folders, as well as a "clarbe.toml" file.
 > to change the configuration manually, refer to the wiki
 - Now write what you wish for the source code and run ```clarbe build```, then ```clarbe run```

<h6><sub>XoXo: Sleep deprived programmer.</sub></h6>
