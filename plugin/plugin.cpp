#include "plugin.h"
#include <iostream>

extern "C" __declspec(dllexport) void pluginFunction() {
    std::cout << "Hello from the plugin! (version 3)\n";
}

extern "C" __declspec(dllexport) const char* getVersion() {
    return "3.0.0";
}