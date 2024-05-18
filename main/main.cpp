#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

typedef void (*PluginFunction)();
typedef const char* (*GetVersionFunction)();

std::atomic<bool> reloadPlugin(false);

void checkUserInput() {
    std::string input;
    while (true) {
        std::cin >> input;
        if (input == "reload") {
            reloadPlugin = true;
        }
    }
}

std::string GetLastErrorAsString() {
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string();
    }

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

std::string getLatestPluginPath(const std::string& directory) {
    std::string latestPluginPath;
    std::string latestVersion;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.path().extension() == ".dll") {
            HMODULE handle = LoadLibrary(entry.path().string().c_str());
            if (handle) {
                GetVersionFunction getVersion = (GetVersionFunction)GetProcAddress(handle, "getVersion");
                if (getVersion) {
                    std::string version = getVersion();
                    if (version > latestVersion) {
                        latestVersion = version;
                        latestPluginPath = entry.path().string();
                    }
                }
                FreeLibrary(handle);
            }
        }
    }
    return latestPluginPath;
}

int main() {
    const std::string pluginDirectory = ".";
    std::string pluginPath = getLatestPluginPath(pluginDirectory);
    if (pluginPath.empty()) {
        std::cerr << "No valid plugin found.\n";
        return 1;
    }

    HMODULE handle = LoadLibrary(pluginPath.c_str());
    if (!handle) {
        std::cerr << "Cannot load library: " << GetLastErrorAsString() << '\n';
        return 1;
    }

    PluginFunction func = (PluginFunction)GetProcAddress(handle, "pluginFunction");
    if (!func) {
        std::cerr << "Cannot load symbol 'pluginFunction': " << GetLastErrorAsString() << '\n';
        FreeLibrary(handle);
        return 1;
    }

    func();

    std::thread userInputThread(checkUserInput);
    userInputThread.detach();

    std::string currentVersion;
    GetVersionFunction getVersion = (GetVersionFunction)GetProcAddress(handle, "getVersion");
    if (getVersion) {
        currentVersion = getVersion();
    }

    while (true) {
        if (reloadPlugin) {
            std::string newPluginPath = getLatestPluginPath(pluginDirectory);
            if (!newPluginPath.empty() && newPluginPath != pluginPath) {
                FreeLibrary(handle);
                std::this_thread::sleep_for(std::chrono::seconds(10));

                handle = LoadLibrary(newPluginPath.c_str());
                if (!handle) {
                    std::cerr << "Cannot load library: " << GetLastErrorAsString() << '\n';
                    return 1;
                }

                func = (PluginFunction)GetProcAddress(handle, "pluginFunction");
                if (!func) {
                    std::cerr << "Cannot load symbol 'pluginFunction': " << GetLastErrorAsString() << '\n';
                    FreeLibrary(handle);
                    return 1;
                }

                getVersion = (GetVersionFunction)GetProcAddress(handle, "getVersion");
                if (getVersion) {
                    currentVersion = getVersion();
                }

                pluginPath = newPluginPath;
                reloadPlugin = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        func();
    }

    FreeLibrary(handle);
    return 0;
}
