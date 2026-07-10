#pragma once
#include <windows.h>
#include <string>

class Dumper {
public:
    static Dumper& GetInstance();

    void Init();
    void DumpSurface(const std::string& name, void* pBits, int width, int height, int pitch, int bpp);

private:
    Dumper();
    Dumper(const Dumper&) = delete;
    Dumper& operator=(const Dumper&) = delete;

    std::string m_dumpFolder;
    bool m_enabled;
    int m_dumpCounter;
};

#define DUMP_SURFACE(name, bits, w, h, p, bpp) Dumper::GetInstance().DumpSurface(name, bits, w, h, p, bpp)