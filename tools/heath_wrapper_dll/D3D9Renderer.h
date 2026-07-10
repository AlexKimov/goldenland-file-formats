#pragma once
#include <d3d9.h>
#include <windows.h>

class D3D9Renderer {
public:
    static D3D9Renderer& GetInstance();

    bool Initialize(HWND hWnd);
    void Shutdown();

    IDirect3DDevice9* GetDevice() const;

    void PresentSurface(WORD* pShadowBuffer, DWORD width, DWORD height);

private:
    D3D9Renderer();
    ~D3D9Renderer();

    D3D9Renderer(const D3D9Renderer&) = delete;
    D3D9Renderer& operator=(const D3D9Renderer&) = delete;

    IDirect3D9* m_pD3D;
    IDirect3DDevice9* m_pDevice;
    HWND m_hWnd;

    static const DWORD BACKBUFFER_WIDTH = 800;
    static const DWORD BACKBUFFER_HEIGHT = 600;

    DWORD m_dwFrameCount;
    DWORD m_dwLastFPSTime;
    DWORD m_dwCurrentFPS;
    char m_szBaseCaption[256];
};