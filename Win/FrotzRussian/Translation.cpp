/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Stub DLL entry point for Windows Frotz translation DLLs
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
  return 1;
}

extern "C" __declspec(dllexport) BOOL IsEnabled(VOID)
{
  return (GetACP() == 1251); // ANSI Cyrillic; Cyrillic (Windows)
}

extern "C" __declspec(dllexport) LPCSTR GetLink(UINT idx)
{
  switch (idx)
  {
  case 0:
    return "https://ifiction.ru";
  case 1:
    return "https://ifwiki.ru";
  case 2:
    return "https://ifhub.club";
  case 3:
    return "https://parserfest.ru";
  case 4:
    return "https://kril.ifiction.ru";
  case 5:
    return "https://rinform.org";
  default:
    return NULL;
  }
}
