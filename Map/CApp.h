#ifndef C_APP_H
#define C_APP_H

#include <Windows.h>
#include <string>

#include "CImage.h"

class CApp {
private:
    // Shows a message box with the given error. Also, prints an error code
    static VOID showError(const char* lpErrorMessage);
    static BOOL drawScaledCenteredImage(HDC hDC, PAINTSTRUCT& ps);
    BOOL registerWndClass();
    BOOL createWnd();

    // Window callback function
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
    static VOID    CALLBACK TimerProc(HWND hWnd, UINT message, UINT timerID, DWORD dwTime);

    std::string m_sWndClassName;
    INT         m_nWndWidth;
    INT         m_nWndHeight;
    HINSTANCE   m_hInstance;
    HWND        m_hWnd;

    static CImage  m_Image;

    static const HBRUSH*  m_pBackground;
    static const UINT     m_nTimerDelay;

public:
    CApp() :
      m_sWndClassName("Map Window Class"),
      m_nWndWidth(0),
      m_nWndHeight(0),
      m_hInstance(NULL),
      m_hWnd(NULL)
      {}
    ~CApp();


    INT  exec(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine);
};

#endif
