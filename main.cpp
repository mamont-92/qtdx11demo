//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <tuple>



namespace wnd {

enum class CreateResult{
    OK = 0,
    RegisterClassFailed,
    CreateInstanceFailed
};

inline static LPCWSTR toErrorStr(CreateResult val)
{
    switch (val) {
    case CreateResult::OK:
        return L"OK";
    case CreateResult::RegisterClassFailed:
        return L"Failed registering window class!";
    case CreateResult::CreateInstanceFailed:
        return L"Failed creation window instance!";
    }
    return L"Unknown error!";
}

constexpr ATOM WINAPI_FAILED = 0;
static const wchar_t g_className[] = L"DX Windowd Class";

std::tuple<CreateResult, HWND> createWindow(HINSTANCE hInstance, int cmdShowFlags, LPCWSTR title, const RECT & coords);
int startAppLoop();


static LRESULT CALLBACK wndCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

template<typename T>
void showMessageError(T errorVal)
{
    MessageBox(nullptr, toErrorStr(errorVal) , L"error!", MB_ICONERROR);
}

} //namespace wnd


//-------------main--func------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR cmdStr, int cmdShowFlags)
{
    RECT wndRect {0, 0, 800, 600};
    auto [createResult, wndDesc] = wnd::createWindow(hInstance, cmdShowFlags, L"DX11 Hello!", wndRect);
    if (createResult != wnd::CreateResult::OK){
        wnd::showMessageError(createResult);
        return 1;
    }

    return wnd::startAppLoop();
}
//-----------------------------------------------------------------------------------------------------------

namespace wnd {

std::tuple<CreateResult, HWND> createWindow(HINSTANCE hInstance, int cmdShowFlags, LPCWSTR title, const RECT & coords)
{
    WNDCLASSEX wndClass;

    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.cbSize = sizeof(wndClass);
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hIcon = LoadIcon(hInstance, IDC_NO);
    wndClass.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wndClass.hInstance = hInstance;
    wndClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = g_className;
    wndClass.lpfnWndProc = wndCallback;

    if (RegisterClassEx(&wndClass) == WINAPI_FAILED)
        return std::make_tuple(CreateResult::RegisterClassFailed, nullptr);

    auto styleFlags = WS_SYSMENU | WS_SIZEBOX;

    auto wndHandle = CreateWindowEx(0L, g_className, title, styleFlags, coords.left, coords.top,
                                    (coords.right - coords.left), (coords.bottom - coords.top), nullptr, nullptr, hInstance, nullptr);
    if (wndHandle == nullptr)
        return std::make_tuple(CreateResult::CreateInstanceFailed, nullptr);

    ShowWindow(wndHandle, cmdShowFlags);
    UpdateWindow(wndHandle);

    return std::make_tuple(CreateResult::OK, wndHandle);
}

int startAppLoop()
{
    MSG msg;
    std::memset(&msg, 0, sizeof(msg));
    while (msg.message != WM_QUIT){
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            //do something while idle
        }
    }
    return static_cast<int>(msg.wParam);
}

} //namespace wnd
