/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz frame window class
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FrotzApp.h"
#include "FrotzWnd.h"
#include "FrotzFrameWnd.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT Indicators[] =
{
  ID_SEPARATOR,
  ID_INDICATOR_ZCODE,
  ID_INDICATOR_TIME,
  ID_INDICATOR_CAPS,
  ID_INDICATOR_NUM
};

bool FilterHotkeys(char c)
{
  return (strchr("DHNPRSUX",c) == NULL);
}

FrotzFrameWnd::FrotzFrameWnd() : m_clientWnd(NULL), m_codePage(CP_ACP), m_dpi(96)
{
  m_menuBar.SetUseF10(false);
  m_menuBar.SetFilterAltX(FilterHotkeys);
}

FrotzFrameWnd::~FrotzFrameWnd()
{
  delete m_clientWnd;
}

BEGIN_MESSAGE_MAP(FrotzFrameWnd, MenuBarFrameWnd)
  //{{AFX_MSG_MAP(FrotzFrameWnd)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_GETMINMAXINFO()
  ON_WM_CHAR()
  ON_MESSAGE(WM_INPUTLANGCHANGE, OnInputLangChange)
  ON_COMMAND(ID_HOT_DEBUG, OnHotDebug)
  ON_COMMAND(ID_HOT_HELP, OnHotHelp)
  ON_COMMAND(ID_HOT_PLAYBACK, OnHotPlayback)
  ON_COMMAND(ID_HOT_QUIT, OnHotQuit)
  ON_COMMAND(ID_HOT_RECORD, OnHotRecord)
  ON_COMMAND(ID_HOT_RESTART, OnHotRestart)
  ON_COMMAND(ID_HOT_SEED, OnHotSeed)
  ON_COMMAND(ID_HOT_UNDO, OnHotUndo)
  ON_COMMAND(ID_RL_BEGIN, OnReadLineStart)
  ON_COMMAND(ID_RL_END, OnReadLineEnd)
  ON_COMMAND(ID_RL_DEL, OnReadLineDel)
  ON_COMMAND(ID_RL_BACK, OnReadLineBack)
  ON_COMMAND(ID_RL_FORWARD, OnReadLineForward)
  ON_COMMAND(ID_RL_RUBOUT, OnReadLineRubout)
  ON_COMMAND(ID_RL_KILL, OnReadLineKill)
  ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
  ON_COMMAND(ID_HELP_FINDER, OnHelpFinder)
  ON_COMMAND(ID_FULLSCREEN, OnFullscreen)
  ON_COMMAND_RANGE(ID_LINKS_FROTZ, ID_LINKS_INFORM6, OnHelpLink)
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_TIME, OnUpdateTime)
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_ZCODE, OnUpdateZcode)
  //}}AFX_MSG_MAP
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

// Create the frame
bool FrotzFrameWnd::Create(bool toolbar, bool statusbar)
{
  // Register a window class
  LPCTSTR wndClass = AfxRegisterWndClass(0);

  // Create the frame
  if (!CreateEx(0,wndClass,CResString(IDS_TITLE),WS_OVERLAPPEDWINDOW,GetDefaultSize(),NULL,0))
    return false;

  // Set the icon
  SetIcon(::LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_INFOCOM)),TRUE);

  // Show the control bars
  ShowControlBar(&m_toolBar,toolbar,TRUE);
  ShowControlBar(&m_statusBar,statusbar,TRUE);

  // Create the child client window
  m_clientWnd = new FrotzWnd;
  return m_clientWnd->Create(this,m_dpi);
}

// Get the client window
FrotzWnd* FrotzFrameWnd::GetClientWnd(void)
{
  return m_clientWnd;
}

// Update the game defined menus
void FrotzFrameWnd::UpdateMenus(CArray<CStringArray,CStringArray&>& menus)
{
  if (m_menuBar.GetSafeHwnd() != 0)
  {
    // Reset the menu
    CMenu* menu = m_menuBar.GetMenu();
    menu->DestroyMenu();
    menu->LoadMenu(IDR_FROTZ);

    // Add the game defined menus
    for (int i = 0; i < menus.GetSize(); i++)
    {
      if (menus[i].GetSize() > 0)
      {
        CMenu submenu;
        submenu.CreateMenu();
        for (int j = 1; j < menus[i].GetSize(); j++)
          submenu.AppendMenu(MF_STRING,0x9000+(i<<8)+j,menus[i][j]);
        menu->AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)submenu.Detach(),menus[i][0]);
      }
    }

    // Make the new menus visible
    m_menuBar.Update();
  }
}

