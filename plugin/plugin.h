#ifndef PLUGIN_H
#define PLUGIN_H

extern "C" __declspec(dllexport) void pluginFunction();
extern "C" __declspec(dllexport) const char* getVersion();

#endif // PLUGIN_H
