#include "CImage.h"

#pragma comment(lib, "Windowscodecs.lib")

#include "CWICImagingFactory.h"

VOID CImage::cleanup() {
    if (m_pConvertedFrame) {
        m_pConvertedFrame.Release();
        m_pConvertedFrame = nullptr;
    }

    if (m_pFrame) {
        m_pFrame.Release();
        m_pFrame = nullptr;
    }

    if (m_pDecoder) {
        m_pDecoder.Release();
        m_pDecoder = nullptr;
    }
}

CImage::CImage() :
    m_pDecoder(nullptr),
    m_pFrame(nullptr),
    m_pConvertedFrame(nullptr),
    m_nWidth(0),
    m_nHeight(0)
{
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
}

CImage::~CImage() {
    cleanup();
    CoUninitialize();
}

VOID CImage::open(const wchar_t* lpszFilename, UINT nFrame) {
    try {
        HRESULT hr;

        cleanup();

        IWICImagingFactory* pFactory = CWICImagingFactory::getInstance().getFactory();

        if (!pFactory) {
            throw WINCODEC_ERR_NOTINITIALIZED;
        }

        hr = pFactory->CreateDecoderFromFilename(lpszFilename, NULL, GENERIC_READ,
            WICDecodeMetadataCacheOnDemand, &m_pDecoder);
        if (FAILED(hr)) {
            throw hr;
        }

        UINT nCount = 0;

        if (SUCCEEDED(m_pDecoder->GetFrameCount(&nCount))) {
            if (nFrame >= nCount) {
                nFrame = nCount - 1;
            }
        }

        hr = m_pDecoder->GetFrame(nFrame, &m_pFrame);
        if (FAILED(hr)) {
            throw hr;
        }

        hr = m_pFrame->GetSize(&m_nWidth, &m_nHeight);
        if (FAILED(hr)) {
            throw hr;
        }

        hr = pFactory->CreateFormatConverter(&m_pConvertedFrame);
        if (FAILED(hr)) {
            throw hr;
        }

        hr = m_pConvertedFrame->Initialize(m_pFrame, GUID_WICPixelFormat32bppBGR,
            WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
        if (FAILED(hr)) {
            throw hr;
        }
    } catch (...) {
        cleanup();
        throw;
    }
}

BOOL CImage::isLoaded() const {
    return m_pConvertedFrame != nullptr;
}

UINT CImage::getWidth() const {
    return m_nWidth;
}

UINT CImage::getHeight() const {
    return m_nHeight;
}

VOID CImage::render(HDC hDC, UINT x, UINT y, UINT cx, UINT cy) {
    HRESULT hr;

    if (!isLoaded()) {
        throw WINCODEC_ERR_WRONGSTATE;
    }

    IWICImagingFactory* pFactory = CWICImagingFactory::getInstance().getFactory();
    if (!pFactory) {
        throw WINCODEC_ERR_NOTINITIALIZED;
    }

    CComPtr<IWICBitmapScaler> pScaler = nullptr;
    hr = pFactory->CreateBitmapScaler(&pScaler);
    if (FAILED(hr)) {
        throw hr;
    }
    hr = pScaler->Initialize(m_pConvertedFrame, cx, cy,
        WICBitmapInterpolationModeFant);
    if (FAILED(hr)) {
        throw hr;
    }

    HBITMAP hBmp = NULL;
    try {
        HDC hdcScreen = GetDC(NULL);
        if (!hdcScreen) {
            throw 1;
        }

        BITMAPINFO bmpInfo;
        ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
        bmpInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth       = cx;
        bmpInfo.bmiHeader.biHeight      = -(LONG)cy;
        bmpInfo.bmiHeader.biPlanes      = 1;
        bmpInfo.bmiHeader.biBitCount    = 32;
        bmpInfo.bmiHeader.biCompression = BI_RGB;

        VOID* pvImageBits = nullptr;
        hBmp = CreateDIBSection(hdcScreen, &bmpInfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
        if (!hBmp) {
            throw 2;
        }

        ReleaseDC(NULL, hdcScreen);

        UINT nStride = ((((cx * 32) + 31) >> 5) << 2);
        UINT nImage = nStride * cy;
        hr = pScaler->CopyPixels(nullptr, nStride, nImage,
            reinterpret_cast<BYTE*>(pvImageBits));

        SetDIBitsToDevice(hDC, x, y, cx, cy, 0, 0, 0, cy,
            pvImageBits, &bmpInfo, DIB_RGB_COLORS);

        DeleteObject(hBmp);
    } catch (...) {
        if (hBmp) {
            DeleteObject(hBmp);
        }
        throw;
    }
}
