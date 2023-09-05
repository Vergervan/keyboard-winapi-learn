
#include <QCoreApplication>
#include <windows.h>
#include <set>
#include <iostream>

LRESULT CALLBACK keyboardProc(int Code, WPARAM wParam, LPARAM lParam)
{
    Q_UNUSED(Code)

    static std::set<DWORD> keysDown;

    PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;

    if(p != nullptr)
    {
        switch(wParam)
        {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            switch(p->vkCode)
            {
            case VK_LCONTROL: case VK_RCONTROL:
            case VK_LSHIFT:   case VK_RSHIFT:
            case VK_LMENU:    case VK_RMENU: // alt
                keysDown.insert(p->vkCode);
                break;
            case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
            case VK_PRIOR: case VK_NEXT: // page up and page down
            case VK_END: case VK_HOME:
            case VK_INSERT: case VK_DELETE:
            case VK_DIVIDE: // numpad slash
            case VK_NUMLOCK:
            case VK_SNAPSHOT: // print screen
            case VK_LWIN: case VK_RWIN:
            case VK_APPS:
                p->scanCode |= 0x100; // set extended bit
            default:
                HKL layout = GetKeyboardLayout(0);
                qDebug() << "Keyboard: " << layout;
                //qDebug() << "Virtual-key code: " << p->vkCode;
                //qDebug() << "Scan code: " << p->scanCode;
                wchar_t keyName[50];
                int cnt = GetKeyNameTextW(p->scanCode << 16, keyName, sizeof(keyName));
                char name[cnt+1];
                for(int i = 0; i < cnt; i++)
                    name[i] = keyName[i];
                name[cnt] = '\0';
                std::string str = name;
                qDebug() << "Key name: " << str << " | " << cnt;

                BYTE keyboardState[256];
                //keyboardState[VK_SHIFT] = 0xff;
                //keyboardState[VK_CONTROL] = 0xff;
                //keyboardState[VK_MENU] = 0xff;
                wchar_t keyCharacterBuffer[10] = { 0 };
                int error = ToUnicodeEx(p->vkCode, p->scanCode, keyboardState, keyCharacterBuffer, 10, 0, layout);
                std::cout << "Error: " << error << std::endl;
                if(error > 0){
                    std::wcout << keyCharacterBuffer << std::endl;
                }

                keysDown.insert(p->vkCode);
                break;
            }
            break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
            if(keysDown.size() > 0)
            {
                //qDebug() << "Keys down: " << keysDown.size();
                keysDown.clear();
            }
            break;
        }
    }
    return CallNextHookEx(NULL, Code, wParam, lParam);
}

void listenKeyboard()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, hInstance, 0);
    if(keyboardHook == NULL)
    {
        qWarning() << "Keyboard Hook failed";
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printf("Start\n");

    listenKeyboard();

    //UnhookWindowsHookEx(keyboardHook);

    return a.exec();
}
