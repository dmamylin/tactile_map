#ifndef C_IMAGE_H
#define C_IMAGE_H

#include <atlbase.h>
#include <wincodec.h>
#include <wincodecsdk.h>

class CImage {
protected:
    CComPtr<IWICBitmapDecoder> m_pDecoder;
    CComPtr<IWICBitmapFrameDecode> m_pFrame;
    CComPtr<IWICFormatConverter> m_pConvertedFrame;
    UINT m_nWidth;
    UINT m_nHeight;

    virtual VOID cleanup();

public:
    CImage();
    virtual ~CImage();

    // Opens the nFrame'th frame of the given image
    virtual VOID open(const wchar_t* lpszFilename, UINT nFrame = 0);

    // Returns TRUE if image was loaded successfully and FALSE otherwise
    virtual BOOL isLoaded() const;

    virtual UINT getWidth() const; // Returns width of this image
    virtual UINT getHeight() const; // Returns height of this image

    // Renders this image on the given hDC at the point (x, y) and with size cx, cy
    virtual VOID render(HDC hDC, UINT x, UINT y, UINT cx, UINT cy);
};

#endif
