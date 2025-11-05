#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <boost/dll.hpp>
#include <zip.h>
#include <fstream>
#include <filesystem>
#include <memory>
#include <cstdlib>

#ifdef __linux__
const std::string dll_ext = "so";
#elif _WIN32
const std::string dll_ext = "dll";
#endif

class AddonFile {
private:
    boost::dll::shared_library lib;
    zip_t* archive = nullptr;
    std::filesystem::path temp_dir;
    std::filesystem::path temp_lib;
    std::string lib_name;

public:
    AddonFile(const std::string& path) {
        spdlog::debug("opening addon path \"{}\"\n", path);

        // Determine platform-specific library name
        #ifdef __linux__
            lib_name = "linux.so";
        #elif _WIN32
            lib_name = "windows.dll";
        #else
            #error "Unsupported platform"
        #endif

        temp_dir = std::filesystem::temp_directory_path();
        temp_lib = temp_dir / lib_name;

        int err = 0;
        archive = zip_open(path.c_str(), ZIP_RDONLY, &err);
        
        if (!archive) {
            zip_error_t error;
            zip_error_init_with_code(&error, err);
            std::string err_str = zip_error_strerror(&error);
            zip_error_fini(&error);
            throw std::runtime_error("Failed to load addon: " + err_str);
        }

        // Get library file info from archive
        zip_stat_t stat;
        zip_stat_init(&stat);
        
        if (zip_stat(archive, lib_name.c_str(), 0, &stat) != 0) {
            zip_close(archive);
            archive = nullptr;
            throw std::runtime_error("Library '" + lib_name + "' not found in archive");
        }
    
        // Open file from archive
        zip_file_t* file = zip_fopen(archive, lib_name.c_str(), 0);
        if (!file) {
            zip_close(archive);
            archive = nullptr;
            throw std::runtime_error("Failed to open '" + lib_name + "' from archive");
        }

        // Read library content
        std::vector<char> buffer(stat.size);
        zip_int64_t bytes_read = zip_fread(file, buffer.data(), stat.size);
        zip_fclose(file);

        if (bytes_read != static_cast<zip_int64_t>(stat.size)) {
            zip_close(archive);
            archive = nullptr;
            throw std::runtime_error("Failed to read complete library file");
        }

        // Write to temporary file
        std::ofstream out(temp_lib, std::ios::binary);
        if (!out) {
            zip_close(archive);
            archive = nullptr;
            throw std::runtime_error("Failed to create temporary file: " + temp_lib.string());
        }

        out.write(buffer.data(), buffer.size());
        out.close();

        // Set executable permissions on Linux
        #ifdef __linux__
            std::filesystem::permissions(temp_lib, 
                std::filesystem::perms::owner_read | std::filesystem::perms::owner_write | std::filesystem::perms::owner_exec |
                std::filesystem::perms::group_read | std::filesystem::perms::group_exec |
                std::filesystem::perms::others_read | std::filesystem::perms::others_exec);
        #endif
        
        try {
            lib.load(temp_lib.string(), boost::dll::load_mode::default_mode);
            spdlog::debug("Successfully loaded library from: {}\n", temp_lib.string());
        } catch (const std::exception& e) {
            if (archive) {
                zip_close(archive);
                archive = nullptr;
            }
            
            if (std::filesystem::exists(temp_lib)) {
                std::filesystem::remove(temp_lib);
            }

            throw std::runtime_error("Failed to load library: " + std::string(e.what()));
        }
    }

    ~AddonFile() {
        // Unload library
        if (lib.is_loaded()) {
            lib.unload();
        }
        
        // Close archive
        if (archive) {
            zip_close(archive);
            archive = nullptr;
        }
        
        // Remove temporary file
        if (std::filesystem::exists(temp_lib)) {
            spdlog::debug("Removing temporary library file: {}\n", temp_lib.string());
            std::filesystem::remove(temp_lib);
        }
    }

    // Delete copy constructor and assignment operator
    AddonFile(const AddonFile&) = delete;
    AddonFile& operator=(const AddonFile&) = delete;

    // Move constructor and assignment
    AddonFile(AddonFile&& other) noexcept 
        : lib(std::move(other.lib))
        , archive(other.archive)
        , temp_dir(std::move(other.temp_dir))
        , temp_lib(std::move(other.temp_lib))
        , lib_name(std::move(other.lib_name)) {
        other.archive = nullptr;
    }

    AddonFile& operator=(AddonFile&& other) noexcept {
        if (this != &other) {
            // Clean up current resources
            if (lib.is_loaded()) lib.unload();
            if (archive) zip_close(archive);
            if (std::filesystem::exists(temp_lib)) std::filesystem::remove(temp_lib);

            // Move resources
            lib = std::move(other.lib);
            archive = other.archive;
            temp_dir = std::move(other.temp_dir);
            temp_lib = std::move(other.temp_lib);
            lib_name = std::move(other.lib_name);

            other.archive = nullptr;
        }
        
        return *this;
    }

    template <typename ReturnType, typename... Args>
    ReturnType execute(const std::string& func, Args... args) {
        if (!lib.has(func)) {
            throw std::runtime_error("Function '" + func + "' not found in library");
        }

        auto func_ptr = lib.get<ReturnType(Args...)>(func);
        return func_ptr(std::forward<Args>(args)...);
    }

    bool hasFunction(const std::string& func) const {
        return lib.has(func);
    }
};
