#pragma once
#include <windows.h>
#include <string>

class Settings {
public:
    static Settings& GetInstance();

    void Load(const std::string& iniPath);

    bool bEnabled;
    int iMapScrollDelay;

    // General
    bool ShowFPS;
    bool BasicLogging;
    bool ExtendedLogging;
    std::string WindowCaption;

    // Mouse
    bool LockMouse;
    int ReleaseMouseKey; 

    // Dump
    bool EnableDumps;
    std::string DumpFolder;

private:
    Settings();
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    static int StringToVK(const std::string& keyName, int defaultValue);
};