#pragma once
#include <ddraw.h>
#include <d3d9.h>
#include <windows.h>

class m_IDirectDrawSurface7;

// ============================================================================
// IDirectDrawGammaControl Wrapper (Bridges to D3D9 Gamma Ramp)
// ============================================================================
class m_IDirectDrawGammaControl : public IDirectDrawGammaControl {
private:
    ULONG m_refCount;
public:
    m_IDirectDrawGammaControl();
    virtual ~m_IDirectDrawGammaControl();

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    // IDirectDrawGammaControl
    HRESULT STDMETHODCALLTYPE GetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRamp) override;
    HRESULT STDMETHODCALLTYPE SetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRamp) override;
};

class m_IDirectDraw7 : public IDirectDraw7 {
private:
    ULONG m_refCount;
    m_IDirectDrawSurface7* m_pPrimary;
    m_IDirectDrawSurface7* m_pBackbuffer;
    DWORD m_currentWidth;
    DWORD m_currentHeight;

public:
    m_IDirectDraw7();
    ~m_IDirectDraw7();

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    // IDirectDraw methods
    HRESULT STDMETHODCALLTYPE Compact() override;
    HRESULT STDMETHODCALLTYPE CreateClipper(DWORD f, LPDIRECTDRAWCLIPPER* c, IUnknown* o) override;
    HRESULT STDMETHODCALLTYPE CreatePalette(DWORD f, LPPALETTEENTRY t, LPDIRECTDRAWPALETTE* p, IUnknown* o) override;
    HRESULT STDMETHODCALLTYPE CreateSurface(LPDDSURFACEDESC2 d, LPDIRECTDRAWSURFACE7* s, IUnknown* o) override;
    HRESULT STDMETHODCALLTYPE DuplicateSurface(LPDIRECTDRAWSURFACE7 s, LPDIRECTDRAWSURFACE7* d) override;
    HRESULT STDMETHODCALLTYPE EnumDisplayModes(DWORD f, LPDDSURFACEDESC2 d, LPVOID c, LPDDENUMMODESCALLBACK2 cb) override;
    HRESULT STDMETHODCALLTYPE EnumSurfaces(DWORD f, LPDDSURFACEDESC2 d, LPVOID c, LPDDENUMSURFACESCALLBACK7 cb) override;
    HRESULT STDMETHODCALLTYPE FlipToGDISurface() override;
    HRESULT STDMETHODCALLTYPE GetCaps(LPDDCAPS d, LPDDCAPS h) override;
    HRESULT STDMETHODCALLTYPE GetDisplayMode(LPDDSURFACEDESC2 d) override;
    HRESULT STDMETHODCALLTYPE GetFourCCCodes(LPDWORD n, LPDWORD c) override;
    HRESULT STDMETHODCALLTYPE GetGDISurface(LPDIRECTDRAWSURFACE7* s) override;
    HRESULT STDMETHODCALLTYPE GetMonitorFrequency(LPDWORD f) override;
    HRESULT STDMETHODCALLTYPE GetScanLine(LPDWORD s) override;
    HRESULT STDMETHODCALLTYPE GetVerticalBlankStatus(LPBOOL b) override;
    HRESULT STDMETHODCALLTYPE Initialize(GUID* g) override;
    HRESULT STDMETHODCALLTYPE RestoreDisplayMode() override;
    HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hWnd, DWORD dwFlags) override;
    HRESULT STDMETHODCALLTYPE SetDisplayMode(DWORD w, DWORD h, DWORD b, DWORD r, DWORD f) override;
    HRESULT STDMETHODCALLTYPE WaitForVerticalBlank(DWORD f, HANDLE h) override;

    // IDirectDraw2+ methods
    HRESULT STDMETHODCALLTYPE GetAvailableVidMem(LPDDSCAPS2 c, LPDWORD t, LPDWORD f) override;

    // IDirectDraw4+ methods
    HRESULT STDMETHODCALLTYPE GetSurfaceFromDC(HDC h, LPDIRECTDRAWSURFACE7* s) override;
    HRESULT STDMETHODCALLTYPE RestoreAllSurfaces() override;
    HRESULT STDMETHODCALLTYPE TestCooperativeLevel() override;
    HRESULT STDMETHODCALLTYPE GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 d, DWORD f) override;

    // IDirectDraw7 methods
    HRESULT STDMETHODCALLTYPE StartModeTest(LPSIZE m, DWORD n, DWORD f) override;
    HRESULT STDMETHODCALLTYPE EvaluateMode(DWORD f, DWORD* r) override;
};

class m_IDirectDrawSurface7 : public IDirectDrawSurface7 {
private:
    ULONG m_refCount;
    IDirect3DTexture9* m_pTexture;
    DWORD m_width;
    DWORD m_height;
    bool m_bIsPrimary;
    bool m_bIsBackbuffer;
    m_IDirectDrawSurface7* m_pAttachedBackbuffer;

    WORD m_colorKeyLow;
    WORD m_colorKeyHigh;
    bool m_hasColorKey;

    WORD* m_pShadowBuffer;
    HDC m_hDC;
    HBITMAP m_hBitmap;
    bool m_bOwnsShadowBuffer;

public:
    m_IDirectDrawSurface7(DWORD w, DWORD h, bool isPrimary, bool isBackbuffer);
    ~m_IDirectDrawSurface7();

    void SetAttachedBackbuffer(m_IDirectDrawSurface7* backbuffer);
    IDirect3DTexture9* GetTexture();
    DWORD GetWidth();
    DWORD GetHeight();
    bool IsPrimary();
    bool IsBackbuffer();
    WORD* GetShadowBuffer();

