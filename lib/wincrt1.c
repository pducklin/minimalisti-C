//+---------------------------------------------------------------------------

// _UNICODE for tchar.h, UNICODE for API
#include <tchar.h>

#include <windows.h>
#include <stdlib.h>

#define __UNKNOWN_APP    0
#define __CONSOLE_APP    1
#define __GUI_APP        2
void __set_app_type(int);
void _controlfp(unsigned a, unsigned b);

#ifdef _UNICODE
#define __tgetmainargs __wgetmainargs
#define _twinstart _wwinstart
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
#else
#define __tgetmainargs __getmainargs
#define _twinstart _winstart
#endif

extern void tinyc_canary_init(void);

typedef struct { int newmode; } _startupinfo;
int __cdecl __tgetmainargs(int *pargc, _TCHAR ***pargv, _TCHAR ***penv, int globb, _startupinfo*);

static int go_winmain(TCHAR *arg1)
{
    STARTUPINFO si;
    _TCHAR *szCmd, *p;
    int fShow;

    GetStartupInfo(&si);
    if (si.dwFlags & STARTF_USESHOWWINDOW)
        fShow = si.wShowWindow;
    else
        fShow = SW_SHOWDEFAULT;

    szCmd = NULL, p = GetCommandLine();
    if (arg1)
        szCmd = _tcsstr(p, arg1);
    if (NULL == szCmd)
        szCmd = _tcsdup(__T(""));
    else if (szCmd > p && szCmd[-1] == __T('"'))
        --szCmd;
#if defined __i386__ || defined __x86_64__
    _controlfp(0x10000, 0x30000);
#endif
    return _tWinMain(GetModuleHandle(NULL), NULL, szCmd, fShow);
}

int _twinstart(void)
{
    tinyc_canary_init();
    __TRY__
    _startupinfo start_info_con = {0};
    __set_app_type(__GUI_APP);
    __tgetmainargs(&__argc, &__targv, &_tenviron, 0, &start_info_con);
    exit(go_winmain(__argc > 1 ? __targv[1] : NULL));
}
