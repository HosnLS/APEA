//
// Created by HosnLS on 2022/7/23.
//

#include "DXGICapture.h"

//#include "stdafx.h"
#include <Windows.h>
#include <gdiplus.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#define RESET_OBJECT(obj) { if(obj) obj->Release(); obj = NULL; }

VideoDXGICaptor::VideoDXGICaptor() {
    m_bInit = FALSE;
    g_bAttach = FALSE;

    m_hDevice = NULL;
    m_hContext = NULL;
    m_hDeskDupl = NULL;

    ZeroMemory(&m_dxgiOutDesc, sizeof(m_dxgiOutDesc));

    this->Init();
}

VideoDXGICaptor::~VideoDXGICaptor() {
    Deinit();
}

BOOL VideoDXGICaptor::Init() {
    HRESULT hr = S_OK;

    if (m_bInit) {
        return FALSE;
    }

    // Driver types supported
    D3D_DRIVER_TYPE DriverTypes[] =
            {
                    D3D_DRIVER_TYPE_HARDWARE,
                    D3D_DRIVER_TYPE_WARP,
                    D3D_DRIVER_TYPE_REFERENCE,
            };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    // Feature levels supported
    D3D_FEATURE_LEVEL FeatureLevels[] =
            {
                    D3D_FEATURE_LEVEL_12_0,
                    D3D_FEATURE_LEVEL_11_1,
                    D3D_FEATURE_LEVEL_11_0,
                    D3D_FEATURE_LEVEL_10_1,
                    D3D_FEATURE_LEVEL_10_0,
                    D3D_FEATURE_LEVEL_9_1
            };
    UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

    D3D_FEATURE_LEVEL FeatureLevel;

    //
    // Create D3D device
    //
    for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex) {
        hr = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels,
                               D3D11_SDK_VERSION, &m_hDevice, &FeatureLevel, &m_hContext);
        if (SUCCEEDED(hr)) {
            break;
        }
    }
    if (FAILED(hr)) {
        return FALSE;
    }

    //
    // Get DXGI device
    //
    IDXGIDevice *hDxgiDevice = NULL;
    hr = m_hDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&hDxgiDevice));
    if (FAILED(hr)) {
        return FALSE;
    }

    //
    // Get DXGI adapter
    //
    IDXGIAdapter *hDxgiAdapter = NULL;
    hr = hDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **>(&hDxgiAdapter));
    RESET_OBJECT(hDxgiDevice);
    if (FAILED(hr)) {
        return FALSE;
    }

    //
    // Get output
    //
    INT nOutput = 0;
    IDXGIOutput *hDxgiOutput = NULL;
    hr = hDxgiAdapter->EnumOutputs(nOutput, &hDxgiOutput);
    RESET_OBJECT(hDxgiAdapter);
    if (FAILED(hr)) {
        return FALSE;
    }

    //
    // get output description struct
    //
    hDxgiOutput->GetDesc(&m_dxgiOutDesc);

    //
    // QI for Output 1
    //
    IDXGIOutput1 *hDxgiOutput1 = NULL;
    hr = hDxgiOutput->QueryInterface(__uuidof(hDxgiOutput1), reinterpret_cast<void **>(&hDxgiOutput1));
    RESET_OBJECT(hDxgiOutput);
    if (FAILED(hr)) {
        return FALSE;
    }

    //
    // Create desktop duplication
    //
    hr = hDxgiOutput1->DuplicateOutput(m_hDevice, &m_hDeskDupl);
    RESET_OBJECT(hDxgiOutput1);
    if (FAILED(hr)) {
        return FALSE;
    }

    // 初始化成功
    m_bInit = TRUE;
    return TRUE;
}

VOID VideoDXGICaptor::Deinit() {
    if (!m_bInit) {
        return;
    }

    m_bInit = FALSE;

    if (m_hDeskDupl) {
        m_hDeskDupl->Release();
        m_hDeskDupl = NULL;
    }

    if (m_hDevice) {
        m_hDevice->Release();
        m_hDevice = NULL;
    }

    if (m_hContext) {
        m_hContext->Release();
        m_hContext = NULL;
    }
// #endif
}

