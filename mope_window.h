/*
    To use, #define MOPE_WINDOW_IMPL before the #include in exactly one of
    your source files
*/

#pragma once

#include <array>
#include <atomic>
#include <unordered_map>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <Windows.h>
#include <strsafe.h>


namespace
{
    std::wstring wfromcstr(const char* cstr)
    {
        size_t len = strlen(cstr);
        wchar_t* buf = new wchar_t[len + 1];
        size_t returnValue{};
        mbstowcs_s(&returnValue, buf, len + 1, cstr, len);
        std::wstring wstr(buf);
        delete[] buf;
        return wstr;
    }
}

/*============================================================================*\
|  Declarations                                                                |
\*============================================================================*/

namespace mope
{
    enum class Key {
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        R0, R1, R2, R3, R4, R5, R6, R7, R8, R9,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        RETURN, BACK, INS, DEL, CTRL, ALT, ESC, SHIFT, CAPS,
        PGUP, PGDN, END, HOME, PAUSE, LEFT, RIGHT, UP, DOWN,
        TAB, SPACE, PLUS, MINUS, PERIOD, COMMA,
        OEM_1, OEM_2, OEM_3, OEM_4, OEM_5, OEM_6, OEM_7,

        // opportunities for special queries
        AlphaNum, Any, None
    };

    class Window
    {
    public:
        Window(LPCWSTR name, int w, int h, HINSTANCE hInstance = GetModuleHandleA(NULL));
        Window(LPCSTR name, int w, int h, HINSTANCE hInstance = GetModuleHandleA(NULL));

        // Must be called on the rendering thread before making any OpenGL calls
        bool GetRenderingContext();

        void MessageLoop();
        void Swap();
        void Destroy();
        void ErrorExit(LPCWSTR lpszFunction);

        // Returns true only after the user has clicked the big red X
        bool WantsToClose();

        // Callback for Windows event handling
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // Makes the given string the title of the window
        void SetTitle(LPCWSTR title);
        void SetTitle(LPCSTR title);

        void SetDimensions(int w, int h);
        void GetDimensions(int* o_w, int* o_h);
        float AspectRatio();

        std::array<uint64_t, 2> GetKeyStates();
        int GetXDelta();
        int GetYDelta();

    private:
        LPCWSTR windowClass = L"mope";
        HWND m_hwnd = NULL;
        HGLRC m_hrc = NULL;
        HDC m_hdc = NULL;
        
        // Old cursor clip rectangle before the application gets focus
        // Should be restored when the application loses focus
        RECT m_oldClip{};

        // all set at first call to handleSize
        WINDOWINFO wi{ sizeof(WINDOWINFO) };
        RECT windowRect;
        RECT clientRect;

        // Window width and height reporting
        std::atomic<int> clientWidth{};
        std::atomic<int> clientHeight{};

        // Input reporting
        std::atomic<int> xDelta{};
        std::atomic<int> yDelta{};
        std::atomic<uint64_t> keystates[2];
        
        static std::unordered_map<unsigned int, uint8_t> keymap;

        bool bClosing = false;
        bool bRenderingContextCreated = false;

        //void buildKeymap();
        void updateSize();

        ATOM registerClass(HINSTANCE hInstance);
        HWND createWindow(LPCWSTR, HINSTANCE, int w, int h);
        HGLRC createRenderingContext();

        // Most Windows event handling is delegated to this...
        LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

        // ... which then forwards events to the handlers
        void handleMouseMove(LPARAM lparam);
        void handleClose();
        void handleSize(LPARAM lparam);
        void handleExitSizeMove();
        void handleSetFocus();
        void handleKillFocus();
        void handleKeyUp(WPARAM vk_code);
        void handleKeyDown(WPARAM vk_code);
    };
}


#ifdef MOPE_WINDOW_IMPL
#undef MOPE_WINDOW_IMPL

/*============================================================================*\
|  Implementation                                                              |
\*============================================================================*/