// Remove any game defined menus
void FrotzFrameWnd::ResetMenus(void)
{
  if (m_menuBar.GetSafeHwnd() != 0)
  {
    CMenu* menu = m_menuBar.GetMenu();
    menu->DestroyMenu();
    menu->LoadMenu(IDR_FROTZ);
    m_menuBar.Update();
  }
}

// Get a default size for the window
CRect FrotzFrameWnd::GetDefaultSize(void)
{
  // Get the size of the display
  CRect screen = ((FrotzApp*)AfxGetApp())->GetScreenSize(false);
  int l = screen.left;
  int t = screen.top;
  int w = screen.Width();
  int h = screen.Height();

  const int x = 8;
  const int y = 14;
  CRect size(l+(w/x),t+(h/y),(w*(x-1))/x,(h*(y-1))/y);
  return size;
}

BOOL FrotzFrameWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
  BOOL bPreCreate = MenuBarFrameWnd::PreCreateWindow(cs);
  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  return bPreCreate;
}

BOOL FrotzFrameWnd::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN)
  {
    if (m_clientWnd->InputVirtualKey(pMsg->wParam))
      return 1;
  }
  else if ((pMsg->message == WM_SYSKEYDOWN) && (pMsg->wParam == VK_F10))
  {
    if (m_clientWnd->InputVirtualKey(pMsg->wParam))
      return 1;
  }
  return MenuBarFrameWnd::PreTranslateMessage(pMsg);
}

int FrotzFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  m_dpi = DPI::getWindowDPI(this);
  if (MenuBarFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Set up the accelerators
  LoadAccelTable(MAKEINTRESOURCE(IDR_FROTZ));

  // Set up the menus and toolbar
  if (!CreateNewBar(IDR_FROTZ,IDR_TOOLBAR))
    return -1;
  if (m_menuBar.GetSafeHwnd() == 0)
  {
    CMenu menu;
    menu.LoadMenu(IDR_FROTZ);
    SetMenu(&menu);
    menu.Detach();
  }

  // Set up the status bar
  if (!m_statusBar.Create(this,WS_CHILD|CBRS_BOTTOM))
    return -1;
  if (!m_statusBar.SetIndicators(Indicators,sizeof(Indicators)/sizeof(UINT)))
    return -1;
  m_statusBar.SetTimer(FrotzWnd::StatusTimer,500,NULL);
  return 0;
}

void FrotzFrameWnd::OnDestroy() 
{
  FrotzApp* app = (FrotzApp*)AfxGetApp();

  app->StoreWindowSize();
  app->StoreBarState(
    (m_toolBar.GetStyle() & WS_VISIBLE) ? true : false,
    (m_statusBar.GetStyle() & WS_VISIBLE) ? true : false);

  MenuBarFrameWnd::OnDestroy();
}

BOOL FrotzFrameWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  BOOL bCmdProcessed = MenuBarFrameWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
  if (bCmdProcessed == FALSE)
  {
    // Check for game defined menus
    switch (nCode)
    {
    case CN_COMMAND:
      if ((nID >= 0x9000) && (nID < 0xA000))
      {
        m_clientWnd->InputMenu(nID-0x9000);
        bCmdProcessed = TRUE;
      }
      break;
    case CN_UPDATE_COMMAND_UI:
      {
        CCmdUI* pCmdUI = (CCmdUI*)pExtra;
        if (pCmdUI != NULL)
        {
          if (pCmdUI->m_pMenu != NULL)
          {
            // Game defined menu items are always enabled
            if ((nID >= 0x9000) && (nID < 0xA000))
            {
              pCmdUI->Enable();
              bCmdProcessed = TRUE;
            }
          }
        }
      }
      break;
    }
  }
  return bCmdProcessed;
}

void FrotzFrameWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  char c = (char)(nChar & 0xFF);

  // Convert to Unicode
  unsigned short unicode;
  if (::MultiByteToWideChar(m_codePage,MB_PRECOMPOSED,&c,1,(LPWSTR)&unicode,1) > 0)
    m_clientWnd->InputUnicode(unicode);

  MenuBarFrameWnd::OnChar(nChar,nRepCnt,nFlags);
}

