//
// Created by RenAhsAcme on 2/9/2026.
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unordered_set>
#include <string>

static HHOOK g_hook = nullptr;
static std::unordered_set<DWORD> g_pressedKeys;

/**
 * 启动 QQ
 */
void LaunchQQ()
{
    const wchar_t* qqPath = L"D:\\Program Files\\Tencent\\QQNT\\QQ.exe";

    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    CreateProcessW(
        qqPath,
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_PROCESS_GROUP,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread)  CloseHandle(pi.hThread);
}

/**
 * 判断是否触发 Copilot 组合键
 */
bool IsCopilotCombo()
{
    return g_pressedKeys.contains(VK_LWIN) &&
           g_pressedKeys.contains(VK_LSHIFT) &&
           g_pressedKeys.contains(VK_F23);
}

/**
 * 低级键盘钩子
 */
LRESULT CALLBACK LowLevelKeyboardProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
)
{
    if (nCode == HC_ACTION)
    {
        auto* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        const bool isKeyDown =
            (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        const bool isKeyUp =
            (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

        if (isKeyDown)
        {
            g_pressedKeys.insert(kb->vkCode);

            if (IsCopilotCombo())
            {
                LaunchQQ();

                // 吞掉这个组合键，系统不会再收到
                return 1;
            }
        }
        else if (isKeyUp)
        {
            g_pressedKeys.erase(kb->vkCode);
        }
    }

    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int
)
{
    g_hook = SetWindowsHookExW(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        hInstance,
        0
    );

    if (!g_hook)
    {
        MessageBoxW(nullptr, L"无法安装键盘钩子", L"Error", MB_ICONERROR);
        return 1;
    }

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(g_hook);
    return 0;
}
