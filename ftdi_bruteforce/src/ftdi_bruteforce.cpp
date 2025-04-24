// filepath: /ftdi_bruteforce/ftdi_bruteforce/src/ftdi_bruteforce.cpp
#include <windows.h>
#include <ftd2xx.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>

#define IDC_START_BTN 1001
#define IDC_PROGRESS  1002
#define WM_SHOW_SUCCESS (WM_APP + 10)

std::atomic<uint32_t> g_progress(0);
std::atomic<bool> g_running(false);
std::atomic<uint32_t> g_last_progress(0);
std::atomic<uint32_t> g_speed(0);
std::atomic<uint32_t> g_success_val(0);

// statistics
void speed_thread(HWND hwnd) {
    while (g_running) {
        uint32_t last = g_progress.load();
        Sleep(1000);
        uint32_t now = g_progress.load();
        g_speed = now - last;
        PostMessage(hwnd, WM_APP + 3, 0, 0);
    }
}

// start
void bruteforce_thread(HWND hwnd) {
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;
    DWORD bytesWritten;
    unsigned char buf[4];

    ftStatus = FT_Open(0, &ftHandle);
    if (ftStatus != FT_OK) {
        MessageBox(hwnd, L"FT_Open failed!", L"Error", MB_ICONERROR);
        g_running = false;
        return;
    }
    FT_SetBitMode(ftHandle, 0xFF, 0x40);

    std::thread(speed_thread, hwnd).detach();
    for (uint32_t val = 0; g_running && val <= 0xFFFFFFFF; ++val) {
        buf[0] = (val >> 0) & 0xFF;
        buf[1] = (val >> 8) & 0xFF;
        buf[2] = (val >> 16) & 0xFF;
        buf[3] = (val >> 24) & 0xFF;
        ftStatus = FT_Write(ftHandle, buf, 4, &bytesWritten);
        // 这里假设FT_Write失败即为破解成功（如有更精确判断请替换此条件）
        if (ftStatus != FT_OK || bytesWritten != 4) {
            g_success_val = val;
            PostMessage(hwnd, WM_SHOW_SUCCESS, 0, 0);
            std::wstringstream ss;
            ss << L"FT_Write failed at " << std::hex << val;
            MessageBox(hwnd, ss.str().c_str(), L"Error", MB_ICONERROR);
            break;
        }
        g_progress = val;
        if (val % 10000 == 0) {
            PostMessage(hwnd, WM_APP + 1, 0, 0);
        }
    }
    FT_Close(ftHandle);
    g_running = false;
    PostMessage(hwnd, WM_APP + 2, 0, 0);
}

// gui
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtn, hEdit;
    static HWND hSpeedEdit;
    static HWND hResultEdit;
    switch (msg) {
    case WM_CREATE:
        hBtn = CreateWindow(L"BUTTON", L"开始暴力破解", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 20, 120, 30, hwnd, (HMENU)IDC_START_BTN, GetModuleHandle(NULL), NULL);
        hEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | ES_READONLY,
            20, 60, 250, 25, hwnd, (HMENU)IDC_PROGRESS, GetModuleHandle(NULL), NULL);
        hSpeedEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | ES_READONLY,
            20, 95, 250, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hResultEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | ES_READONLY,
            20, 130, 250, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_START_BTN && !g_running) {
            g_running = true;
            g_progress = 0;
            SetWindowText(hEdit, L"进度: 0x00000000");
            std::thread(bruteforce_thread, hwnd).detach();
        }
        break;
    case WM_APP + 1: { // update
        wchar_t buf[64];
        swprintf_s(buf, L"进度: 0x%08X", g_progress.load());
        SetWindowText(hEdit, buf);
        break;
    }
    case WM_APP + 2: // done
        SetWindowText(hEdit, L"完成!");
        break;
    case WM_APP + 3: { // update
        wchar_t buf[64];
        swprintf_s(buf, L"速率: %u 次/秒", g_speed.load());
        SetWindowText(hSpeedEdit, buf);
        break;
    }
    case WM_SHOW_SUCCESS: {
        wchar_t buf[64];
        swprintf_s(buf, L"成功值: 0x%08X", g_success_val.load());
        SetWindowText(hResultEdit, buf);
        break;
    }
    case WM_CLOSE:
        g_running = false;
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// WinMain 入口
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"BruteForceUI";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, L"FTDI 暴力破解工具", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 210, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}