LRESULT FrotzFrameWnd::OnInputLangChange(WPARAM wParam, LPARAM lParam)
{
  CHARSETINFO charSet;
  if (::TranslateCharsetInfo((DWORD*)wParam,&charSet,TCI_SRCCHARSET))
    m_codePage = charSet.ciACP;
  return DefWindowProc(WM_INPUTLANGCHANGE,wParam,lParam);
}

void FrotzFrameWnd::OnUpdateTime(CCmdUI* pCmdUI)
{
  CTimeSpan elapsed = ((FrotzApp*)AfxGetApp())->GetElapsedTime();
  pCmdUI->SetText(elapsed.Format("%H:%M:%S"));
  pCmdUI->Enable(TRUE);
}

void FrotzFrameWnd::OnUpdateZcode(CCmdUI* pCmdUI)
{
  CString version;
  version.Format("Z%d",h_version);
  pCmdUI->SetText(version);
  pCmdUI->Enable(TRUE);
}

void FrotzFrameWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  // Make the size a bit bigger than the display so full screen mode works
  CRect rect = ((FrotzApp*)AfxGetApp())->GetScreenSize(true);
  rect.InflateRect(64,128);
  CPoint size(rect.Size()); 

  lpMMI->ptMaxSize = size;
  lpMMI->ptMaxTrackSize = size;
}

void FrotzFrameWnd::OnUpdateFrameTitle(BOOL)
{
  CString title = ((FrotzApp*)AfxGetApp())->GetGameInfo().title;
  if (title.GetLength() > 0)
    title += " - ";
  CResString appTitle(IDS_TITLE);
  title += appTitle;
  AfxSetWindowText(GetSafeHwnd(),title);
}

void FrotzFrameWnd::OnEditPaste()
{
  if (OpenClipboard())
  {
    HGLOBAL handle = ::GetClipboardData(CF_TEXT);
    if (handle)
    {
      LPTSTR text = (LPTSTR)::GlobalLock(handle); 
      if (text) 
      {
        int len = strlen(text);
        for (int i = 0; i < len; i++)
        {
          unsigned short unicode;
          if (::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,text+i,1,(LPWSTR)&unicode,1) > 0)
            m_clientWnd->InputUnicode(unicode);
        }
        ::GlobalUnlock(handle); 
      }
    }
    CloseClipboard();
  }
}

void FrotzFrameWnd::OnHotDebug() 
{
  m_clientWnd->InputZcodeKey(ZC_HKEY_DEBUG);
}

void FrotzFrameWnd::OnHotHelp() 
{
  m_clientWnd->InputZcodeKey(ZC_HKEY_HELP);
}

void FrotzFrameWnd::OnHotPlayback() 
{
  m_clientWnd->InputZcodeKey(ZC_HKEY_PLAYBACK);
}

void FrotzFrameWnd::OnHotQuit() 
{
  m_clientWnd->InputZcodeKey(ZC_HKEY_QUIT);
}

void FrotzFrameWnd::OnHotRecord() 
{
  m_clientWnd->InputZcodeKey(ZC_HKEY_RECORD);
}

void FrotzFrameWnd::OnHotRestart() 
{
  m_clientWnd->InputZcodeKey(ZC_HKEY_RESTART);
}

void FrotzFrameWnd::OnHotSeed() 
{
  m_clientWnd->InputZcodeKey(ZC_HKEY_SEED);
}

void FrotzFrameWnd::OnHotUndo() 
{
  m_clientWnd->InputZcodeKey(ZC_HKEY_UNDO);
}

