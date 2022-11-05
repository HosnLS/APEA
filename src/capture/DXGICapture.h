//
// Created by HosnLS on 2022/7/23.
//

#ifndef FPSHELPER_DXGI_H
#define FPSHELPER_DXGI_H

#include <d3d11.h>
#include <dxgi1_2.h>

class VideoDXGICaptor {
public:
    VideoDXGICaptor();

    ~VideoDXGICaptor();

    virtual BOOL CaptureImage(RECT &rect, void *pData);

    virtual BOOL CaptureImage(void *pData);

    virtual BOOL ResetDevice();

private:
    BOOL Init();

    VOID Deinit();

    BOOL AttatchToThread(VOID);

    BOOL QueryFrame(RECT &rect, void *pImgData);

private:
    IDXGIResource *zhDesktopResource;
    DXGI_OUTDUPL_FRAME_INFO zFrameInfo;
    ID3D11Texture2D *zhAcquiredDesktopImage;
    IDXGISurface *zhStagingSurf;

    BOOL m_bInit, g_bAttach;

    ID3D11Device *m_hDevice;
    ID3D11DeviceContext *m_hContext;

    IDXGIOutputDuplication *m_hDeskDupl;
    DXGI_OUTPUT_DESC m_dxgiOutDesc;
};

#endif //FPSHELPER_DXGI_H