namespace mope
{
    std::unordered_map<unsigned int, uint8_t> Window::keymap{
        // Alphanumeric:
        {0x41u, (uint8_t)Key::A},   {0x42u, (uint8_t)Key::B},   {0x43u, (uint8_t)Key::C},
        {0x44u, (uint8_t)Key::D},   {0x45u, (uint8_t)Key::E},   {0x46u, (uint8_t)Key::F},
        {0x47u, (uint8_t)Key::G},   {0x48u, (uint8_t)Key::H},   {0x49u, (uint8_t)Key::I},
        {0x4Au, (uint8_t)Key::J},   {0x4Bu, (uint8_t)Key::K},   {0x4Cu, (uint8_t)Key::L},
        {0x4Du, (uint8_t)Key::M},   {0x4Eu, (uint8_t)Key::N},   {0x4Fu, (uint8_t)Key::O},
        {0x50u, (uint8_t)Key::P},   {0x51u, (uint8_t)Key::Q},   {0x52u, (uint8_t)Key::R},
        {0x53u, (uint8_t)Key::S},   {0x54u, (uint8_t)Key::T},   {0x55u, (uint8_t)Key::U},
        {0x56u, (uint8_t)Key::V},   {0x57u, (uint8_t)Key::W},   {0x58u, (uint8_t)Key::X},
        {0x59u, (uint8_t)Key::Y},   {0x5Au, (uint8_t)Key::Z},
        {0x30u, (uint8_t)Key::R0},  {0x31u, (uint8_t)Key::R1},  {0x32u, (uint8_t)Key::R2},
        {0x33u, (uint8_t)Key::R3},  {0x34u, (uint8_t)Key::R4},  {0x35u, (uint8_t)Key::R5},
        {0x36u, (uint8_t)Key::R6},  {0x37u, (uint8_t)Key::R7},  {0x38u, (uint8_t)Key::R8},
        {0x39u, (uint8_t)Key::R9},
        // Function Keys:
        {0x70u, (uint8_t)Key::F1},  {0x71u, (uint8_t)Key::F2},  {0x72u, (uint8_t)Key::F3},
        {0x73u, (uint8_t)Key::F4},  {0x74u, (uint8_t)Key::F5},  {0x75u, (uint8_t)Key::F6},
        {0x76u, (uint8_t)Key::F7},  {0x77u, (uint8_t)Key::F8},  {0x78u, (uint8_t)Key::F9},
        {0x79u, (uint8_t)Key::F10}, {0x7Au, (uint8_t)Key::F11}, {0x7Bu, (uint8_t)Key::F12},
        // Other:
        {0x1Bu, (uint8_t)Key::ESC},     {0x0Du, (uint8_t)Key::RETURN},
        {0x2Du, (uint8_t)Key::INS},     {0x13u, (uint8_t)Key::PAUSE},
        {0x2Eu, (uint8_t)Key::DEL},     {0x08u, (uint8_t)Key::BACK},
        {0x10u, (uint8_t)Key::SHIFT},   {0x14u, (uint8_t)Key::CAPS},
        {0x11u, (uint8_t)Key::CTRL},    {0x12u, (uint8_t)Key::ALT},
        {0x23u, (uint8_t)Key::END},     {0x24u, (uint8_t)Key::HOME},
        {0x21u, (uint8_t)Key::PGUP},    {0x22u, (uint8_t)Key::PGDN},
        {0x25u, (uint8_t)Key::LEFT},    {0x27u, (uint8_t)Key::RIGHT},
        {0x26u, (uint8_t)Key::UP},      {0x28u, (uint8_t)Key::DOWN},
        {0x09u, (uint8_t)Key::TAB},     {0x20u, (uint8_t)Key::SPACE},
        {0xBBu, (uint8_t)Key::PLUS},    {0xBDu, (uint8_t)Key::MINUS},
        {0xBCu, (uint8_t)Key::COMMA},   {0xBEu, (uint8_t)Key::PERIOD},
        // OEM specific q.v. https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
        {0xBAu, (uint8_t)Key::OEM_1},   {0xBFu, (uint8_t)Key::OEM_2},
        {0xC0u, (uint8_t)Key::OEM_3},   {0xDBu, (uint8_t)Key::OEM_4},
        {0xDCu, (uint8_t)Key::OEM_5},   {0xDDu, (uint8_t)Key::OEM_6},
        {0xDEu, (uint8_t)Key::OEM_7}
    };

    /*========================================================================*\
    |  API                                                                     |
    \*========================================================================*/

