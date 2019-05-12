#include "CApp.h"

#include <stdio.h>
#include <atlstr.h>
#include <ctype.h>
#include <stdio.h>

#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "Winmm.lib")

CImage CApp::m_Image;
const HBRUSH* CApp::m_pBackground;
const UINT CApp::m_nTimerDelay = 25000;

const HBRUSH g_BrushWhite = CreateSolidBrush(RGB(255, 255, 255));
const HBRUSH g_BrushGray  = CreateSolidBrush(RGB(104, 115, 135));

void CApp::showError(const char* lpErrorMessage) {
    char msgError[100];
    DWORD result = GetLastError();

    sprintf(msgError, "%s Error code: %d", lpErrorMessage, result);
    MessageBox(NULL, msgError, "Error!", MB_ICONERROR);
}

// Scales CApp::m_Image, puts itt to the center (of the window m_hWnd) and draws it
BOOL CApp::drawScaledCenteredImage(HDC hDC, PAINTSTRUCT& ps) {
    if (CApp::m_Image.isLoaded()) {
        CImage* img = &CApp::m_Image;
        FLOAT imgAspectRatio = FLOAT(img->getWidth()) / img->getHeight();
        LONG wndWidth = ps.rcPaint.right - ps.rcPaint.left;
        LONG wndHeight = ps.rcPaint.bottom - ps.rcPaint.top;
        UINT imgPosX, imgPosY, imgWidth, imgHeight;

        if (wndHeight * wndHeight * imgAspectRatio <
            wndWidth * wndWidth / imgAspectRatio) {
            imgHeight = wndHeight;
            imgWidth = INT(imgHeight * imgAspectRatio);
            imgPosY = 0;
            imgPosX = (wndWidth - imgWidth) / 2;
        } else {
            imgWidth = wndWidth;
            imgHeight = INT(imgWidth / imgAspectRatio);
            imgPosX = 0;
            imgPosY = (wndHeight - imgHeight) / 2;
        }

        CApp::m_Image.render(hDC, imgPosX, imgPosY, imgWidth, imgHeight);

        return TRUE;
    }

    return FALSE;
}

BOOL CApp::registerWndClass() {
    WNDCLASSEX wClass;
    ZeroMemory(&wClass, sizeof(WNDCLASSEX));

    wClass.cbClsExtra = NULL;
    wClass.cbSize = sizeof(WNDCLASSEX);
    wClass.cbWndExtra = NULL;
    wClass.hbrBackground = g_BrushWhite;
    wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wClass.hIcon = NULL;
    wClass.hIconSm = NULL;
    wClass.hInstance = m_hInstance;
    wClass.lpfnWndProc = (WNDPROC)WndProc;
    wClass.lpszClassName = m_sWndClassName.c_str();
    wClass.lpszMenuName = NULL;
    wClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassEx(&wClass)) {
        showError("Window class creation error!");
        return FALSE;
    }

    return TRUE;
}

BOOL CApp::createWnd() {
    m_nWndWidth = GetDeviceCaps(GetDC(NULL), HORZRES);
    m_nWndHeight = GetDeviceCaps(GetDC(NULL), VERTRES);
    m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, m_sWndClassName.c_str(), "Map",
        WS_POPUP, 0, 0, m_nWndWidth, m_nWndHeight, NULL, NULL, m_hInstance, NULL);

    if (!m_hWnd) {
        showError("Window creation error");
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK CApp::WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
    static char accumMessage[3];
    static int accumPos = 0;

    switch(nMsg) {
		case WM_DESTROY: {
		    PostQuitMessage(0);
		    return 0;
        }
		break;

        //Map sends message: NUM_NUM_(VK_SPACE) in a row where each entity have 32 bit size
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
                return 0;
            }

            if (wParam == VK_SPACE) {
                if (accumPos != 2) { // Update something
                    accumPos = 0;
                    break;
                }

                char temp[256];
                wchar_t path[256];
                int commonIndex; /* Stores index of the end of the common part of the
                                 picture path and sound path */

                temp[0] = '\0';
                accumMessage[2] = '\0'; // Now in accumMessage: NUM(2 bytes)_'\0'

                strcat(temp, "media/");
                strcat(temp, accumMessage);
                strcat(temp, "/"); // Now in temp: "media/NUM/"; it's common part
                commonIndex = strlen(temp); // Saves common part's index

                // Now, loading an image:
                strcat(temp, "picture.jpg");
                //GetFullPathName(temp, sizeof(path), temp, NULL); // Optional

                mbstowcs(path, temp, sizeof(temp) / sizeof(char));

                CApp::m_Image.open(path); // TODO: error handling
                if (!CApp::m_Image.isLoaded()) {
                    char errorMsg[256];

                    sprintf(errorMsg, "Cannot open the image: %s", temp);
                    CApp::showError(errorMsg);
                    break;
                }

                accumPos = 0;
                CApp::m_pBackground = &g_BrushGray;
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE); // Sends WM_PAINT message

                // Playing a sound:
                char commandMCI[256];
                temp[commonIndex] = '\0';
                strcat(temp, "sound.mp3"); // Now in temp: "media/NUM/sound.mp3"
                sprintf(commandMCI, "play \"%s\"", temp);
                mciSendString("close all", NULL, 0, 0);
                mciSendString(commandMCI, NULL, 0, 0);
                SetTimer(hWnd, 1, CApp::m_nTimerDelay, CApp::TimerProc);

                break;
            }

            if (isdigit((char)wParam)) { // Stores digits ONLY
                accumPos = accumPos >= 3 ? 0 : accumPos;
                accumMessage[accumPos] = (char)wParam;
                accumPos++;
            }
        }
        break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC wndDC = BeginPaint(hWnd, &ps);
            FillRect(wndDC, &ps.rcPaint, *CApp::m_pBackground);

            CApp::drawScaledCenteredImage(wndDC, ps);

            EndPaint(hWnd, &ps);
            ReleaseDC(hWnd, wndDC);
        }
        break;

        default:
        break;
	}

	return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

VOID CALLBACK CApp::TimerProc(HWND hWnd, UINT message, UINT timerID, DWORD dwTime) {
    CApp::m_pBackground = &g_BrushWhite;
    CApp::m_Image.open(L"media/logos/logo.jpg");
    KillTimer(hWnd, 1);
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
}

INT CApp::exec(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine) {
    m_hInstance = hInstance;
    if (!registerWndClass() || !createWnd()) {
        return 1;
    }

    ShowWindow(m_hWnd, SW_SHOW);
    BringWindowToTop(m_hWnd);
    ShowCursor(FALSE); // Hide cursor; it's more eye-candy'er
    CApp::m_pBackground = &g_BrushWhite;
    CApp::m_Image.open(L"media/logos/logo.jpg"); // Shows logo on startup

    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));
    while (GetMessage(&msg, NULL, 0, 0)) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
    }

    return 0;
}

CApp::~CApp() {
    DestroyWindow(m_hWnd);
    UnregisterClass(m_sWndClassName.c_str(), m_hInstance);
}
