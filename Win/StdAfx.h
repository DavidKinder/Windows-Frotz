/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Precompiled headers
/////////////////////////////////////////////////////////////////////////////

#pragma once
#define VC_EXTRALEAN
#define _AFX_ALL_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <afxmt.h>
#include <afxole.h>
#include <afxtempl.h>
#include <afxdlgs.h>
#include <afxhtml.h>
#include <afxpriv.h>

#include <atlbase.h>

#include <comutil.h>

#if (_WIN32_WINNT < 0x0600)

typedef enum _TASKDIALOG_COMMON_BUTTON_FLAGS
{
  TDCBF_OK_BUTTON =     0x0001,
  TDCBF_YES_BUTTON =    0x0002,
  TDCBF_NO_BUTTON =     0x0004,
  TDCBF_CANCEL_BUTTON = 0x0008,
  TDCBF_RETRY_BUTTON =  0x0016,
  TDCBF_CLOSE_BUTTON =  0x0032,
}
TASKDIALOG_COMMON_BUTTON_FLAGS;

enum _TASKDIALOG_FLAGS
{
  TDF_ENABLE_HYPERLINKS               = 0x0001,
  TDF_USE_HICON_MAIN                  = 0x0002,
  TDF_USE_HICON_FOOTER                = 0x0004,
  TDF_ALLOW_DIALOG_CANCELLATION       = 0x0008,
  TDF_USE_COMMAND_LINKS               = 0x0010,
  TDF_USE_COMMAND_LINKS_NO_ICON       = 0x0020,
  TDF_EXPAND_FOOTER_AREA              = 0x0040,
  TDF_EXPANDED_BY_DEFAULT             = 0x0080,
  TDF_VERIFICATION_FLAG_CHECKED       = 0x0100,
  TDF_SHOW_PROGRESS_BAR               = 0x0200,
  TDF_SHOW_MARQUEE_PROGRESS_BAR       = 0x0400,
  TDF_CALLBACK_TIMER                  = 0x0800,
  TDF_POSITION_RELATIVE_TO_WINDOW     = 0x1000,
  TDF_RTL_LAYOUT                      = 0x2000,
  TDF_NO_DEFAULT_RADIO_BUTTON         = 0x4000,
  TDF_CAN_BE_MINIMIZED                = 0x8000,
  TDF_NO_SET_FOREGROUND               = 0x00010000,
  TDF_SIZE_TO_CONTENT                 = 0x01000000
};

#define TD_INFORMATION_ICON MAKEINTRESOURCEW(-3)

#pragma pack(1)
typedef struct _TASKDIALOGCONFIG
{
  UINT        cbSize;
  HWND        hwndParent;
  HINSTANCE   hInstance;
  int         dwFlags;
  TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons;
  PCWSTR      pszWindowTitle;
  union
  {
    HICON   hMainIcon;
    PCWSTR  pszMainIcon;
  }
  DUMMYUNIONNAME;
  PCWSTR      pszMainInstruction;
  PCWSTR      pszContent;
  UINT        cButtons;
  const VOID *pButtons;
  int         nDefaultButton;
  UINT        cRadioButtons;
  const VOID *pRadioButtons;
  int         nDefaultRadioButton;
  PCWSTR      pszVerificationText;
  PCWSTR      pszExpandedInformation;
  PCWSTR      pszExpandedControlText;
  PCWSTR      pszCollapsedControlText;
  union
  {
    HICON   hFooterIcon;
    PCWSTR  pszFooterIcon;
  }
  DUMMYUNIONNAME2;
  PCWSTR      pszFooter;
  LPVOID      pfCallback;
  LONG_PTR    lpCallbackData;
  UINT        cxWidth;
}
TASKDIALOGCONFIG;
#pragma pack()

typedef enum _TASKDIALOG_NOTIFICATIONS
{
  TDN_CREATED                = 0,
  TDN_NAVIGATED              = 1,
  TDN_BUTTON_CLICKED         = 2,
  TDN_HYPERLINK_CLICKED      = 3,
  TDN_TIMER                  = 4,
  TDN_DESTROYED              = 5,
  TDN_RADIO_BUTTON_CLICKED   = 6,
  TDN_DIALOG_CONSTRUCTED     = 7,
  TDN_VERIFICATION_CLICKED   = 8,
  TDN_HELP                   = 9,
  TDN_EXPANDO_BUTTON_CLICKED = 10
}
TASKDIALOG_NOTIFICATIONS;

#endif // _WIN32_WINNT < 0x0600
