#include "D3D9Renderer.h"
#include "Logger.h"
#include "Settings.h"

#pragma comment(lib, "d3d9.lib")

D3D9Renderer& D3D9Renderer::GetInstance() {
    static D3D9Renderer instance;
    return instance;
}

D3D9Renderer::D3D9Renderer()
    : m_pD3D(nullptr)
    , m_pDevice(nullptr)
    , m_hWnd(nullptr)
    , m_dwFrameCount(0)
    , m_dwLastFPSTime(0)
    , m_dwCurrentFPS(0)
{
}

D3D9Renderer::~D3D9Renderer() {
    Shutdown();
}

bool D3D9Renderer::Initialize(HWND hWnd) {
    LOG_BASIC("D3D9Renderer initializing...");

    m_hWnd = hWnd;
    GetWindowTextA(hWnd, m_szBaseCaption, sizeof(m_szBaseCaption));

    m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!m_pD3D) {
        LOG_BASIC("  -> Direct3DCreate9 failed");
        return false;
    }

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferWidth = BACKBUFFER_WIDTH;
    d3dpp.BackBufferHeight = BACKBUFFER_HEIGHT;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    HRESULT hr = m_pD3D->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &m_pDevice
    );

    if (FAILED(hr)) {
        LOG_BASIC("  -> CreateDevice failed: 0x%08X", hr);
        return false;
    }

    LOG_BASIC("  -> D3D9 device created successfully");
    return true;
}

void D3D9Renderer::Shutdown() {
    if (m_pDevice) {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }
    if (m_pD3D) {
        m_pD3D->Release();
        m_pD3D = nullptr;
    }
}

IDirect3DDevice9* D3D9Renderer::GetDevice() const {
    return m_pDevice;
}

void D3D9Renderer::PresentSurface(WORD* pShadowBuffer, DWORD width, DWORD height) {
    if (!m_pDevice || !pShadowBuffer) return;

    // fps counter
    if (Settings::GetInstance().ShowFPS) {
        m_dwFrameCount++;

        DWORD currentTime = GetTickCount();
        DWORD elapsedTime = currentTime - m_dwLastFPSTime;

        if (elapsedTime >= 1000) {
            m_dwCurrentFPS = (m_dwFrameCount * 1000) / elapsedTime;
            m_dwFrameCount = 0;
            m_dwLastFPSTime = currentTime;

            char newCaption[512];
            sprintf_s(newCaption, "%s | FPS: %lu", m_szBaseCaption, m_dwCurrentFPS);
            SetWindowTextA(m_hWnd, newCaption);
        }
    }

    IDirect3DSurface9* pBackBuffer = nullptr;
    HRESULT hr = m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

    if (SUCCEEDED(hr)) {
        D3DLOCKED_RECT dstLock;
        hr = pBackBuffer->LockRect(&dstLock, NULL, 0);

        if (SUCCEEDED(hr)) {
            DWORD* pDst = (DWORD*)dstLock.pBits;
            WORD* pSrc = pShadowBuffer;
            int dstPitch = dstLock.Pitch / 4;
            int srcPitch = width;

            for (DWORD y = 0; y < height; y++) {
                for (DWORD x = 0; x < width; x++) {
                    WORD pixel = pSrc[y * srcPitch + x];
                    BYTE r = (pixel >> 11) & 0x1F; r = (r << 3) | (r >> 2);
                    BYTE g = (pixel >> 5) & 0x3F; g = (g << 2) | (g >> 4);
                    BYTE b = pixel & 0x1F; b = (b << 3) | (b >> 2);
                    pDst[y * dstPitch + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
                }
            }
            pBackBuffer->UnlockRect();
        }
        pBackBuffer->Release();
    }

    m_pDevice->Present(NULL, NULL, NULL, NULL);
}