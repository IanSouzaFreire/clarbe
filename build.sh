CXX="g++"
FLAGS="-std=c++2b -O0 -g"
LIBS="-lstdc++exp"
INC="-I ../include -I ../third-party/tomlplusplus -I ../third-party/rapidjson/include -I ../third-party/curl/include"

mkdir -p third-party
cd third-party

git clone "https://github.com/marzer/tomlplusplus.git" && git -C tomlplusplus pull "https://github.com/marzer/tomlplusplus.git"

cd ..
mkdir -p build
cd build

mkdir -p bin

echo "Building compiler objects:"

# Compile each .cpp file into an object file
for src in ../src/*.cpp; do
  # Get the base name of the source file (without path and extension)
  base=$(basename "$src" .cpp)
  # Compile the source file into an object file
  $CXX -c "$src" -o "${base}.o" $INC $LIBS $FLAGS $CFLAGS $CXXFLAGS $LDFLAGS
done

echo "Building compiler binary:"

# Link all object files to create the final binary
$CXX -o bin/clarbe main.o $INC $LIBS $FLAGS $CFLAGS $CXXFLAGS $LDFLAGS

mkdir -p cmd

echo "Building commands:"

# Compile each command to it's own dll file
for src in ../commands/*.cpp; do
  base=$(basename "$src" .cpp)
  $CXX -c "$src" -o "cmd/${base}.o" $INC $LIBS $FLAGS $CFLAGS $CXXFLAGS $LDFLAGS
  $CXX -shared -o "bin/${base}.dll" "cmd/${base}.o" $INC $LIBS $FLAGS $CFLAGS $CXXFLAGS $LDFLAGS
done

# Send binary directory to the clarbe binaries dir
# cp -a "bin/." "$CLARBE_HOME/bin/"

echo "Ended build procedure."

cd ..
