#include "CApp.h"
#include "CImage.h"

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd) {
    CApp app;
    return app.exec(hInstance, hPrevInstance, lpCmdLine);
}