void FrotzFrameWnd::OnFullscreen()
{
  // Get the current window size
  WINDOWPLACEMENT place;
  ::ZeroMemory(&place,sizeof(WINDOWPLACEMENT));
  place.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(&place);
  CRect size(place.rcNormalPosition);

  // Get the size of the display
  FrotzApp* app = (FrotzApp*)AfxGetApp();
  CRect screen = app->GetScreenSize(true);

  // Is the window already full screen?
  if ((size.Width() > screen.Width()) && (size.Height() > screen.Height()))
  {
    // Restore the normal window size
    place.showCmd = SW_SHOWNORMAL;
    if (m_normalSize.Width() > 0)
      place.rcNormalPosition = m_normalSize;
    else
      place.rcNormalPosition = GetDefaultSize();
    SetWindowPlacement(&place);
  }
  else
  {
    if (app->GetNotifyFullScreen())
    {
      // Tell the user
      if (AfxMessageBox(IDS_FULLSCREEN,MB_YESNO|MB_ICONINFORMATION) == IDNO)
        return;
      app->SetNotifyFullScreen(false);
    }

    // Save the current window size
    m_normalSize = size;

    // Get the current Frotz window size
    CRect client;
    m_clientWnd->GetClientRect(&client);
    m_clientWnd->ClientToScreen(&client);

    // Calculate the window frame offsets
    GetWindowRect(&size);
    int x1 = client.left-size.left;
    int y1 = client.top-size.top;
    int x2 = size.right-client.right;
    int y2 = size.bottom-client.bottom;

    // Make the Frotz window client cover the entire display
    place.showCmd = SW_SHOWNORMAL;
    place.rcNormalPosition.left = screen.left-x1;
    place.rcNormalPosition.top = screen.top-y1;
    place.rcNormalPosition.right = screen.right+x2;
    place.rcNormalPosition.bottom = screen.bottom+y2;
    SetWindowPlacement(&place);
  }
}

void FrotzFrameWnd::OnReadLineStart()
{
  m_clientWnd->InputVirtualKey(VK_HOME);
}

void FrotzFrameWnd::OnReadLineEnd()
{
  m_clientWnd->InputVirtualKey(VK_END);
}

void FrotzFrameWnd::OnReadLineDel()
{
  m_clientWnd->InputVirtualKey(VK_DELETE);
}

void FrotzFrameWnd::OnReadLineBack()
{
  m_clientWnd->InputZcodeKey(ZC_ARROW_LEFT);
}

void FrotzFrameWnd::OnReadLineForward()
{
  m_clientWnd->InputZcodeKey(ZC_ARROW_RIGHT);
}

void FrotzFrameWnd::OnReadLineRubout()
{
  m_clientWnd->InputType(FrotzWnd::Input::RuboutWord);
}

void FrotzFrameWnd::OnReadLineKill()
{
  m_clientWnd->InputType(FrotzWnd::Input::KillLine);
}

namespace
{
  static char* links[] =
  {
    "http://www.davidkinder.co.uk/frotz.html",
    "http://www.ifarchive.org",
    "http://www.ifarchive.org/indexes/if-archive/games/zcode/",
    "http://www.ifdb.tads.org",
    "http://www.intfiction.org",
    "http://www.planet-if.com",
    "http://www.ifwiki.org",
    "http://www.ifcomp.org",
    "http://ifmud.port4000.com",
    "http://inform7.com",
    "http://inform-fiction.org"
  };
}

void FrotzFrameWnd::OnHelpLink(UINT nID)
{
  int i = nID - ID_LINKS_FROTZ;
  if (( i >= 0) && (i < sizeof links / sizeof links[0]))
    ::ShellExecute(0,NULL,links[i],NULL,NULL,SW_SHOWNORMAL);
}

void FrotzFrameWnd::GetMessageString(UINT nID, CString& rMessage) const
{
  int i = nID - ID_LINKS_FROTZ;
  if (( i >= 0) && (i < sizeof links / sizeof links[0]))
  {
    rMessage.Format(IDS_LINK,links[i]);
    return;
  }

  MenuBarFrameWnd::GetMessageString(nID,rMessage);
}

LRESULT FrotzFrameWnd::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  int newDpi = (int)HIWORD(wparam);
  MoveWindow((LPRECT)lparam,TRUE);

  if (m_dpi != newDpi)
  {
    m_dpi = newDpi;

    m_clientWnd->CreateFonts(m_dpi);
    FrotzWnd::TextSettings savedText = m_clientWnd->GetTextSettings();
    m_clientWnd->ApplyTextSettings(FrotzWnd::TextSettings(0,FIXED_WIDTH_FONT));
    h_font_width = (zbyte)m_clientWnd->GetCharWidth('0');
    h_font_height = (zbyte)m_clientWnd->GetFontHeight();
    m_clientWnd->ApplyTextSettings(savedText);
    m_clientWnd->InputType(FrotzWnd::Input::Reset);
  }

  // Force the menu and status bars to update
  UpdateDPI(newDpi);
  m_statusBar.SetIndicators(Indicators,sizeof(Indicators)/sizeof(UINT));
  return 0;
}
