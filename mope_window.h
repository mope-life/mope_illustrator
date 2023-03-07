/*
    To use, #define MOPE_WINDOW_IMPL before the #include in exactly one of
    your source files
*/

#pragma once

#include <utility>
#include <unordered_map>
#include <string_view>
#include <bitset>
#include <iostream>

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


/**
* KeyType must be statically castable to a uint8_t
*
* WindowType requires the following public methods:
*
* // builds the window with this title and these dimensions, return true if successful
* bool build(std::string& name, int& width, int& height);
*
* // acquires an OpenGL context in the calling thread, returns true if successful
* bool acquireContext();
*
* // displays the graphics for one frame
* void swap();
*
* // sets the title bar of the window
* void setTitle(std::string&);
*
* // get the client (drawable) dimensions of the window
* int getWidth();
* int getHeight();
*
* // get the mouse deltas since last call and reset
* int retrieveXDelta();
* int retrieveYDelta();
*
* // get a bitet representing currently pressed keys
* // to be indexed using [static_cast<uint8_t>(KeyType)]
* std::bitset<256> getKeyStates();
* 
* // process any messages the window has to handle, return false if ready to quit
* bool processMessages();
*
* // force the window to start quitting
* void close();
*/

namespace
{
    std::wstring wfromcstr(const char* cstr, size_t len)
    {
        wchar_t* buf = new wchar_t[len + 1];
        size_t n_chars{ };
        mbstowcs_s(&n_chars, buf, len + 1, cstr, len);
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
    enum class Key : uint8_t {
        A = 'A', B = 'B', C = 'C', D = 'D', E = 'E', F = 'F', G = 'G', H = 'H', I = 'I',
        J = 'J', K = 'K', L = 'L', M = 'M', N = 'N', O = 'O', P = 'P', Q = 'Q', R = 'R',
        S = 'S', T = 'T', U = 'U', V = 'V', W = 'W', X = 'X', Y = 'Y', Z = 'Z',
        R0 = '0', R1 = '1', R2 = '2', R3 = '3', R4 = '4',
        R5 = '5', R6 = '6', R7 = '7', R8 = '8', R9 = '9',
        F1 = VK_F1, F2 = VK_F2, F3 = VK_F3, F4 = VK_F4, F5 = VK_F5, F6 = VK_F6,
        F7 = VK_F7, F8 = VK_F8, F9 = VK_F9, F10 = VK_F10, F11 = VK_F11, F12 = VK_F12,
        RETURN = VK_RETURN, BACK = VK_BACK, INS = VK_INSERT, DEL = VK_DELETE,
        CTRL = VK_CONTROL, ALT = VK_MENU, ESC = VK_ESCAPE, SHIFT = VK_SHIFT, CAPS = VK_CAPITAL,
        PGUP = VK_PRIOR, PGDN = VK_NEXT, END = VK_END, HOME = VK_HOME, PAUSE = VK_PAUSE,
        LEFT = VK_LEFT, RIGHT = VK_RIGHT, UP = VK_UP, DOWN = VK_DOWN,
        TAB = VK_TAB, SPACE = VK_SPACE,
        PLUS = VK_OEM_PLUS, MINUS = VK_OEM_MINUS, PERIOD = VK_OEM_PERIOD, COMMA = VK_OEM_COMMA,
        OEM_1 = VK_OEM_1, OEM_2 = VK_OEM_2, OEM_3 = VK_OEM_3, OEM_4 = VK_OEM_4,
        OEM_5 = VK_OEM_5, OEM_6 = VK_OEM_6, OEM_7 = VK_OEM_8,
    };

    class Window
    {
    public:
        ~Window();

        // Creates the window and returns true if successful
        bool build(LPCWSTR name, int w, int h, HINSTANCE hInstance = GetModuleHandle(NULL));
        bool build(std::string_view name, int w, int h, HINSTANCE hInstance = GetModuleHandle(NULL));

        // Display graphics
        void swap();

        // Process any messages in the queue, return false when it's time start quitting (on WM_CLOSE)
        bool processMessages();

        // Makes the given string the title of the window
        void setTitle(LPCWSTR title);
        void setTitle(std::string_view title);

        // get size of client area
        int getWidth();
        int getHeight();

        // get the mouse deltas since last call and reset
        int retrieveXDelta();
        int retrieveYDelta();

        // get a bitet representing currently pressed keys
        // to be indexed using [static_cast<uint8_t>(KeyType)]
        std::bitset<256> getKeyStates();

        // Start quitting
        void close();

        // get an OpenGL context in the calling thread
        bool acquireContext();

    private:
        LPCWSTR m_windowClass = L"mope";
        HWND m_hwnd = NULL;
        HGLRC m_hrc = NULL;
        HDC m_hdc = NULL;
        
        // Old cursor clip rectangle before the application gets focus
        // Should be restored when the application loses focus
        RECT m_oldClip{};

        // all set at first call to handleSize
        WINDOWINFO m_wi{ sizeof(WINDOWINFO) };
        RECT m_windowRect;
        RECT m_clientRect;

        int m_width;
        int m_height;
        int m_deltaX;
        int m_deltaY;
        std::bitset<256> m_keystates{ };

        ATOM registerClass(HINSTANCE hInstance);
        HWND createWindow(LPCWSTR, HINSTANCE, int w, int h);
        HGLRC createRenderingContext();
        void updateSize();

        // Callback for Windows event handling
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
    Window::~Window()
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(m_hrc);

        // now that the context is deleted, we can destroy the window
        DestroyWindow(m_hwnd);
    }

    bool Window::build(LPCWSTR lpWindowName, int w, int h, HINSTANCE hInstance)
    {
        bool built =
            (registerClass(hInstance) != 0)
            && (m_hwnd = createWindow(lpWindowName, hInstance, w, h)) != NULL
            && (m_hdc = GetDC(m_hwnd)) != NULL
            && (m_hrc = createRenderingContext()) != NULL;

        if (built) {
            ShowWindow(m_hwnd, SW_SHOWDEFAULT);
            ShowCursor(FALSE);
            updateSize();
        }

        return built;
    }

    bool Window::build(std::string_view name, int w, int h, HINSTANCE hInstance)
    {
        return build(wfromcstr(name.data(), name.length()).c_str(), w, h, hInstance);
    }

    bool Window::acquireContext()
    {
        return wglMakeCurrent(m_hdc, m_hrc) == TRUE;
    }

    void Window::swap()
    {
        SwapBuffers(m_hdc);
    }

    // Makes the given string the title of the window
    void Window::setTitle(LPCWSTR title)
    {
        SetWindowText(m_hwnd, title);
    }

    void Window::setTitle(std::string_view title)
    {
        setTitle(wfromcstr(title.data(), title.length()).c_str());
    }

    int Window::getWidth()
    {
        return m_width;
    }

    int Window::getHeight()
    {
        return m_height;
    }

    int Window::retrieveXDelta()
    {
        int result = m_deltaX;
        m_deltaX = 0;
        return result;
    }

    int Window::retrieveYDelta()
    {
        int result = m_deltaY;
        m_deltaY = 0;
        return result;
    }

    std::bitset<256> Window::getKeyStates()
    {
        return m_keystates;
    }

    void Window::close()
    {
        PostMessage(m_hwnd, WM_CLOSE, NULL, NULL);
    }


    /*========================================================================*\
    |  Window creation                                                         |
    \*========================================================================*/

    ATOM Window::registerClass(HINSTANCE hInstance)
    {
        WNDCLASSEX wcx{ }, gwc{ };
        if (GetClassInfoEx(hInstance, m_windowClass, &gwc))
        {
            // class already exists - we must have already registered it
            return TRUE;
        }

        wcx.cbSize = sizeof(WNDCLASSEX);
        wcx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wcx.lpfnWndProc = Window::windowProc;
        wcx.lpszClassName = m_windowClass;
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
            dwExStyle, m_windowClass, lpWindowName, dwStyle,
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
        GetWindowInfo(m_hwnd, &m_wi);
        GetWindowRect(m_hwnd, &m_windowRect);
        GetClientRect(m_hwnd, &m_clientRect);

        m_width = m_clientRect.right - m_clientRect.left;
        m_height = m_clientRect.bottom - m_clientRect.top;
    }

    /*========================================================================*\
    |  Message Loop and Callbacks                                              |
    \*========================================================================*/
    
    bool Window::processMessages()
    {
        MSG msg{ };

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_CLOSE)
            {
                return false;
            }
        }

