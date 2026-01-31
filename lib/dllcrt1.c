//+---------------------------------------------------------------------------

#include <windows.h>

BOOL WINAPI DllMain (HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved);

BOOL WINAPI _dllstart(HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved)
{
   tinyc_canary_init();
   BOOL bRet;
   bRet = DllMain (hDll, dwReason, lpReserved);
   return bRet;
}
