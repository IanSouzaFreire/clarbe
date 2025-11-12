CXX   = g++
FLAGS = -std=c++23 -O0 -g3 -Wall -Wextra -Wno-unused-parameter
LIBS  = -lzip -lspdlog -lfmt -ltomlplusplus -lboost_filesystem-mt -pthread
INC   = -I include

ifeq ($(OS),Windows_NT)
    DLL_EXT = dll
else
	DLL_EXT = so
endif

MAIN_SOURCES = src/main.cpp
CMD_SOURCES = $(wildcard commands/*.cpp)
CMD_OBJECTS = $(patsubst commands/%.cpp,target/dlls/%.o,$(CMD_SOURCES))
CMD_DLLS = $(patsubst commands/%.cpp,target/bin/%.$(DLL_EXT),$(CMD_SOURCES))

DLL_FILES := $(wildcard target/bin/*.$(DLL_EXT))
DLL_FILE_NAMES := $(notdir $(DLL_FILES))
DLL_BASE_NAMES := $(basename $(DLL_FILE_NAMES))
ZIP_TARGETS := $(addprefix target/bin/,$(addsuffix .zip,$(DLL_BASE_NAMES)))

all: build cmd zip

build:
	mkdir -p target
	mkdir -p target/objects
	mkdir -p target/bin
	mkdir -p target/dlls

	$(CXX) -c src/main.cpp -o target/objects/main.o $(INC) $(FLAGS) $(CFLAGS) $(CXXFLAGS)
	$(CXX) -o target/bin/clarbe target/objects/main.o $(LIBS) $(FLAGS) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS)

cmd: $(CMD_DLLS)

target/dlls/%.o: commands/%.cpp
	@mkdir -p target/dlls
	@echo Compiling $*.cpp
	$(CXX) -fPIC -c $< -o $@ $(INC) $(FLAGS) $(CFLAGS) $(CXXFLAGS)

target/bin/%.$(DLL_EXT): target/dlls/%.o
	@mkdir -p target/bin
	$(CXX) -shared -o $@ $< $(LIBS) $(FLAGS) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS)

zip: $(ZIP_TARGETS)

target/bin/%.zip: target/bin/%.$(DLL_EXT)
	mv $< target/bin/windows.$(DLL_EXT)
	cd target/bin && zip -q $*.zip windows.$(DLL_EXT)
	rm target/bin/windows.$(DLL_EXT)

clean:
	rm -rf target
