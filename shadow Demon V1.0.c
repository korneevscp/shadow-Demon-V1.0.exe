#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <tlhelp32.h>
#include <stdio.h>

#pragma comment(lib, "comctl32.lib")

#define IDC_PROCESS   101
#define IDC_DLLPATH   102
#define IDC_BROWSE    103
#define IDC_INJECT    104
#define IDC_STATUS    105

DWORD GetProcessIdByName(const char *processName) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return 0;
}

BOOL InjectDLL(DWORD pid, const char *dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return FALSE;

    size_t dllPathLen = strlen(dllPath) + 1;
    LPVOID pRemotePath = VirtualAllocEx(hProcess, NULL, dllPathLen, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemotePath) {
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!WriteProcessMemory(hProcess, pRemotePath, dllPath, dllPathLen, NULL)) {
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
                                        (LPTHREAD_START_ROUTINE)LoadLibraryA,
                                        pRemotePath, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return TRUE;
}

HWND hProcessEdit, hDllEdit, hStatus;


void SetStatus(HWND hwnd, const char *msg) {
    SetWindowTextA(hStatus, msg);
}

void ApplyBlackCyanStyle(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    SetBkColor(hdc, RGB(0,0,0));
    SetTextColor(hdc, RGB(0,255,255));
    ReleaseDC(hwnd, hdc);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

        CreateWindowA("STATIC", "Process name:", WS_VISIBLE | WS_CHILD,
                      10, 10, 100, 20, hwnd, NULL, NULL, NULL);
        hProcessEdit = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
                                     120, 10, 200, 20, hwnd, (HMENU)IDC_PROCESS, NULL, NULL);
        SendMessageA(hProcessEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        CreateWindowA("STATIC", "DLL path:", WS_VISIBLE | WS_CHILD,
                      10, 40, 100, 20, hwnd, NULL, NULL, NULL);
        hDllEdit = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
                                 120, 40, 200, 20, hwnd, (HMENU)IDC_DLLPATH, NULL, NULL);
        SendMessageA(hDllEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        CreateWindowA("BUTTON", "Browse", WS_VISIBLE | WS_CHILD,
                      330, 40, 80, 20, hwnd, (HMENU)IDC_BROWSE, NULL, NULL);

        CreateWindowA("BUTTON", "Inject", WS_VISIBLE | WS_CHILD,
                      120, 80, 100, 30, hwnd, (HMENU)IDC_INJECT, NULL, NULL);

        hStatus = CreateWindowA("STATIC", "Status: waiting", WS_VISIBLE | WS_CHILD,
                                10, 130, 400, 20, hwnd, (HMENU)IDC_STATUS, NULL, NULL);
        SendMessageA(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

        ApplyBlackCyanStyle(hProcessEdit);
        ApplyBlackCyanStyle(hDllEdit);
        ApplyBlackCyanStyle(hStatus);

        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == IDC_BROWSE) {
            char fileName[MAX_PATH] = "";
            OPENFILENAMEA ofn = {0};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = "DLL Files\0*.dll\0All Files\0*.*\0";
            ofn.lpstrFile = fileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            if (GetOpenFileNameA(&ofn)) {
                SetWindowTextA(hDllEdit, fileName);
            }
        }
        if (LOWORD(wParam) == IDC_INJECT) {
            char processName[260];
            char dllPath[MAX_PATH];
            GetWindowTextA(hProcessEdit, processName, sizeof(processName));
            GetWindowTextA(hDllEdit, dllPath, sizeof(dllPath));

            DWORD pid = GetProcessIdByName(processName);
            if (!pid) {
                SetStatus(hwnd, "Process not found.");
                break;
            }

            if (InjectDLL(pid, dllPath)) {
                SetStatus(hwnd, "Injection success.");
            } else {
                SetStatus(hwnd, "Injection failed.");
            }
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(0,255,255));
        SetBkColor(hdc, RGB(0,0,0));
        return (INT_PTR)GetStockObject(NULL_BRUSH);
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEXA wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(0,0,0));
    wcex.lpszClassName = "InjectorGUI";


    wcex.hIcon = (HICON)LoadImageA(NULL, "C:\\Users\\vladi\\Downloads\\cheat\\555.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wcex.hIconSm = (HICON)LoadImageA(NULL, "C:\\Users\\vladi\\Downloads\\cheat\\555.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

    RegisterClassExA(&wcex);

    HWND hwnd = CreateWindowA(
        "InjectorGUI",
        "korneevscp-INJ",
        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 200,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