    Window::Window(LPCWSTR lpWindowName, int w, int h, HINSTANCE hInstance)
    {
        // buildKeymap();
        bool success =
            // class atom != 0
            (registerClass(hInstance) != 0)
            // m_hwnd != NULL
            && (m_hwnd = createWindow(lpWindowName, hInstance, w, h)) != NULL
            // m_hdc != NULL
            && (m_hdc = GetDC(m_hwnd)) != NULL
            // m_hrc != NULL
            && (m_hrc = createRenderingContext()) != NULL;

        if (success) {
            ShowWindow(m_hwnd, SW_SHOWDEFAULT);
            ShowCursor(FALSE);
            updateSize();
        }
        else {
            ErrorExit(L"Window construction");
        }
    }

    Window::Window(LPCSTR name, int w, int h, HINSTANCE hInstance)
        : Window(wfromcstr(name).c_str(), w, h, hInstance)
    {}

    bool Window::GetRenderingContext()
    {
        return wglMakeCurrent(m_hdc, m_hrc) == TRUE;
    }

    void Window::Swap()
    {
        SwapBuffers(m_hdc);
    }

    void Window::Destroy()
    {
        wglMakeCurrent(NULL, NULL);
        ReleaseDC(m_hwnd, m_hdc);
        wglDeleteContext(m_hrc);
        PostMessage(m_hwnd, WM_DESTROY, NULL, NULL);
    }

    bool Window::WantsToClose()
    {
        return bClosing;
    }

    void Window::SetTitle(LPCSTR title)
    {
        SetTitle(wfromcstr(title).c_str());
    }

    void Window::SetTitle(LPCWSTR title)
    {
        SetWindowText(m_hwnd, title);
    }


    void Window::SetDimensions(int w, int h)
    {
        RECT rect = { 0, 0, w, h };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    }

    void Window::GetDimensions(int* o_w, int* o_h)
    {
        *o_w = clientWidth.load();
        *o_h = clientHeight.load();
    }

    float Window::AspectRatio()
    {
        int w, h;
        GetDimensions(&w, &h);
        return static_cast<float>(w) / h;
    }

    std::array<uint64_t, 2> Window::GetKeyStates()
    {
        std::array<uint64_t, 2> returnstates{};
        returnstates[0] = keystates[0].load();
        returnstates[1] = keystates[1].load();
        return returnstates;
    }

    int Window::GetXDelta()
    {

        return xDelta.exchange(0);
    }

    int Window::GetYDelta()
    {
        return yDelta.exchange(0);
    }

    // Copied directly from
    // https://learn.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
    void Window::ErrorExit(LPCWSTR lpszFunction)
    {
        // Retrieve the system error message for the last-error code

        LPVOID lpMsgBuf;
        LPVOID lpDisplayBuf;
        DWORD dw = GetLastError();

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0, NULL);

        // Display the error message and exit the process

        lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
            (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen(lpszFunction) + 40) * sizeof(TCHAR));
        StringCchPrintf((LPTSTR)lpDisplayBuf,
            LocalSize(lpDisplayBuf) / sizeof(TCHAR),
            TEXT("%s failed with error %d: %s"),
            lpszFunction, dw, lpMsgBuf);
        MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

        LocalFree(lpMsgBuf);
        LocalFree(lpDisplayBuf);
        ExitProcess(dw);
    }

    /*========================================================================*\
    |  Window creation                                                         |
    \*========================================================================*/

    ATOM Window::registerClass(HINSTANCE hInstance)
    {
        WNDCLASSEX wcx{}, gwc{};
        if (GetClassInfoEx(hInstance, windowClass, &gwc))
        {
            UnregisterClass(windowClass, hInstance);
        }

        wcx.cbSize = sizeof(WNDCLASSEX);
        wcx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wcx.lpfnWndProc = Window::WindowProc;
        wcx.lpszClassName = windowClass;
        wcx.hbrBackground = NULL;

        return RegisterClassEx(&wcx);
    }

    HWND Window::createWindow(LPCWSTR lpWindowName, HINSTANCE hInstance, int w, int h)
    {
        // Use default width/height if args are 0
        int width = w > 0 ? w : CW_USEDEFAULT;
        int height = h > 0 ? h : CW_USEDEFAULT;

        // Default window settings
        DWORD dwStyle = WS_OVERLAPPEDWINDOW;
        DWORD dwExStyle = 0;
        int x = CW_USEDEFAULT;
        int y = CW_USEDEFAULT;
        HWND hWndParent = NULL;
        HMENU hMenu = NULL;

        return CreateWindowEx(
            dwExStyle, windowClass, lpWindowName, dwStyle,
            x, y, width, height, hWndParent, hMenu, hInstance, this
        );
    }

    HGLRC Window::createRenderingContext()
    {
        PIXELFORMATDESCRIPTOR pfd = { 0 };
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 16;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int pf = ChoosePixelFormat(m_hdc, &pfd);
        SetPixelFormat(m_hdc, pf, &pfd);

        return wglCreateContext(m_hdc);
    }

    void Window::updateSize()
    {
        GetWindowInfo(m_hwnd, &wi);
        GetWindowRect(m_hwnd, &windowRect);
        GetClientRect(m_hwnd, &clientRect);
        clientWidth = clientRect.right - clientRect.left;
        clientHeight = clientRect.bottom - clientRect.top;
    }

    /*========================================================================*\
    |  Message Loop and Callbacks                                              |
    \*========================================================================*/
    
    void Window::MessageLoop()
    {
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Window* pThis = NULL;
        // On window creation, save a pointer to the window structure
        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (Window*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hwnd = hwnd;
        }
        // Retrieve said pointer on subsequent events
        else {
            pThis = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis) {
            return pThis->handleMessage(uMsg, wParam, lParam);
        }
        else {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    LRESULT Window::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_MOUSEMOVE:      handleMouseMove(lParam);    break;
        case WM_KEYDOWN:        handleKeyDown(wParam);              break;  
        case WM_KEYUP:          handleKeyUp(wParam);                break;
        case WM_SIZE:           handleSize(lParam);                 break;
        case WM_EXITSIZEMOVE:   handleExitSizeMove();               break;
        case WM_SETFOCUS:       handleSetFocus();                   break;
        case WM_KILLFOCUS:      handleKillFocus();                  break;
        case WM_CLOSE:		    handleClose();                      break;
        case WM_DESTROY:	    PostQuitMessage(0);                 break;

        default: return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    void Window::handleMouseMove(LPARAM lparam)
    {
        // Since we set cursor pos again in this method, we guard against infinite recursion
        static bool setFlag = false;
        if (setFlag) {
            setFlag = false;
        }
        else {
            setFlag = true;

            // relative to client area
            WORD xPos = LOWORD(lparam);
            WORD yPos = HIWORD(lparam);

            int clientMidpointX = (clientRect.right - clientRect.left) / 2;
            int clientMidpointY = (clientRect.bottom - clientRect.top) / 2;

            xDelta += xPos - clientMidpointX;
            // up is down, the sky is falling
            yDelta += clientMidpointY - yPos;

            int clientLeft = wi.rcClient.left;
            int clientTop = wi.rcClient.top;

            // NOT relative to client area
            SetCursorPos(clientLeft + clientMidpointX, clientTop + clientMidpointY);
        }
    }

    void Window::handleKeyUp(WPARAM vk_code)
    {
        uint8_t keyId = keymap[(unsigned int)vk_code];
        size_t idx = 0;
        if (keyId & 64) { // this works because keyId < 128
            idx++;
            keyId ^= 64;
        }
        keystates[idx] &= ~(1ui64 << keyId);
    }

    void Window::handleKeyDown(WPARAM vk_code)
    {
        uint8_t keyId = keymap[(unsigned int)vk_code];
        size_t idx = 0;
        if (keyId & 64) { // this works because keyId < 128
            idx++;
            keyId ^= 64;
        }
        keystates[idx] |= 1ui64 << keyId;
    }

    void Window::handleClose()
    {
        bClosing = true;
    }   

    void Window::handleSize(LPARAM lparam)
    {
        clientWidth = LOWORD(lparam);
        clientHeight = HIWORD(lparam);
    }

    void Window::handleExitSizeMove()
    {
        updateSize();
    }

    void Window::handleSetFocus()
    {
        updateSize();
        GetClipCursor(&m_oldClip);
        ClipCursor(&wi.rcClient);
    }

    void Window::handleKillFocus()
    {
        keystates[0].store(0);
        keystates[1].store(0);
        ClipCursor(&m_oldClip);
    }
}
#endif //MOPE_WINDOW_IMPL