    void BlitToScreen();

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    // IDirectDrawSurface methods
    HRESULT STDMETHODCALLTYPE AddAttachedSurface(LPDIRECTDRAWSURFACE7 s) override;
    HRESULT STDMETHODCALLTYPE AddOverlayDirtyRect(LPRECT r) override;
    HRESULT STDMETHODCALLTYPE Blt(LPRECT d, LPDIRECTDRAWSURFACE7 s, LPRECT sr, DWORD f, LPDDBLTFX fx) override;
    HRESULT STDMETHODCALLTYPE BltBatch(LPDDBLTBATCH b, DWORD c, DWORD f) override;
    HRESULT STDMETHODCALLTYPE BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) override;
    HRESULT STDMETHODCALLTYPE DeleteAttachedSurface(DWORD f, LPDIRECTDRAWSURFACE7 s) override;
    HRESULT STDMETHODCALLTYPE EnumAttachedSurfaces(LPVOID c, LPDDENUMSURFACESCALLBACK7 cb) override;
    HRESULT STDMETHODCALLTYPE EnumOverlayZOrders(DWORD f, LPVOID c, LPDDENUMSURFACESCALLBACK7 cb) override;
    HRESULT STDMETHODCALLTYPE Flip(LPDIRECTDRAWSURFACE7 s, DWORD f) override;
    HRESULT STDMETHODCALLTYPE GetAttachedSurface(LPDDSCAPS2 c, LPDIRECTDRAWSURFACE7* s) override;
    HRESULT STDMETHODCALLTYPE GetBltStatus(DWORD f) override;
    HRESULT STDMETHODCALLTYPE GetCaps(LPDDSCAPS2 c) override;
    HRESULT STDMETHODCALLTYPE GetClipper(LPDIRECTDRAWCLIPPER* c) override;
    HRESULT STDMETHODCALLTYPE GetColorKey(DWORD f, LPDDCOLORKEY k) override;
    HRESULT STDMETHODCALLTYPE GetDC(HDC* h) override;
    HRESULT STDMETHODCALLTYPE GetFlipStatus(DWORD f) override;
    HRESULT STDMETHODCALLTYPE GetOverlayPosition(LPLONG x, LPLONG y) override;
    HRESULT STDMETHODCALLTYPE GetPalette(LPDIRECTDRAWPALETTE* p) override;
    HRESULT STDMETHODCALLTYPE GetPixelFormat(LPDDPIXELFORMAT p) override;
    HRESULT STDMETHODCALLTYPE GetSurfaceDesc(LPDDSURFACEDESC2 d) override;
    HRESULT STDMETHODCALLTYPE Initialize(LPDIRECTDRAW d, LPDDSURFACEDESC2 s) override;
    HRESULT STDMETHODCALLTYPE IsLost() override;
    HRESULT STDMETHODCALLTYPE Lock(LPRECT r, LPDDSURFACEDESC2 d, DWORD f, HANDLE h) override;
    HRESULT STDMETHODCALLTYPE ReleaseDC(HDC h) override;
    HRESULT STDMETHODCALLTYPE Restore() override;
    HRESULT STDMETHODCALLTYPE SetClipper(LPDIRECTDRAWCLIPPER c) override;
    HRESULT STDMETHODCALLTYPE SetColorKey(DWORD f, LPDDCOLORKEY k) override;
    HRESULT STDMETHODCALLTYPE SetOverlayPosition(LONG x, LONG y) override;
    HRESULT STDMETHODCALLTYPE SetPalette(LPDIRECTDRAWPALETTE p) override;
    HRESULT STDMETHODCALLTYPE Unlock(LPRECT r) override;
    HRESULT STDMETHODCALLTYPE UpdateOverlay(LPRECT s, LPDIRECTDRAWSURFACE7 d, LPRECT dr, DWORD f, LPDDOVERLAYFX fx) override;
    HRESULT STDMETHODCALLTYPE UpdateOverlayDisplay(DWORD f) override;
    HRESULT STDMETHODCALLTYPE UpdateOverlayZOrder(DWORD f, LPDIRECTDRAWSURFACE7 s) override;

    // IDirectDrawSurface2+ methods
    HRESULT STDMETHODCALLTYPE GetDDInterface(LPVOID* d) override;
    HRESULT STDMETHODCALLTYPE PageLock(DWORD f) override;
    HRESULT STDMETHODCALLTYPE PageUnlock(DWORD f) override;

    // IDirectDrawSurface3+ methods
    HRESULT STDMETHODCALLTYPE SetSurfaceDesc(LPDDSURFACEDESC2 d, DWORD f) override;

    // IDirectDrawSurface4+ methods
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID t, LPVOID p, DWORD s, DWORD f) override;
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID t, LPVOID p, LPDWORD s) override;
    HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID t) override;
    HRESULT STDMETHODCALLTYPE GetUniquenessValue(LPDWORD v) override;
    HRESULT STDMETHODCALLTYPE ChangeUniquenessValue() override;

    // IDirectDrawSurface7 methods
    HRESULT STDMETHODCALLTYPE SetPriority(DWORD p) override;
    HRESULT STDMETHODCALLTYPE GetPriority(LPDWORD p) override;
    HRESULT STDMETHODCALLTYPE SetLOD(DWORD l) override;
    HRESULT STDMETHODCALLTYPE GetLOD(LPDWORD l) override;
};