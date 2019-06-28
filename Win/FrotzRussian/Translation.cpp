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
