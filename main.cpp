#include <tuple>
#include <array>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11_1.h>

namespace wnd {

using Coords = RECT;

enum class CreateResult{
    OK = 0,
    RegisterClassFailed,
    CreateInstanceFailed
};

constexpr ATOM WINAPI_FAILED = 0;
static const wchar_t g_className[] = L"DX Windowd Class";

std::tuple<CreateResult, HWND> createWindow(HINSTANCE hInstance, int cmdShowFlags, LPCWSTR title, const Coords & coords);
int startAppLoop();

template<typename T>
void showMessageError(T errorVal)
{
    MessageBox(nullptr, toErrorStr(errorVal) , L"error!", MB_ICONERROR);
}

} //namespace wnd


namespace dx {

enum class CreateResult{
    OK = 0,
    CreateDeviceAndSwapChainFailed,
    BackBufferError
};

struct DXInfo{
    D3D_DRIVER_TYPE driverType;
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Device * devicePtr;
    ID3D11DeviceContext * immediateContexPtr;
    IDXGISwapChain * swapChainPtr;
    ID3D11RenderTargetView * renderTargetPtr;
    D3D11_VIEWPORT viewPort;
};

std::tuple<CreateResult, DXInfo> initD3D(HWND hWnd);

} //namspace dx

//-------------main--func------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR cmdStr, int cmdShowFlags)
{
    wnd::Coords wndCoords {0, 0, 800, 600};
    auto [wndCreateResult, wndHandle] = wnd::createWindow(hInstance, cmdShowFlags, L"DX11 Hello!", wndCoords);
    if (wndCreateResult != wnd::CreateResult::OK){
        wnd::showMessageError(wndCreateResult);
        return 1;
    }

    auto [d3dCreateResult, dxInfo] = dx::initD3D(wndHandle);
    if (d3dCreateResult != dx::CreateResult::OK){
        wnd::showMessageError(d3dCreateResult);
        return 1;
    }

    return wnd::startAppLoop();
}
//-----------------------------------------------------------------------------------------------------------

namespace wnd {

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

static LRESULT CALLBACK wndCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

std::tuple<CreateResult, HWND> createWindow(HINSTANCE hInstance, int cmdShowFlags, LPCWSTR title, const Coords & coords)
{
    WNDCLASSEX wndClass;
    memset(&wndClass, 0, sizeof(wndClass));

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

    if (RegisterClassEx(&wndClass) == WINAPI_FAILED){
        //auto regError = GetLastError();
        return std::make_tuple(CreateResult::RegisterClassFailed, nullptr);
    }



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


namespace dx {

inline static LPCWSTR toErrorStr(CreateResult val)
{
    switch (val) {
    case CreateResult::OK:
        return L"OK";
    case CreateResult::BackBufferError:
        return L"D3D backBuffer error!";
    case CreateResult::CreateDeviceAndSwapChainFailed:
        return L"Failed creation D3D device and swapchain!";
    }
    return L"Unknown error!";
}

std::tuple<CreateResult, DXInfo> initD3D(HWND hWnd)
{
    DXInfo resultInfo;
    HRESULT d3d11Result = E_UNEXPECTED;
    memset(&resultInfo, 0, sizeof(resultInfo));

    RECT wndCoords;
    GetClientRect(hWnd, &wndCoords);

    resultInfo.viewPort.Width = static_cast<float>(wndCoords.right - wndCoords.left);
    resultInfo.viewPort.Height = static_cast<float>(wndCoords.bottom - wndCoords.top);
    resultInfo.viewPort.MinDepth = 0.0f;
    resultInfo.viewPort.MaxDepth = 1.0f;
    resultInfo.viewPort.TopLeftX = 0.0f;
    resultInfo.viewPort.TopLeftY = 0.0f;

    std::array<D3D_DRIVER_TYPE, 1> d3dDriverTypes {
        D3D_DRIVER_TYPE_HARDWARE
    };
    std::array<D3D_FEATURE_LEVEL, 4> d3dFeatureLevel {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    memset(&swapChainDesc, 0, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = wndCoords.right - wndCoords.left;
    swapChainDesc.BufferDesc.Height = wndCoords.bottom - wndCoords.top;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    UINT deviceFlags = 0;

    for (const auto & driverType : d3dDriverTypes){
        resultInfo.driverType = driverType;
        d3d11Result = D3D11CreateDeviceAndSwapChain(/*adapter*/nullptr, driverType, /*software*/nullptr, deviceFlags,
                                                          d3dFeatureLevel.data(), d3dFeatureLevel.size(), D3D11_SDK_VERSION,
                                                          &swapChainDesc, &resultInfo.swapChainPtr, &resultInfo.devicePtr,
                                                          &resultInfo.featureLevel, &resultInfo.immediateContexPtr);
        if (d3d11Result == E_INVALIDARG) // DX11.0 don't recognize 11_1 feature lvl and return error
            d3d11Result = D3D11CreateDeviceAndSwapChain(/*adapter*/nullptr, driverType, /*software*/nullptr, deviceFlags,
                                                         &d3dFeatureLevel[1], (d3dFeatureLevel.size() - 1), D3D11_SDK_VERSION,
                                                         &swapChainDesc, &resultInfo.swapChainPtr, &resultInfo.devicePtr,
                                                         &resultInfo.featureLevel, &resultInfo.immediateContexPtr);
        if (SUCCEEDED(d3d11Result))
            break;
    }
    if (FAILED(d3d11Result))
        return std::make_tuple(CreateResult::CreateDeviceAndSwapChainFailed, resultInfo);

    ID3D11Texture2D * backBufferPtr = nullptr;
    auto backBufferResult = resultInfo.swapChainPtr->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&backBufferPtr));
    if ( FAILED(backBufferResult) || (backBufferPtr == nullptr) )
        return std::make_tuple(CreateResult::BackBufferError, resultInfo);

    auto rtResult = resultInfo.devicePtr->CreateRenderTargetView(backBufferPtr, nullptr, &resultInfo.renderTargetPtr);
    if ( FAILED(rtResult) || (resultInfo.renderTargetPtr == nullptr) )
        return std::make_tuple(CreateResult::BackBufferError, resultInfo);

    resultInfo.immediateContexPtr->OMSetRenderTargets(1, &resultInfo.renderTargetPtr, nullptr);
    resultInfo.immediateContexPtr->RSSetViewports(1, &resultInfo.viewPort);
    std::array<float, 4> clearColor {0.3f, 0.5f, 0.6f, 0.0f};
    resultInfo.immediateContexPtr->ClearRenderTargetView(resultInfo.renderTargetPtr, clearColor.data());
    resultInfo.swapChainPtr->Present(0, 0);

    return std::make_tuple(CreateResult::OK, resultInfo);
}

} //namespace dx
