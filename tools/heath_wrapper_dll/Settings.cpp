#include "Settings.h"
#include <algorithm>
#include <cctype>

Settings& Settings::GetInstance() {
    static Settings instance;
    return instance;
}

Settings::Settings() {
    iMapScrollDelay = 1;
    bEnabled = true;

    ShowFPS = true;
    BasicLogging = true;
    ExtendedLogging = false;
    
    WindowCaption = "Zlatogorye (D3D9 Wrapper)";
 
    LockMouse = true;
    ReleaseMouseKey = 192; // VK_OEM_3 ( ~ )

    EnableDumps = false;
    DumpFolder = "dumps";
}

int Settings::StringToVK(const std::string& keyName, int defaultValue) {
    if (keyName.empty()) return defaultValue;

    std::string upper = keyName;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    struct KeyMapping {
        const char* name;
        int vkCode;
    };

    static const KeyMapping mappings[] = {
        {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
        {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
        {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},

        {"ESC", VK_ESCAPE}, {"ESCAPE", VK_ESCAPE},
        {"TAB", VK_TAB},
        {"CAPS", VK_CAPITAL}, {"CAPSLOCK", VK_CAPITAL},
        {"SHIFT", VK_SHIFT}, {"LSHIFT", VK_LSHIFT}, {"RSHIFT", VK_RSHIFT},
        {"CTRL", VK_CONTROL}, {"CONTROL", VK_CONTROL}, {"LCTRL", VK_LCONTROL}, {"RCTRL", VK_RCONTROL},
        {"ALT", VK_MENU}, {"LALT", VK_LMENU}, {"RALT", VK_RMENU},
        {"SPACE", VK_SPACE},
        {"ENTER", VK_RETURN}, {"RETURN", VK_RETURN},
        {"BACKSPACE", VK_BACK}, {"BACK", VK_BACK},
        {"DELETE", VK_DELETE}, {"DEL", VK_DELETE},
        {"INSERT", VK_INSERT}, {"INS", VK_INSERT},
        {"HOME", VK_HOME},
        {"END", VK_END},
        {"PAGEUP", VK_PRIOR}, {"PGUP", VK_PRIOR},
        {"PAGEDOWN", VK_NEXT}, {"PGDN", VK_NEXT},

        {"UP", VK_UP}, {"DOWN", VK_DOWN}, {"LEFT", VK_LEFT}, {"RIGHT", VK_RIGHT},

        {"0", 0x30}, {"1", 0x31}, {"2", 0x32}, {"3", 0x33}, {"4", 0x34},
        {"5", 0x35}, {"6", 0x36}, {"7", 0x37}, {"8", 0x38}, {"9", 0x39},

        {"A", 0x41}, {"B", 0x42}, {"C", 0x43}, {"D", 0x44}, {"E", 0x45},
        {"F", 0x46}, {"G", 0x47}, {"H", 0x48}, {"I", 0x49}, {"J", 0x4A},
        {"K", 0x4B}, {"L", 0x4C}, {"M", 0x4D}, {"N", 0x4E}, {"O", 0x4F},
        {"P", 0x50}, {"Q", 0x51}, {"R", 0x52}, {"S", 0x53}, {"T", 0x54},
        {"U", 0x55}, {"V", 0x56}, {"W", 0x57}, {"X", 0x58}, {"Y", 0x59},
        {"Z", 0x5A},

        {"TILDE", VK_OEM_3}, {"~", VK_OEM_3}, {"`", VK_OEM_3},
        {"MINUS", VK_OEM_MINUS}, {"-", VK_OEM_MINUS},
        {"EQUALS", VK_OEM_PLUS}, {"=", VK_OEM_PLUS},
        {"LBRACKET", VK_OEM_4}, {"[", VK_OEM_4},
        {"RBRACKET", VK_OEM_6}, {"]", VK_OEM_6},
        {"BACKSLASH", VK_OEM_5}, {"\\", VK_OEM_5},
        {"SEMICOLON", VK_OEM_1}, {";", VK_OEM_1},
        {"QUOTE", VK_OEM_7}, {"'", VK_OEM_7},
        {"COMMA", VK_OEM_COMMA}, {",", VK_OEM_COMMA},
        {"PERIOD", VK_OEM_PERIOD}, {".", VK_OEM_PERIOD},
        {"SLASH", VK_OEM_2}, {"/", VK_OEM_2},
    };

    for (const auto& mapping : mappings) {
        if (upper == mapping.name) {
            return mapping.vkCode;
        }
    }

    try {
        return std::stoi(keyName);
    }
    catch (...) {
        return defaultValue;
    }
}

void Settings::Load(const std::string& iniPath) {
    char buffer[256];

    bEnabled = GetPrivateProfileIntA("General", "Enabled", 1, iniPath.c_str()) != 0;
    iMapScrollDelay = GetPrivateProfileIntA("General", "MapScrollDelay", -1, iniPath.c_str());
    ShowFPS = GetPrivateProfileIntA("General", "ShowFPS", ShowFPS ? 1 : 0, iniPath.c_str()) != 0;
    BasicLogging = GetPrivateProfileIntA("General", "BasicLogging", BasicLogging ? 1 : 0, iniPath.c_str()) != 0;
    ExtendedLogging = GetPrivateProfileIntA("General", "ExtendedLogging", ExtendedLogging ? 1 : 0, iniPath.c_str()) != 0;

    GetPrivateProfileStringA("General", "WindowCaption", WindowCaption.c_str(), buffer, sizeof(buffer), iniPath.c_str());
    if (buffer[0] != '\0') WindowCaption = buffer;

    LockMouse = GetPrivateProfileIntA("Mouse", "LockMouse", LockMouse ? 1 : 0, iniPath.c_str()) != 0;

    char keyBuffer[64];
    ReleaseMouseKey = GetPrivateProfileIntA("Mouse", "CursorToggleKey", ReleaseMouseKey, iniPath.c_str());
    ReleaseMouseKey = StringToVK(keyBuffer, ReleaseMouseKey);

    EnableDumps = GetPrivateProfileIntA("Dump", "EnableDumps", EnableDumps ? 1 : 0, iniPath.c_str()) != 0;
    GetPrivateProfileStringA("Dump", "DumpFolder", DumpFolder.c_str(), buffer, sizeof(buffer), iniPath.c_str());
    if (buffer[0] != '\0') DumpFolder = buffer;
}