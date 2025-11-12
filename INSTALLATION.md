# Buid from source
### Needed:
 - C++ compiler ( clang or g++ will do )
 - make
 - zip

### Libraries:
 - toml++
 - curl
 - spdlog
 - fmt
 - boost
 - libzip

### Method:
Run ```make all``` to build the commands and binaries needed.

Add ```CLARBE_HOME``` to your environment variables

Add ```$(CLARBE_HOME)/bin/``` to your path

move the contents of ```target/bin/``` to ```$(CLARBE_HOME)/bin/```

<br><br>

# NSIS install package

Add ```CLARBE_HOME``` to your environment variables

Add ```$(CLARBE_HOME)/bin/``` to your path

Download the release file ```clarbe-installer-windows.zip``` and extract

Run the file ```clarbe-installer-windows.exe``` and select the desired location
