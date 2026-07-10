#include "Dumper.h"
#include "Settings.h"
#include "Logger.h"
#include <windows.h>
#include <stdio.h>
#include <string>

Dumper& Dumper::GetInstance() {
    static Dumper instance;
    return instance;
}

Dumper::Dumper() : m_enabled(false), m_dumpCounter(0) {}

void Dumper::Init() {
    m_enabled = Settings::GetInstance().EnableDumps;
    m_dumpFolder = Settings::GetInstance().DumpFolder;

    if (m_enabled) {
        // Используем Windows API для создания папки - это надежнее, чем _mkdir
        if (CreateDirectoryA(m_dumpFolder.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
            LOG_BASIC("Dump folder ready: %s", m_dumpFolder.c_str());
        }
        else {
            LOG_BASIC("Failed to create dump folder: %s (Error: %d)", m_dumpFolder.c_str(), GetLastError());
        }
    }
    else {
        LOG_BASIC("Dumps are disabled in wrapper.ini");
    }
}

void Dumper::DumpSurface(const std::string& name, void* pBits, int width, int height, int pitch, int bpp) {
    if (!m_enabled || !pBits) return;

    char filename[512];
    // Сохраняем ВНУТРИ папки dumps
    sprintf_s(filename, "%s\\%s_%dx%d_%d.raw", m_dumpFolder.c_str(), name.c_str(), width, height, m_dumpCounter++);

    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        int bytesPerPixel = bpp / 8;
        if (bytesPerPixel == 0) bytesPerPixel = 2;

        for (int y = 0; y < height; y++) {
            BYTE* rowPtr = (BYTE*)pBits + (y * pitch);
            WriteFile(hFile, rowPtr, width * bytesPerPixel, &written, NULL);
        }
        CloseHandle(hFile);
        LOG_EXT("Dumped surface to %s", filename);
    }
    else {
        LOG_BASIC("Failed to create dump file: %s (Error: %d)", filename, GetLastError());
    }
}