        return true;
    }

    LRESULT CALLBACK Window::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Window* pThis = nullptr;

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
        case WM_KEYDOWN:        handleKeyDown(wParam);      break;  
        case WM_KEYUP:          handleKeyUp(wParam);        break;
        case WM_SIZE:           handleSize(lParam);         break;
        case WM_EXITSIZEMOVE:   handleExitSizeMove();       break;
        case WM_SETFOCUS:       handleSetFocus();           break;
        case WM_KILLFOCUS:      handleKillFocus();          break;
        case WM_CLOSE:		    handleClose();              break;

        default: return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    void Window::handleMouseMove(LPARAM lparam)
    {
        // Since we set cursor pos again in this method, we guard against infinite recursion
        static bool setFlag = false;
        if (setFlag)
        {
            setFlag = false;
        }
        else
        {
            setFlag = true;

            // relative to client area
            WORD xPos = LOWORD(lparam);
            WORD yPos = HIWORD(lparam);

            int clientMidpointX = (m_clientRect.right - m_clientRect.left) / 2;
            int clientMidpointY = (m_clientRect.bottom - m_clientRect.top) / 2;

            m_deltaX += xPos - clientMidpointX;
            m_deltaY += clientMidpointY - yPos;

            int clientLeft = m_wi.rcClient.left;
            int clientTop = m_wi.rcClient.top;

            // NOT relative to client area
            SetCursorPos(clientLeft + clientMidpointX, clientTop + clientMidpointY);
        }
    }

    void Window::handleKeyUp(WPARAM vk_code)
    {
        m_keystates.reset(vk_code);
    }

    void Window::handleKeyDown(WPARAM vk_code)
    {
        m_keystates.set(vk_code);
    }

    void Window::handleClose()
    {
        // nothing to do, handled in destructor
    }   

    void Window::handleSize(LPARAM lparam)
    {
        m_width = LOWORD(lparam);
        m_height = HIWORD(lparam);
    }

    void Window::handleExitSizeMove()
    {
        updateSize();
    }

    void Window::handleSetFocus()
    {
        updateSize();
        GetClipCursor(&m_oldClip);
        ClipCursor(&m_wi.rcClient);
    }

    void Window::handleKillFocus()
    {
        ClipCursor(&m_oldClip);
        m_keystates.reset();
    }
}
#endif //MOPE_WINDOW_IMPL