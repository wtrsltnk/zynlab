// =============================================================================
//                                  INCLUDES
// =============================================================================
#include <Windows.h>
#include <examples/imgui_impl_win32.h>
#include <glad/glad.h>
#include <imgui.h>
#include <iostream>
#include <random>
#include <stdio.h>
#include <chrono>
#include <examples/imgui_impl_opengl3.h>
#include <iapplication.h>
#include <imgui_internal.h>
#include <wchar.h>

IApplication::~IApplication() = default;

HGLRC g_GLRenderContext;
HDC g_HDCDeviceContext;
HWND g_hwnd;
int g_display_w = 800;
int g_display_h = 600;
ImVec4 clear_color;

void CreateGlContext();

void SetCurrentContext();

LRESULT WINAPI WndProc(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

bool Init(
    HINSTANCE hInstance);

void Cleanup(
    HINSTANCE hInstance);

extern IApplication *CreateApplication();

static IApplication *app = nullptr;
static bool glReady = false;

int wmain(
    int argc,
    wchar_t *argv[])
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    app = CreateApplication();

    if (!Init(hInstance))
    {
        return 1;
    }

    if (!app->Setup())
    {
        Cleanup(
            hInstance);

        return 1;
    }

    // Main loop
    MSG msg;
    ZeroMemory(
        &msg,
        sizeof(msg));

    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    app->Cleanup();

    delete app;

    Cleanup(
        hInstance);

    return 0;
}

bool Init(
    HINSTANCE hInstance)
{
    WNDCLASS wc;

    ZeroMemory(
        &wc,
        sizeof(wc));

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"IMGUI";
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClass(&wc))
    {
        return false;
    }

    g_hwnd = CreateWindow(
        wc.lpszClassName,
        L"IMGUI-all-in-one",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hInstance,
        0);

    // Show the window
    ShowWindow(
        g_hwnd,
        SW_SHOWDEFAULT);

    UpdateWindow(
        g_hwnd);

    //Prepare OpenGlContext
    CreateGlContext();

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    //Init Win32
    ImGui_ImplWin32_Init(
        g_hwnd);

    //Init OpenGL Imgui Implementation
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(
        glsl_version);

    //Set Window bg color
    clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Setup style
    ImGui::StyleColorsDark();

    return true;
}

void Cleanup(
    HINSTANCE hInstance)
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();

    wglDeleteContext(
        g_GLRenderContext);

    ImGui::DestroyContext();

    ImGui_ImplWin32_Shutdown();

    DestroyWindow(g_hwnd);

    UnregisterClass(
        L"IMGUI",
        hInstance);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
        case WM_MOUSEMOVE:
        {
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }
        case WM_SIZE:
        {
            if (wParam != SIZE_MINIMIZED)
            {
                g_display_w = (UINT)LOWORD(lParam);
                g_display_h = (UINT)HIWORD(lParam);
            }
            return 0;
        }
        case WM_SYSCOMMAND:
        {
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            {
                return 0;
            }
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_PAINT:
        {
            if (!glReady)
            {
                return 0;
            }

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            app->Render2d();

            // Rendering
            ImGui::Render();

            wglMakeCurrent(
                g_HDCDeviceContext,
                g_GLRenderContext);

            glViewport(
                0,
                0,
                g_display_w,
                g_display_h);

            glClearColor(
                clear_color.x,
                clear_color.y,
                clear_color.z,
                clear_color.w);

            glClear(
                GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(
                ImGui::GetDrawData());

            SwapBuffers(
                g_HDCDeviceContext);

            ValidateRect(hWnd, NULL);

            return 0L;
        }
    }

    return DefWindowProc(
        hWnd,
        msg,
        wParam,
        lParam);
}

void CreateGlContext()
{
    PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, //Flags
            PFD_TYPE_RGBA,                                              // The kind of framebuffer. RGBA or palette.
            32,                                                         // Colordepth of the framebuffer.
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            24, // Number of bits for the depthbuffer
            8,  // Number of bits for the stencilbuffer
            0,  // Number of Aux buffers in the framebuffer.
            PFD_MAIN_PLANE,
            0,
            0, 0, 0};

    g_HDCDeviceContext = GetDC(
        g_hwnd);

    int pixelFormat = ChoosePixelFormat(
        g_HDCDeviceContext,
        &pfd);

    SetPixelFormat(
        g_HDCDeviceContext,
        pixelFormat,
        &pfd);

    g_GLRenderContext = wglCreateContext(
        g_HDCDeviceContext);

    wglMakeCurrent(
        g_HDCDeviceContext,
        g_GLRenderContext);

    gladLoadGL();

    glReady = true;

    InvalidateRect(g_hwnd, NULL, FALSE);
}

int IApplication::Width() const
{
    return g_display_w;
}

int IApplication::Height() const
{
    return g_display_h;
}

void IApplication::PostRedraw()
{
    InvalidateRect(g_hwnd, NULL, FALSE);
}
