#include "pch.h"
#include "MeshRenderPass.h"
#include "SkyboxRenderPass.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 616; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct VortexInstance
{
    Vortex::Renderer* pRenderer;
};

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800

_Use_decl_annotations_
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, PWSTR /*pCmdLine*/, int nCmdShow)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    VortexInstance vxInstance;

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"Vortex Window Class";
    ::RegisterClassEx(&windowClass);

    // Create the window.
    HWND hWnd = ::CreateWindowEx(
        0,                              // Optional window styles.
        windowClass.lpszClassName,                     // Window class
        L"Vortex Engine",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        &vxInstance        // Additional application data
    );

    vxInstance.pRenderer = new Vortex::Renderer(hWnd);
    vxInstance.pRenderer->AddPass<Vortex::SkyboxRenderPass>();
    vxInstance.pRenderer->AddPass<Vortex::MeshRenderPass>();

    if (hWnd == NULL)
    {
        return 0;
    }

    ::ShowWindow(hWnd, nCmdShow);

    // Run the message loop.
    MSG msg = { };
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    delete vxInstance.pRenderer;

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    VortexInstance* vxInstance = reinterpret_cast<VortexInstance*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_CREATE:
    {
        DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
        ::DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));

        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        break;
    }
    case WM_PAINT:
    {
        if (vxInstance->pRenderer)
        {
            vxInstance->pRenderer->Execute();
        }
        break;
    }
    case WM_DESTROY:
    {
        ::PostQuitMessage(0);
        break;
    }
    default:
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}