BOOL VideoDXGICaptor::AttatchToThread(VOID) {
    if (g_bAttach) {
        return TRUE;
    }

    HDESK hCurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
    if (!hCurrentDesktop) {
        return FALSE;
    }

    // Attach desktop to this thread
    BOOL bDesktopAttached = SetThreadDesktop(hCurrentDesktop);
    CloseDesktop(hCurrentDesktop);
    hCurrentDesktop = NULL;

    g_bAttach = TRUE;

    return bDesktopAttached;
}

BOOL VideoDXGICaptor::CaptureImage(RECT &rect, void *pData) {
    return QueryFrame(rect, pData);
}

BOOL VideoDXGICaptor::CaptureImage(void *pData) {
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = m_dxgiOutDesc.DesktopCoordinates.right;
    rect.bottom = m_dxgiOutDesc.DesktopCoordinates.bottom;
    return QueryFrame(rect, pData);
}

BOOL VideoDXGICaptor::ResetDevice() {
    Deinit();
    return Init();
}

/// 获取桌面图像
/// \param rect
/// \param pImgData BGRA
/// \return
BOOL VideoDXGICaptor::QueryFrame(RECT &rect, void *pImgData) {
    if (!m_bInit || !AttatchToThread()) {
        return FALSE;
    }

    IDXGIResource *hDesktopResource = NULL;
    DXGI_OUTDUPL_FRAME_INFO FrameInfo;
    HRESULT hr = m_hDeskDupl->AcquireNextFrame(0, &FrameInfo, &hDesktopResource);
    if (FAILED(hr)) {
        //
        // 在一些win10的系统上,如果桌面没有变化的情况下，;
        // 这里会发生超时现象，但是这并不是发生了错误，而是系统优化了刷新动作导致的。;
        // 所以，这里没必要返回FALSE，返回不带任何数据的TRUE即可;
        //
        return TRUE;
    }

    //
    // query next frame staging buffer
    //
    ID3D11Texture2D *hAcquiredDesktopImage = NULL;
    hr = hDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&hAcquiredDesktopImage));
    RESET_OBJECT(hDesktopResource);
    if (FAILED(hr)) {
        return FALSE;
    }

    //
    // copy old description
    //
    D3D11_TEXTURE2D_DESC frameDescriptor;
    hAcquiredDesktopImage->GetDesc(&frameDescriptor);

    //
    // create a new staging buffer for fill frame image
    //
    ID3D11Texture2D *hNewDesktopImage = NULL;
    frameDescriptor.Usage = D3D11_USAGE_STAGING;
    frameDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    frameDescriptor.BindFlags = 0;
    frameDescriptor.MiscFlags = 0;
    frameDescriptor.MipLevels = 1;
    frameDescriptor.ArraySize = 1;
    frameDescriptor.SampleDesc.Count = 1;
    hr = m_hDevice->CreateTexture2D(&frameDescriptor, NULL, &hNewDesktopImage);
    if (FAILED(hr)) {
        RESET_OBJECT(hAcquiredDesktopImage);
        m_hDeskDupl->ReleaseFrame();
        return FALSE;
    }

    //
    // copy next staging buffer to new staging buffer
    //
    m_hContext->CopyResource(hNewDesktopImage, hAcquiredDesktopImage);

    RESET_OBJECT(hAcquiredDesktopImage);
    m_hDeskDupl->ReleaseFrame();

    //
    // create staging buffer for map bits
    //
    IDXGISurface *hStagingSurf = NULL;
    hr = hNewDesktopImage->QueryInterface(__uuidof(IDXGISurface), (void **) (&hStagingSurf));
    RESET_OBJECT(hNewDesktopImage);
    if (FAILED(hr)) {
        return FALSE;
    }

    //
    // copy bits to user space
    //
    DXGI_MAPPED_RECT mappedRect;
    hr = hStagingSurf->Map(&mappedRect, DXGI_MAP_READ);
    if (SUCCEEDED(hr)) {
        INT canvasWidth = rect.right - rect.left;
        INT canvasHeight = rect.bottom - rect.top;
        INT ScreenWidth = mappedRect.Pitch / 4;
        for (INT indRow = 0; indRow < canvasHeight; indRow++) {
            memcpy((BYTE *) pImgData + indRow * canvasWidth * 4,
                   (BYTE *) mappedRect.pBits + ((indRow + rect.top) * ScreenWidth + rect.left) * 4,
                   canvasWidth * 4);
        }
        hStagingSurf->Unmap();
    }

    RESET_OBJECT(hStagingSurf);
    return SUCCEEDED(hr);
}