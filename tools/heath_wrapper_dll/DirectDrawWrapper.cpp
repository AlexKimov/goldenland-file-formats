#include "DirectDrawWrapper.h"
#include "D3D9Renderer.h"
#include "WindowManager.h"
#include "Settings.h"
#include "Logger.h"
#include "Dumper.h"

// ============================================================================
// m_IDirectDraw7 Implementation
// ============================================================================

m_IDirectDraw7::m_IDirectDraw7()
    : m_refCount(1)
    , m_pPrimary(nullptr)
    , m_pBackbuffer(nullptr)
    , m_currentWidth(800)
    , m_currentHeight(600)
{
    LOG_BASIC("Created IDirectDraw7 wrapper");
}

m_IDirectDraw7::~m_IDirectDraw7() {
    if (m_pPrimary) m_pPrimary->Release();
    if (m_pBackbuffer) m_pBackbuffer->Release();
    D3D9Renderer::GetInstance().Shutdown();
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::QueryInterface(REFIID riid, LPVOID* ppvObj) {
    if (!ppvObj) return E_POINTER;

    char guidStr[64];
    sprintf_s(guidStr, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        riid.Data1, riid.Data2, riid.Data3,
        riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
        riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);

    LOG_BASIC("IDirectDraw7::QueryInterface %s", guidStr);

    if (riid == IID_IUnknown ||
        riid == IID_IDirectDraw ||
        riid == IID_IDirectDraw2 ||
        riid == IID_IDirectDraw4 ||
        riid == IID_IDirectDraw7) {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }

    // 2. GAMMA CONTROL SUPPORT (Brightness / Contrast Sliders in Settings)
    if (riid.Data1 == 0x69C11C3E) {
        LOG_BASIC("  -> DirectDraw: Intercepted IID_IDirectDrawGammaControl.");
        m_IDirectDrawGammaControl* pGamma = new m_IDirectDrawGammaControl();
        *ppvObj = pGamma;
        return S_OK;
    }

    // 3. Fallback для полностью неизвестных интерфейсов
    LOG_BASIC("  -> E_NOINTERFACE (Unsupported)");
    *ppvObj = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE m_IDirectDraw7::AddRef() {
    return ++m_refCount;
}

ULONG STDMETHODCALLTYPE m_IDirectDraw7::Release() {
    if (m_refCount == 0) return 0;
    ULONG count = --m_refCount;
    if (count == 0) {
        LOG_BASIC("IDirectDraw7 destroyed");
    }
    return count;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::Compact() {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::CreateClipper(DWORD f, LPDIRECTDRAWCLIPPER* c, IUnknown* o) {
    return DDERR_GENERIC;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::CreatePalette(DWORD f, LPPALETTEENTRY t, LPDIRECTDRAWPALETTE* p, IUnknown* o) {
    return DDERR_GENERIC;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::CreateSurface(LPDDSURFACEDESC2 d, LPDIRECTDRAWSURFACE7* s, IUnknown* o) {
    DWORD w = d->dwWidth;
    DWORD h = d->dwHeight;
    bool isPrimary = (d->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0;
    bool isBackbuffer = (d->ddsCaps.dwCaps & DDSCAPS_BACKBUFFER) != 0;

    LOG_BASIC("CreateSurface %dx%d Caps:0x%08X", w, h, d->ddsCaps.dwCaps);

    if (w == 0 && h == 0 && isPrimary) {
        w = m_currentWidth;
        h = m_currentHeight;
        LOG_BASIC("CreateSurface fixed 0x0 to %dx%d", w, h);
    }

    m_IDirectDrawSurface7* surf = new m_IDirectDrawSurface7(w, h, isPrimary, isBackbuffer);

    if (isPrimary) {
        m_pPrimary = surf;
        m_pBackbuffer = new m_IDirectDrawSurface7(w, h, false, true);
        surf->SetAttachedBackbuffer(m_pBackbuffer);
        LOG_BASIC("  -> Created Primary+Backbuffer chain %dx%d", w, h);
    }

    *s = surf;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::DuplicateSurface(LPDIRECTDRAWSURFACE7 s, LPDIRECTDRAWSURFACE7* d) {
    return DDERR_GENERIC;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::EnumDisplayModes(DWORD f, LPDDSURFACEDESC2 d, LPVOID c, LPDDENUMMODESCALLBACK2 cb) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::EnumSurfaces(DWORD f, LPDDSURFACEDESC2 d, LPVOID c, LPDDENUMSURFACESCALLBACK7 cb) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::FlipToGDISurface() {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetCaps(LPDDCAPS d, LPDDCAPS h) {
    ZeroMemory(d, sizeof(DDCAPS));
    d->dwSize = sizeof(DDCAPS);
    d->dwCaps = DDCAPS_BLT | DDCAPS_BLTCOLORFILL | DDCAPS_BLTSTRETCH;
    ZeroMemory(h, sizeof(DDCAPS));
    h->dwSize = sizeof(DDCAPS);
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetDisplayMode(LPDDSURFACEDESC2 d) {
    d->dwSize = sizeof(DDSURFACEDESC2);
    d->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    d->dwWidth = 800;
    d->dwHeight = 600;
    d->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    d->ddpfPixelFormat.dwFlags = DDPF_RGB;
    d->ddpfPixelFormat.dwRGBBitCount = 16;
    d->ddpfPixelFormat.dwRBitMask = 0xF800;
    d->ddpfPixelFormat.dwGBitMask = 0x07E0;
    d->ddpfPixelFormat.dwBBitMask = 0x001F;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetFourCCCodes(LPDWORD n, LPDWORD c) {
    *n = 0;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetGDISurface(LPDIRECTDRAWSURFACE7* s) {
    return DDERR_NOTFOUND;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetMonitorFrequency(LPDWORD f) {
    *f = 60;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetScanLine(LPDWORD s) {
    *s = 0;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetVerticalBlankStatus(LPBOOL b) {
    *b = FALSE;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::Initialize(GUID* g) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::RestoreDisplayMode() {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::SetCooperativeLevel(HWND hWnd, DWORD dwFlags) {
    LOG_BASIC("SetCooperativeLevel hWnd:%p Flags:0x%08X", hWnd, dwFlags);

    SetWindowTextA(hWnd, Settings::GetInstance().WindowCaption.c_str());

    if (!D3D9Renderer::GetInstance().Initialize(hWnd)) {
        return DDERR_GENERIC;
    }

    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::SetDisplayMode(DWORD w, DWORD h, DWORD b, DWORD r, DWORD f) {
    LOG_BASIC("SetDisplayMode %dx%d BPP:%d", w, h, b);
    m_currentWidth = w;
    m_currentHeight = h;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::WaitForVerticalBlank(DWORD f, HANDLE h) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetAvailableVidMem(LPDDSCAPS2 c, LPDWORD t, LPDWORD f) {
    *t = 256 * 1024 * 1024;
    *f = 128 * 1024 * 1024;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetSurfaceFromDC(HDC h, LPDIRECTDRAWSURFACE7* s) {
    return DDERR_NOTFOUND;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::RestoreAllSurfaces() {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::TestCooperativeLevel() {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 d, DWORD f) {
    ZeroMemory(d, sizeof(DDDEVICEIDENTIFIER2));
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::StartModeTest(LPSIZE m, DWORD n, DWORD f) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDraw7::EvaluateMode(DWORD f, DWORD* r) {
    *r = 0;
    return DD_OK;
}


// ============================================================================
// m_IDirectDrawGammaControl Implementation
// ============================================================================

m_IDirectDrawGammaControl::m_IDirectDrawGammaControl() : m_refCount(1) {
    LOG_BASIC("Created IDirectDrawGammaControl wrapper");
}

m_IDirectDrawGammaControl::~m_IDirectDrawGammaControl() {
    LOG_BASIC("Destroyed IDirectDrawGammaControl wrapper");
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawGammaControl::QueryInterface(REFIID riid, LPVOID* ppvObj) {
    if (!ppvObj) return E_POINTER;

    if (riid == IID_IUnknown || riid.Data1 == 0x69C11C3E) {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE m_IDirectDrawGammaControl::AddRef() {
    return ++m_refCount;
}

ULONG STDMETHODCALLTYPE m_IDirectDrawGammaControl::Release() {
    ULONG count = --m_refCount;
    if (count == 0) delete this;
    return count;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawGammaControl::GetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRamp) {
    LOG_BASIC("IDirectDrawGammaControl::GetGammaRamp called");
    if (lpRamp) {
        for (int i = 0; i < 256; i++) {
            lpRamp->red[i] = (WORD)(i * 257);
            lpRamp->green[i] = (WORD)(i * 257);
            lpRamp->blue[i] = (WORD)(i * 257);
        }
    }
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawGammaControl::SetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRamp) {
    LOG_BASIC("IDirectDrawGammaControl::SetGammaRamp called");
    if (lpRamp) {
        IDirect3DDevice9* pDevice = D3D9Renderer::GetInstance().GetDevice();
        if (pDevice) {
            pDevice->SetGammaRamp(0, 0, reinterpret_cast<const D3DGAMMARAMP*>(lpRamp));
            LOG_BASIC("  -> Successfully applied Gamma Ramp to D3D9 Device!");
        }
    }
    return DD_OK;
}


// ============================================================================
// m_IDirectDrawSurface7 Implementation
// ============================================================================

m_IDirectDrawSurface7::m_IDirectDrawSurface7(DWORD w, DWORD h, bool isPrimary, bool isBackbuffer)
    : m_refCount(1)
    , m_pTexture(nullptr)
    , m_width(w)
    , m_height(h)
    , m_bIsPrimary(isPrimary)
    , m_bIsBackbuffer(isBackbuffer)
    , m_pAttachedBackbuffer(nullptr)
    , m_colorKeyLow(0)
    , m_colorKeyHigh(0)
    , m_hasColorKey(false)
    , m_pShadowBuffer(nullptr)
    , m_hDC(nullptr)
    , m_hBitmap(nullptr)
    , m_bOwnsShadowBuffer(false)
{
    LOG_BASIC("Constructor Surface %dx%d (Primary:%d Backbuffer:%d) this:%p",
        w, h, isPrimary, isBackbuffer, this);

    if (m_width > 0 && m_height > 0) {
        struct {
            BITMAPINFOHEADER bmiHeader;
            DWORD dwMasks[3];
        } bmi;
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_width;
        bmi.bmiHeader.biHeight = -(LONG)m_height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 16;
        bmi.bmiHeader.biCompression = BI_BITFIELDS;
        bmi.dwMasks[0] = 0xF800;
        bmi.dwMasks[1] = 0x07E0;
        bmi.dwMasks[2] = 0x001F;

        void* pBits = nullptr;
        m_hBitmap = CreateDIBSection(NULL, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

        if (m_hBitmap && pBits) {
            m_pShadowBuffer = (WORD*)pBits;
            m_bOwnsShadowBuffer = false;
            m_hDC = CreateCompatibleDC(NULL);
            SelectObject(m_hDC, m_hBitmap);
            LOG_BASIC("  -> DIB Section SUCCESS. Shadow buffer: %p", m_pShadowBuffer);
        }
        else {
            m_pShadowBuffer = new WORD[m_width * m_height]();
            m_bOwnsShadowBuffer = true;
            LOG_BASIC("  -> DIB Section FAILED! Fallback allocated. Shadow buffer: %p", m_pShadowBuffer);
        }
    }

    LOG_BASIC("Created Surface %dx%d (Primary:%d Backbuffer:%d)", w, h, isPrimary, isBackbuffer);

    IDirect3DDevice9* pDevice = D3D9Renderer::GetInstance().GetDevice();
    if (pDevice) {
        HRESULT hr = pDevice->CreateTexture(
            w, h, 1, 0,
            D3DFMT_R5G6B5,
            D3DPOOL_SYSTEMMEM,
            &m_pTexture,
            NULL
        );
        if (FAILED(hr)) {
            LOG_BASIC("  -> CreateTexture failed: 0x%08X", hr);
        }
        else {
            LOG_BASIC("  -> Texture created");
        }
    }
}

m_IDirectDrawSurface7::~m_IDirectDrawSurface7() {
    if (m_hDC) DeleteDC(m_hDC);
    if (m_hBitmap) DeleteObject(m_hBitmap);
    if (m_bOwnsShadowBuffer && m_pShadowBuffer) delete[] m_pShadowBuffer;
    if (m_pTexture) m_pTexture->Release();
}

void m_IDirectDrawSurface7::SetAttachedBackbuffer(m_IDirectDrawSurface7* backbuffer) {
    m_pAttachedBackbuffer = backbuffer;
}

IDirect3DTexture9* m_IDirectDrawSurface7::GetTexture() {
    return m_pTexture;
}

DWORD m_IDirectDrawSurface7::GetWidth() {
    return m_width;
}

DWORD m_IDirectDrawSurface7::GetHeight() {
    return m_height;
}

bool m_IDirectDrawSurface7::IsPrimary() {
    return m_bIsPrimary;
}

bool m_IDirectDrawSurface7::IsBackbuffer() {
    return m_bIsBackbuffer;
}

WORD* m_IDirectDrawSurface7::GetShadowBuffer() {
    return m_pShadowBuffer;
}

void m_IDirectDrawSurface7::BlitToScreen() {
    if (!m_bIsPrimary) return;
    D3D9Renderer::GetInstance().PresentSurface(m_pShadowBuffer, m_width, m_height);
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::QueryInterface(REFIID riid, LPVOID* ppvObj) {
    if (!ppvObj) return E_POINTER;

    char guidStr[64];
    sprintf_s(guidStr, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        riid.Data1, riid.Data2, riid.Data3,
        riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
        riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);

    LOG_BASIC("IDirectDrawSurface7::QueryInterface %s", guidStr);

    if (riid == IID_IDirectDrawSurface7 || riid == IID_IDirectDrawSurface4 ||
        riid == IID_IDirectDrawSurface3 || riid == IID_IDirectDrawSurface2 ||
        riid == IID_IDirectDrawSurface || riid == IID_IUnknown) {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }

    if (riid.Data1 == 0x69C11C3E) {
        LOG_BASIC("  -> Surface: Intercepted IID_IDirectDrawGammaControl.");
        m_IDirectDrawGammaControl* pGamma = new m_IDirectDrawGammaControl();
        *ppvObj = pGamma;
        return S_OK;
    }

    LOG_BASIC("  -> E_NOINTERFACE (Unsupported)");
    *ppvObj = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE m_IDirectDrawSurface7::AddRef() {
    return ++m_refCount;
}

ULONG STDMETHODCALLTYPE m_IDirectDrawSurface7::Release() {
    if (m_refCount == 0) return 0;
    ULONG count = --m_refCount;
    if (count == 0) {
        LOG_BASIC("Surface %dx%d destroyed", m_width, m_height);
    }
    return count;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::AddAttachedSurface(LPDIRECTDRAWSURFACE7 s) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::AddOverlayDirtyRect(LPRECT r) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) {
    if ((dwFlags & DDBLT_COLORFILL) && lpDDBltFx && m_pShadowBuffer) {
        RECT dstRect;
        if (lpDestRect) {
            dstRect = *lpDestRect;
        }
        else {
            dstRect.left = 0; dstRect.top = 0;
            dstRect.right = m_width; dstRect.bottom = m_height;
        }

        if (dstRect.left < 0) dstRect.left = 0;
        if (dstRect.top < 0) dstRect.top = 0;
        if (dstRect.right > (LONG)m_width) dstRect.right = m_width;
        if (dstRect.bottom > (LONG)m_height) dstRect.bottom = m_height;

        if (dstRect.left < dstRect.right && dstRect.top < dstRect.bottom) {
            WORD fillColor = (WORD)(lpDDBltFx->dwFillColor & 0xFFFF);
            for (int y = dstRect.top; y < dstRect.bottom; y++) {
                WORD* pDst = m_pShadowBuffer + y * m_width + dstRect.left;
                int widthToFill = dstRect.right - dstRect.left;
                for (int x = 0; x < widthToFill; x++) {
                    pDst[x] = fillColor;
                }
            }
        }

        if (m_bIsPrimary) BlitToScreen();
        return DD_OK;
    }

    if (lpDDSrcSurface) {
        m_IDirectDrawSurface7* srcSurf = (m_IDirectDrawSurface7*)lpDDSrcSurface;
        if (!srcSurf->m_pShadowBuffer || !m_pShadowBuffer) return DD_OK;

        RECT srcRect = lpSrcRect ? *lpSrcRect : RECT{ 0, 0, (LONG)srcSurf->m_width, (LONG)srcSurf->m_height };
        RECT dstRect = lpDestRect ? *lpDestRect : RECT{ 0, 0, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top };

        int srcW = srcRect.right - srcRect.left;
        int srcH = srcRect.bottom - srcRect.top;
        int dstW = dstRect.right - dstRect.left;
        int dstH = dstRect.bottom - dstRect.top;

        if (srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0) return DD_OK;

        bool useColorKey = (dwFlags & DDBLT_KEYSRC) && srcSurf->m_hasColorKey;
        WORD ckLow = srcSurf->m_colorKeyLow;
        WORD ckHigh = srcSurf->m_colorKeyHigh;

        for (int y = 0; y < dstH; y++) {
            int srcY = srcRect.top + (y * srcH) / dstH;
            if (srcY >= (int)srcSurf->m_height) srcY = srcSurf->m_height - 1;

            int dstY = dstRect.top + y;
            if (dstY < 0 || dstY >= (int)m_height) continue;

            WORD* pDst = m_pShadowBuffer + dstY * m_width;
            WORD* pSrc = srcSurf->m_pShadowBuffer + srcY * srcSurf->m_width;

            for (int x = 0; x < dstW; x++) {
                int srcX = srcRect.left + (x * srcW) / dstW;
                if (srcX >= (int)srcSurf->m_width) srcX = srcSurf->m_width - 1;

                int dstX = dstRect.left + x;
                if (dstX < 0 || dstX >= (int)m_width) continue;

                WORD pixel = pSrc[srcX];
                if (!useColorKey || pixel < ckLow || pixel > ckHigh) {
                    pDst[dstX] = pixel;
                }
            }
        }

        if (m_bIsPrimary) BlitToScreen();
    }

    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::BltBatch(LPDDBLTBATCH b, DWORD c, DWORD f) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) {
    m_IDirectDrawSurface7* srcSurf = lpDDSrcSurface ? (m_IDirectDrawSurface7*)lpDDSrcSurface : nullptr;

    LOG_EXT("BltFast: dst(%s this:%p) <- src(%s this:%p) at (%d,%d) trans:0x%08X",
        m_bIsPrimary ? "PRIMARY" : (m_pAttachedBackbuffer ? "BACKBUFFER" : "OFFSCREEN"), this,
        srcSurf ? (srcSurf->m_bIsPrimary ? "PRIMARY" : (srcSurf->m_pAttachedBackbuffer ? "BACKBUFFER" : "OFFSCREEN")) : "NULL",
        srcSurf, dwX, dwY, dwTrans);

    if (!srcSurf) return DD_OK;

    if (!m_pShadowBuffer || !srcSurf->m_pShadowBuffer) {
        LOG_BASIC("  -> ERROR: Shadow buffer is NULL!");
        return DD_OK;
    }

    int srcX = lpSrcRect ? lpSrcRect->left : 0;
    int srcY = lpSrcRect ? lpSrcRect->top : 0;
    int copyW = lpSrcRect ? (lpSrcRect->right - lpSrcRect->left) : srcSurf->m_width;
    int copyH = lpSrcRect ? (lpSrcRect->bottom - lpSrcRect->top) : srcSurf->m_height;

    if (dwX < 0) { srcX -= dwX; copyW += dwX; dwX = 0; }
    if (dwY < 0) { srcY -= dwY; copyH += dwY; dwY = 0; }
    if (dwX + copyW > (int)m_width) copyW = m_width - dwX;
    if (dwY + copyH > (int)m_height) copyH = m_height - dwY;

    if (copyW > 0 && copyH > 0) {
        WORD* pDst = m_pShadowBuffer + dwY * m_width + dwX;
        WORD* pSrc = srcSurf->m_pShadowBuffer + srcY * srcSurf->m_width + srcX;

        bool useColorKey = (dwTrans & DDBLTFAST_SRCCOLORKEY) != 0 && srcSurf->m_hasColorKey;
        WORD ckLow = srcSurf->m_colorKeyLow;
        WORD ckHigh = srcSurf->m_colorKeyHigh;

        for (int row = 0; row < copyH; row++) {
            if (!useColorKey) {
                memcpy(pDst + row * m_width, pSrc + row * srcSurf->m_width, copyW * 2);
            }
            else {
                for (int col = 0; col < copyW; col++) {
                    WORD pixel = pSrc[row * srcSurf->m_width + col];
                    if (pixel < ckLow || pixel > ckHigh) {
                        pDst[row * m_width + col] = pixel;
                    }
                }
            }
        }
        LOG_EXT("  -> BltFast copied pixels successfully");
    }

    if (m_bIsPrimary) {
        BlitToScreen();
    }

    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::DeleteAttachedSurface(DWORD f, LPDIRECTDRAWSURFACE7 s) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::EnumAttachedSurfaces(LPVOID c, LPDDENUMSURFACESCALLBACK7 cb) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::EnumOverlayZOrders(DWORD f, LPVOID c, LPDDENUMSURFACESCALLBACK7 cb) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::Flip(LPDIRECTDRAWSURFACE7 s, DWORD f) {
    LOG_BASIC("Flip called on %s (this:%p)", m_bIsPrimary ? "PRIMARY" : "BACKBUFFER", this);

    if (m_bIsPrimary && m_pAttachedBackbuffer) {
        LOG_BASIC("  -> Swapping shadow buffers between Primary and Backbuffer");

        WORD* tempBuf = m_pShadowBuffer;
        m_pShadowBuffer = m_pAttachedBackbuffer->m_pShadowBuffer;
        m_pAttachedBackbuffer->m_pShadowBuffer = tempBuf;

        HDC tempDC = m_hDC;
        m_hDC = m_pAttachedBackbuffer->m_hDC;
        m_pAttachedBackbuffer->m_hDC = tempDC;

        HBITMAP tempBmp = m_hBitmap;
        m_hBitmap = m_pAttachedBackbuffer->m_hBitmap;
        m_pAttachedBackbuffer->m_hBitmap = tempBmp;

        bool tempOwns = m_bOwnsShadowBuffer;
        m_bOwnsShadowBuffer = m_pAttachedBackbuffer->m_bOwnsShadowBuffer;
        m_pAttachedBackbuffer->m_bOwnsShadowBuffer = tempOwns;

        BlitToScreen();
    }
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetAttachedSurface(LPDDSCAPS2 c, LPDIRECTDRAWSURFACE7* s) {
    LOG_BASIC("GetAttachedSurface called, caps: 0x%08X", c ? c->dwCaps : 0);
    if (c && (c->dwCaps & DDSCAPS_BACKBUFFER) && m_pAttachedBackbuffer) {
        LOG_BASIC("  -> Returning attached backbuffer");
        m_pAttachedBackbuffer->AddRef();
        *s = m_pAttachedBackbuffer;
        return DD_OK;
    }
    return DDERR_NOTFOUND;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetBltStatus(DWORD f) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetCaps(LPDDSCAPS2 c) {
    c->dwCaps = 0;
    if (m_bIsPrimary) c->dwCaps |= DDSCAPS_PRIMARYSURFACE;
    if (m_bIsBackbuffer) c->dwCaps |= DDSCAPS_BACKBUFFER;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetClipper(LPDIRECTDRAWCLIPPER* c) {
    return DDERR_NOCLIPPERATTACHED;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetColorKey(DWORD f, LPDDCOLORKEY k) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetDC(HDC* h) {
    LOG_BASIC("GetDC called on %s (this:%p)",
        m_bIsPrimary ? "PRIMARY" : (m_pAttachedBackbuffer ? "BACKBUFFER" : "OFFSCREEN"), this);
    if (m_hDC) {
        *h = m_hDC;
        LOG_BASIC("GetDC success: %p", m_hDC);
        return DD_OK;
    }
    return DDERR_NODC;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetFlipStatus(DWORD f) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetOverlayPosition(LPLONG x, LPLONG y) {
    return DDERR_NOTAOVERLAYSURFACE;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetPalette(LPDIRECTDRAWPALETTE* p) {
    return DDERR_NOPALETTEATTACHED;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetPixelFormat(LPDDPIXELFORMAT p) {
    p->dwSize = sizeof(DDPIXELFORMAT);
    p->dwFlags = DDPF_RGB;
    p->dwRGBBitCount = 16;
    p->dwRBitMask = 0xF800;
    p->dwGBitMask = 0x07E0;
    p->dwBBitMask = 0x001F;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetSurfaceDesc(LPDDSURFACEDESC2 d) {
    d->dwSize = sizeof(DDSURFACEDESC2);
    d->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    d->dwWidth = m_width;
    d->dwHeight = m_height;
    d->ddsCaps.dwCaps = 0;
    if (m_bIsPrimary) d->ddsCaps.dwCaps |= DDSCAPS_PRIMARYSURFACE;
    if (m_bIsBackbuffer) d->ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
    d->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    d->ddpfPixelFormat.dwFlags = DDPF_RGB;
    d->ddpfPixelFormat.dwRGBBitCount = 16;
    d->ddpfPixelFormat.dwRBitMask = 0xF800;
    d->ddpfPixelFormat.dwGBitMask = 0x07E0;
    d->ddpfPixelFormat.dwBBitMask = 0x001F;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::Initialize(LPDIRECTDRAW d, LPDDSURFACEDESC2 s) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::IsLost() {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::Lock(LPRECT r, LPDDSURFACEDESC2 d, DWORD f, HANDLE h) {
    LOG_BASIC("Surface %dx%d: Lock called", m_width, m_height);

    if (!m_pShadowBuffer) {
        m_pShadowBuffer = new WORD[m_width * m_height]();
        m_bOwnsShadowBuffer = true;
    }

    d->dwSize = sizeof(DDSURFACEDESC2);
    d->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE | DDSD_PITCH | DDSD_PIXELFORMAT;
    d->dwWidth = m_width;
    d->dwHeight = m_height;
    d->lpSurface = m_pShadowBuffer;
    d->lPitch = m_width * 2;
    d->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    d->ddpfPixelFormat.dwFlags = DDPF_RGB;
    d->ddpfPixelFormat.dwRGBBitCount = 16;
    d->ddpfPixelFormat.dwRBitMask = 0xF800;
    d->ddpfPixelFormat.dwGBitMask = 0x07E0;
    d->ddpfPixelFormat.dwBBitMask = 0x001F;

    LOG_BASIC("  -> Lock success (Shadow Buffer)");
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::ReleaseDC(HDC h) {
    LOG_BASIC("ReleaseDC called on %s (this:%p)",
        m_bIsPrimary ? "PRIMARY" : (m_pAttachedBackbuffer ? "BACKBUFFER" : "OFFSCREEN"), this);

    GdiFlush();

    if (m_pShadowBuffer && m_width > 0 && m_height > 0) {
        static int gdiDumpCount = 0;
        if (gdiDumpCount < 5) {
            char name[64];
            sprintf_s(name, "gdi_%p", this);
            Dumper::GetInstance().DumpSurface(name, m_pShadowBuffer, m_width, m_height, m_width * 2, 16);
            gdiDumpCount++;
        }
    }

    if (m_bIsPrimary) {
        BlitToScreen();
    }
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::Restore() {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::SetClipper(LPDIRECTDRAWCLIPPER c) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::SetColorKey(DWORD f, LPDDCOLORKEY k) {
    if (k) {
        m_colorKeyLow = (WORD)k->dwColorSpaceLowValue;
        m_colorKeyHigh = (WORD)k->dwColorSpaceHighValue;
        m_hasColorKey = true;
        LOG_BASIC("SetColorKey: %04X - %04X", m_colorKeyLow, m_colorKeyHigh);
    }
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::SetOverlayPosition(LONG x, LONG y) {
    return DDERR_NOTAOVERLAYSURFACE;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::SetPalette(LPDIRECTDRAWPALETTE p) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::Unlock(LPRECT r) {
    LOG_BASIC("Surface %dx%d: Unlock called", m_width, m_height);

    if (m_pShadowBuffer) {
        char name[64];
        sprintf_s(name, "shadow_%p", this);
        Dumper::GetInstance().DumpSurface(name, m_pShadowBuffer, m_width, m_height, m_width * 2, 16);
    }

    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::UpdateOverlay(LPRECT s, LPDIRECTDRAWSURFACE7 d, LPRECT dr, DWORD f, LPDDOVERLAYFX fx) {
    return DDERR_NOTAOVERLAYSURFACE;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::UpdateOverlayDisplay(DWORD f) {
    return DDERR_NOTAOVERLAYSURFACE;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::UpdateOverlayZOrder(DWORD f, LPDIRECTDRAWSURFACE7 s) {
    return DDERR_NOTAOVERLAYSURFACE;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetDDInterface(LPVOID* d) {
    return DDERR_GENERIC;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::PageLock(DWORD f) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::PageUnlock(DWORD f) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::SetSurfaceDesc(LPDDSURFACEDESC2 d, DWORD f) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::SetPrivateData(REFGUID t, LPVOID p, DWORD s, DWORD f) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetPrivateData(REFGUID t, LPVOID p, LPDWORD s) {
    return DDERR_NOTFOUND;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::FreePrivateData(REFGUID t) {
    return DDERR_NOTFOUND;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetUniquenessValue(LPDWORD v) {
    *v = 0;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::ChangeUniquenessValue() {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::SetPriority(DWORD p) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetPriority(LPDWORD p) {
    *p = 0;
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::SetLOD(DWORD l) {
    return DD_OK;
}

HRESULT STDMETHODCALLTYPE m_IDirectDrawSurface7::GetLOD(LPDWORD l) {
    *l = 0;
    return DD_OK;
}