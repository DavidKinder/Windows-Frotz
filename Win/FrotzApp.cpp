/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz application class
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FrotzApp.h"
#include "FrotzDialogs.h"
#include "FrotzGfx.h"
#include "FrotzSound.h"
#include "FrotzWnd.h"
#include "FrotzFrameWnd.h"

#include <MultiMon.h>

extern "C"
{
#include "blorblow.h"

int main(int argc, char* argv[]);
void reset_memory(void);
void replay_close(void);
void set_header_extension (int entry, zword val);
int colour_in_use(zword colour);
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(FrotzApp, CWinApp)
  //{{AFX_MSG_MAP(FrotzApp)
  ON_COMMAND(ID_FILE_NEW, OnFileNew)
  ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
  ON_COMMAND(ID_FILE_SAVE, OnFileSave)
  ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
  //}}AFX_MSG_MAP
  ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
  ON_COMMAND(ID_VIEW_SCROLLBACK, OnViewScrollback)
  ON_UPDATE_COMMAND_UI(ID_APP_ABOUT_GAME, OnUpdateAppAboutGame)
  ON_COMMAND(ID_APP_ABOUT_GAME, OnAppAboutGame)
  ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
  ON_COMMAND(ID_FILE_STOP, OnFileStop)
  ON_UPDATE_COMMAND_UI(ID_FILE_STOP, OnUpdateFileStop)
END_MESSAGE_MAP()

// The single application instance
FrotzApp theApp;

// The single output window instance
FrotzWnd* theWnd = NULL;

// Constructor
FrotzApp::FrotzApp()
{
  Initialize();
  FrotzGfx::SetGamma(2.2);
  EnableHtmlHelp();

  m_register = true;
  m_toolBar = true;
  m_statusBar = true;
  m_notifyFull = true;
  m_fastScroll = false;
  m_morePrompts = true;
  m_iFiction = Show_iF_First_Time;
  m_speak = false;
  m_speechRate = 0;
  m_spokenIndex = 0;

  // Standard Z-Machine colours
  m_colours[0]  = RGB5ToTrue(0x0000); // black
  m_colours[1]  = RGB5ToTrue(0x001D); // red
  m_colours[2]  = RGB5ToTrue(0x0340); // green
  m_colours[3]  = RGB5ToTrue(0x03BD); // yellow
  m_colours[4]  = RGB5ToTrue(0x59A0); // blue
  m_colours[5]  = RGB5ToTrue(0x7C1F); // magenta
  m_colours[6]  = RGB5ToTrue(0x77A0); // cyan
  m_colours[7]  = RGB5ToTrue(0x7FFF); // white
  m_colours[8]  = RGB5ToTrue(0x5AD6); // light grey
  m_colours[9]  = RGB5ToTrue(0x4631); // medium grey
  m_colours[10] = RGB5ToTrue(0x2D6B); // dark grey
}

// Initialize the application
void FrotzApp::Initialize(void)
{
  m_exitPause = false;
  m_blorbFile = NULL;
  m_blorbMap = NULL;

  for (int i = 0; i < NON_STD_COLS; i++)
    m_nonStdColours[i] = 0xFFFFFFFF;
  m_nonStdIndex = 0;

  if (theWnd != NULL)
    theWnd->Initialize();

  m_scrollback.SetSize(0,8192);
  m_startTime = CTime::GetCurrentTime();
}

// Run the interpreter core
FrotzApp::ExitStatus FrotzApp::RunInterpreter(void)
{
  try
  {
    // Start the interpreter
    main(__argc,__argv);
    AfxGetMainWnd()->DestroyWindow();
  }
  catch (FrotzApp::AbortFrotz&)
  {
    return Aborted;
  }
  catch (FrotzApp::ExitFrotz&)
  {
    return Exited;
  }
  catch (FrotzApp::RestartFrotz&)
  {
    return Restarting;
  }
  return Exited;
}

// Read in settings
void FrotzApp::ReadSettings(void)
{
  h_interpreter_number = GetProfileInt("Interpreter","Number",INTERP_AMIGA);
  err_report_mode = GetProfileInt("Interpreter","Error Reporting",ERR_REPORT_ONCE);
  option_ignore_errors = GetProfileInt("Interpreter","Ignore Errors",0);
  option_expand_abbreviations = GetProfileInt("Interpreter","Expand Abbreviations",0);
  m_tandy = GetProfileInt("Interpreter","Tandy Bit",0) ? true : false;
  m_quetzal = GetProfileInt("Interpreter","Quetzal Format",1) ? true : false;
  option_script_cols = GetProfileInt("Interpreter","Wrap Script Lines",1) ? 80 : 0;
  m_username = GetProfileString("Interpreter","Username");

  m_filename = GetProfileString("Files","Initial Game","");
  m_register = GetProfileInt("Files","Register File Types",0) ? true : false;
  m_iFiction = (Show_iFiction)GetProfileInt("Files","Show iFiction Dialog",Show_iF_First_Time);

  m_speak = GetProfileInt("Speech","Speak Text",0) ? true : false;
  m_speechVoice = GetProfileString("Speech","Voice","");
  m_speechRate = GetProfileInt("Speech","Speech Rate",0);

  m_wndSize.left = GetProfileInt("Window","Left",0);
  m_wndSize.top = GetProfileInt("Window","Top",0);
  m_wndSize.right = GetProfileInt("Window","Right",0);
  m_wndSize.bottom = GetProfileInt("Window","Bottom",0);
  m_wndState = GetProfileInt("Window","State",SW_SHOWNORMAL);

  m_toolBar = GetProfileInt("Window","Toolbar",1) ? true : false;
  m_statusBar = GetProfileInt("Window","Status Bar",1) ? true : false;
  m_notifyFull = GetProfileInt("Window","Notify Full Screen",1) ? true : false;

  m_propFontName = GetProfileString("Display","Proportional Font Name",
    GetDefaultFont());
  m_fixedFontName = GetProfileString("Display","Fixed Font Name",
    GetDefaultFixedFont());
  m_fontSize = GetProfileInt("Display","Font Size",10);
  m_v6scale = GetProfileInt("Display","Infocom V6 Scaling",2);
  m_defaultFore = Make5BitColour(
    GetProfileInt("Display","Foreground",RGB(0xFF,0xFF,0xFF)));
  m_defaultBack = Make5BitColour(
    GetProfileInt("Display","Background",RGB(0x00,0x00,0x80)));
  m_fastScroll = GetProfileInt("Display","Fast Scrolling",0) ? true : false;
  m_morePrompts = GetProfileInt("Display","Show More Prompts",1) ? true : false;
  option_left_margin = GetProfileInt("Display","Left Margin",0);
  option_right_margin = GetProfileInt("Display","Right Margin",0);

  // Are the settings from the latest version?
  int version = GetProfileInt("Version","Number",0);
  if (version < 103)
  {
    CRect screen = GetScreenSize(true);
    if ((screen.Width() > 800) && (screen.Height() > 600))
    {
      m_fontSize = 10;
      m_v6scale = 3;
    }
    else
    {
      m_fontSize = 10;
      m_v6scale = 2;
    }

    // Ignore errors by default
    err_report_mode = ERR_REPORT_NEVER;
  }
  if (version < 111)
  {
    if (m_propFontName.Compare("Times New Roman") == 0)
      m_propFontName = GetDefaultFont();
    if (m_fixedFontName.Compare("Courier New") == 0)
      m_fixedFontName = GetDefaultFixedFont();
  }
  if (version < 112)
  {
    if (option_left_margin < 4)
      option_left_margin = 4;
    if (option_right_margin < 4)
      option_right_margin = 4;
  }
}

// Write out settings
void FrotzApp::WriteSettings(void)
{
  // The current version number
  WriteProfileInt("Version","Number",112);

  WriteProfileInt("Interpreter","Number",h_interpreter_number);
  WriteProfileInt("Interpreter","Error Reporting",err_report_mode);
  WriteProfileInt("Interpreter","Ignore Errors",option_ignore_errors);
  WriteProfileInt("Interpreter","Expand Abbreviations",option_expand_abbreviations);
  WriteProfileInt("Interpreter","Tandy Bit",m_tandy ? 1 : 0);
  WriteProfileInt("Interpreter","Quetzal Format",m_quetzal ? 1 : 0);
  WriteProfileInt("Interpreter","Wrap Script Lines",option_script_cols == 0 ? 0 : 1);
  WriteProfileString("Interpreter","Username",m_username);

  WriteProfileString("Files","Initial Game",m_filename);
  WriteProfileInt("Files","Register File Types",m_register ? 1 : 0);
  WriteProfileInt("Files","Show iFiction Dialog",m_iFiction);

  WriteProfileInt("Speech","Speak Text",m_speak ? 1 : 0);
  WriteProfileString("Speech","Voice",m_speechVoice);
  WriteProfileInt("Speech","Speech Rate",m_speechRate);

  WriteProfileInt("Window","Left",m_wndSize.left);
  WriteProfileInt("Window","Top",m_wndSize.top);
  WriteProfileInt("Window","Right",m_wndSize.right);
  WriteProfileInt("Window","Bottom",m_wndSize.bottom);
  WriteProfileInt("Window","State",m_wndState);

  WriteProfileInt("Window","Toolbar",m_toolBar ? 1 : 0);
  WriteProfileInt("Window","Status Bar",m_statusBar ? 1 : 0);
  WriteProfileInt("Window","Notify Full Screen",m_notifyFull ? 1 : 0);

  WriteProfileString("Display","Proportional Font Name",m_propFontName);
  WriteProfileString("Display","Fixed Font Name",m_fixedFontName);
  WriteProfileInt("Display","Font Size",m_fontSize);
  WriteProfileInt("Display","Infocom V6 Scaling",m_v6scale);
  WriteProfileInt("Display","Foreground",m_defaultFore);
  WriteProfileInt("Display","Background",m_defaultBack);
  WriteProfileInt("Display","Fast Scrolling",m_fastScroll);
  WriteProfileInt("Display","Show More Prompts",m_morePrompts);
  WriteProfileInt("Display","Left Margin",option_left_margin);
  WriteProfileInt("Display","Right Margin",option_right_margin);
}

// Get the default font
CString FrotzApp::GetDefaultFont(void)
{
  // Get desktop settings
  NONCLIENTMETRICS ncm;
  ::ZeroMemory(&ncm,sizeof ncm);
  ncm.cbSize = sizeof ncm;
  ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof ncm,&ncm,0);
  CString fontName(ncm.lfMessageFont.lfFaceName);

  // Get a device context
  CWnd* wnd = CWnd::GetDesktopWindow();
  CDC* dc = wnd->GetDC();

  // Create the font
  CFont font;
  font.CreatePointFont(10,fontName);

  // Test if the font is TrueType
  CFont* oldFont = dc->SelectObject(&font);
  if (dc->GetOutlineTextMetrics(0,NULL) == 0)
    fontName = "Times New Roman";

  // Free the device context
  dc->SelectObject(oldFont);
  wnd->ReleaseDC(dc);

  return fontName;
}

static int CALLBACK EnumFontProc(ENUMLOGFONTEX*, NEWTEXTMETRICEX* ,DWORD, LPARAM found)
{
  *((bool*)found) = true;
  return 0;
}

// Get the default fixed width font
CString FrotzApp::GetDefaultFixedFont(void)
{
  CString fontName = "Courier";

  // Get a device context for the display
  CWnd* wnd = CWnd::GetDesktopWindow();
  CDC* dc = wnd->GetDC();

  // List of fixed width fonts to look for
  const char* fixedFonts[] =
  {
    "Consolas",
    "Lucida Console",
    "Courier New"
  };

  // Search the list of fixed width fonts for a match
  LOGFONT fontInfo;
  ::ZeroMemory(&fontInfo,sizeof fontInfo);
  fontInfo.lfCharSet = DEFAULT_CHARSET;
  bool found = false;
  for (int i = 0; i < sizeof fixedFonts / sizeof fixedFonts[0]; i++)
  {
    strcpy(fontInfo.lfFaceName,fixedFonts[i]);
    ::EnumFontFamiliesEx(dc->GetSafeHdc(),&fontInfo,(FONTENUMPROC)EnumFontProc,(LPARAM)&found,0);
    if (found)
    {
      fontName = fontInfo.lfFaceName;
      break;
    }
  }

  // Release the desktop device context
  wnd->ReleaseDC(dc);
  return fontName;
}

// Get the display settings
void FrotzApp::GetDisplaySettings(CString& propName, CString& fixedName, int& size)
{
  propName = m_propFontName;
  fixedName = m_fixedFontName;
  size = m_fontSize;
}

BOOL AFXAPI _AfxSetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);
void AFXAPI AfxGetModuleShortFileName(HINSTANCE hInst, CString& strShortName);

// Register file types
void FrotzApp::RegisterFileTypes(void)
{
  // Check that file registration hasn't been disabled
  if (m_register == false)
    return;

  // Get the path to the executable
  CString path;
  AfxGetModuleShortFileName(AfxGetInstanceHandle(),path);

  // Register each Z-code file extension
  CString extension, type, key, value;
  for (int i = 1; i <= 8; i++)
  {
    extension.Format(".z%d",i);
    type.Format("ZMachine.V%d",i);
    if (_AfxSetRegKey(extension,type))
    {
      // Set up a name for the file type
      _AfxSetRegKey(type,"Z-code Text Adventure");

      // Set up an icon
      key.Format("%s\\DefaultIcon",type);
      value.Format("%s,%d",path,i);
      _AfxSetRegKey(key,value);

      // Set up an open command
      key.Format("%s\\shell\\open\\command",type);
      value.Format("%s \"%%1\"",path);
      _AfxSetRegKey(key,value);
    }
  }

  // Register file extensions for Blorb files with Z-code in them
  type = "ZMachine.Blorb";
  if (_AfxSetRegKey(".zlb",type) && _AfxSetRegKey(".zblorb",type))
  {
    // Set up a name for the file type
    _AfxSetRegKey(type,"Z-code Text Adventure");

    // Set up an icon
    key.Format("%s\\DefaultIcon",type);
    value.Format("%s,%d",path,9);
    _AfxSetRegKey(key,value);

    // Set up an open command
    key.Format("%s\\shell\\open\\command",type);
    value.Format("%s \"%%1\"",path);
    _AfxSetRegKey(key,value);
  }

  // Notify the shell that associations have changed
  ::SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,0,0);
}

// Load an international version of resources
void FrotzApp::LoadInternationalResources(void)
{
  const char* resDllName = NULL;
  switch (PRIMARYLANGID(::GetUserDefaultLangID()))
  {
  case LANG_FRENCH:
    resDllName = "FrotzFrançais.dll";
    break;
  case LANG_GERMAN:
    resDllName = "FrotzDeutsch.dll";
    break;
  case LANG_ITALIAN:
    resDllName = "FrotzItaliano.dll";
    break;
  case LANG_SPANISH:
    resDllName = "FrotzEspañol.dll";
    break;
  }

  if (resDllName != NULL)
  {
    HINSTANCE dll = ::LoadLibrary(resDllName);
    if (dll != NULL)
      AfxSetResourceHandle(dll);
  }
}

// Open a file dialog to prompt the user for a game
bool FrotzApp::PromptForGame(bool initial)
{
  if (initial)
  {
    // Use the command line argument, if available
    if (__argc > 1)
    {
      m_filename = __argv[1];

      // Convert to a full path name
      char fullname[_MAX_PATH];
      char* part;
      if (::GetFullPathName(__argv[1],_MAX_PATH,fullname,&part) != 0)
        m_filename = fullname;

      return true;
    }

    // Look for a Z-code file with the same name as the
    // interpreter executable
    char filename[_MAX_PATH];
    char* extensions[] = { "z5","z6","z8","zlb",NULL };
    if (::GetModuleFileName(0,filename,_MAX_PATH) != 0)
    {
      strlwr(filename);
      char* ext = strstr(filename,".exe");
      if (ext != NULL)
      {
        ext++;
        for (int i = 0; extensions[i] != NULL; i++)
        {
          strcpy(ext,extensions[i]);
          if (::GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES)
          {
            m_filename = filename;
            return true;
          }
        }
      }
    }
  }

  return GameFileDialog::ShowDialog(m_filename,AfxGetMainWnd());
}

// Get the filename of the game to run
LPCTSTR FrotzApp::GetGameFileName(void)
{
  return m_filename;
}

// Create the main window
void FrotzApp::CreateMainWindow(void)
{
  FrotzFrameWnd* wnd = (FrotzFrameWnd*)m_pMainWnd;
  if (wnd == NULL)
  {
    wnd = new FrotzFrameWnd;
    if (wnd->Create(m_toolBar,m_statusBar) == false)
      throw AbortFrotz();
    m_pMainWnd = wnd;
  }

  // Reset the menus
  wnd->ResetMenus();

  // Set the graphics scaling
  if (IsInfocomV6() || (story_id == BEYOND_ZORK))
    m_gfxScale = m_v6scale;
  else
    m_gfxScale = 1;

  // Get the Frotz output window
  theWnd = wnd->GetClientWnd();

  // Resize the window. For Infocom's V6 games, a fixed
  // window size is always used
  theWnd->SetAllowResize(false);
  if (IsInfocomV6())
  {
    CRect screen;
    ::SystemParametersInfo(SPI_GETWORKAREA,0,(LPRECT)screen,0);

    // Resize the window large enough that the non-client
    // area can be measured
    if (wnd->IsWindowVisible() == FALSE)
      wnd->MoveWindow(CRect(0,0,640,400),TRUE);

    CRect clientSize, windowSize;
    theWnd->GetClientRect(clientSize);
    wnd->GetWindowRect(windowSize);

    int borderX = windowSize.Width() - clientSize.Width();
    int borderY = windowSize.Height() - clientSize.Height();
    int width = (320*m_gfxScale)+borderX;
    int height = (200*m_gfxScale)+borderY;

    WINDOWPLACEMENT place;
    ::ZeroMemory(&place,sizeof(WINDOWPLACEMENT));
    place.length = sizeof(WINDOWPLACEMENT);
    wnd->GetWindowPlacement(&place);
    if (m_wndSize.Width() > 0)
      place.rcNormalPosition = m_wndSize;

    int x = place.rcNormalPosition.left;
    int y = place.rcNormalPosition.top;
    if (x+width > screen.Width())
      x = screen.Width()-width;
    if (y+height > screen.Height())
      y = screen.Height()-height;
    if (x < 0)
      x = 0;
    if (y < 0)
      y = 0;

    place.showCmd = SW_SHOWNORMAL;
    place.rcNormalPosition.left = x;
    place.rcNormalPosition.top = y;
    place.rcNormalPosition.right = x+width;
    place.rcNormalPosition.bottom = y+height;
    wnd->SetWindowPlacement(&place);
  }
  else
  {
    WINDOWPLACEMENT place;
    ::ZeroMemory(&place,sizeof(WINDOWPLACEMENT));
    place.length = sizeof(WINDOWPLACEMENT);

    wnd->GetWindowPlacement(&place);
    place.showCmd = m_wndState;
    if (m_wndSize.Width() > 0)
      place.rcNormalPosition = m_wndSize;
    wnd->SetWindowPlacement(&place);
  }
  theWnd->SetAllowResize(true);

  wnd->UpdateWindow();
  ::SetCursor(::LoadCursor(NULL,IDC_ARROW));
}

// Store the size of the main window
void FrotzApp::StoreWindowSize(void)
{
  if (IsInfocomV6() == false)
  {
    if (m_pMainWnd != NULL)
    {
      WINDOWPLACEMENT place;
      ::ZeroMemory(&place,sizeof(WINDOWPLACEMENT));
      place.length = sizeof(WINDOWPLACEMENT);
      m_pMainWnd->GetWindowPlacement(&place);

      m_wndSize = place.rcNormalPosition;
      m_wndState = place.showCmd;
    }
  }
}

// Store the state of the control bars
void FrotzApp::StoreBarState(bool toolbar, bool statusbar)
{
  m_toolBar = toolbar;
  m_statusBar = statusbar;
}

// Process messages
void FrotzApp::MessagePump(void)
{
  MSG msg;
  if (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
  { 
    while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
    {
      if (PumpMessage() == FALSE)
        throw ExitFrotz();
    }
  }
  else
  {
    LONG lIdle = 0;
    BOOL bIdle = TRUE;
    while (AfxGetMainWnd() && bIdle)
      bIdle = CWinApp::OnIdle(lIdle++);
    ::WaitMessage();
  }

  if (AfxGetMainWnd() == NULL)
    throw ExitFrotz();
}

// Get the elapsed time for this game
CTimeSpan FrotzApp::GetElapsedTime(void)
{
  return CTime::GetCurrentTime() - m_startTime;
}

// Check if a character is printable
bool FrotzApp::IsValidChar(unsigned short c)
{
  if (c >= ZC_ASCII_MIN && c <= ZC_ASCII_MAX)
    return true;
  if (c >= ZC_LATIN1_MIN && c <= ZC_LATIN1_MAX)
    return true;
  if (c >= 0x100)
    return true;
  return false;
}

// Get the size of the screen
CRect FrotzApp::GetScreenSize(bool full)
{
  MONITORINFO monInfo;
  ::ZeroMemory(&monInfo,sizeof monInfo);
  monInfo.cbSize = sizeof monInfo;

  HMONITOR mon = ::MonitorFromWindow(AfxGetMainWnd()->GetSafeHwnd(),MONITOR_DEFAULTTOPRIMARY);
  if (::GetMonitorInfo(mon,&monInfo))
    return full ? monInfo.rcMonitor : monInfo.rcWork;

  return CRect(0,0,::GetSystemMetrics(SM_CXSCREEN),::GetSystemMetrics(SM_CYSCREEN));
}

// Get a default colour
COLORREF FrotzApp::GetDefaultColour(bool fore)
{
  if (IsInfocomV6())
    return GetColour(fore ? WHITE_COLOUR : BLACK_COLOUR);
  return fore ? m_defaultFore : m_defaultBack;
}

// Get a colour
COLORREF FrotzApp::GetColour(int colour)
{
  // Standard colours
  if ((colour >= BLACK_COLOUR) && (colour <= DARKGREY_COLOUR))
    return m_colours[colour-BLACK_COLOUR];

  // Default colours
  if (colour == 16)
    return m_defaultFore;
  if (colour == 17)
    return m_defaultBack;

  // Non standard colours
  if ((colour >= 18) && (colour < 256))
  {
    if (m_nonStdColours[colour-18] != 0xFFFFFFFF)
      return m_nonStdColours[colour-18];
  }
  return m_colours[0];
}

// Get an index for a non-standard colour
int FrotzApp::GetColourIndex(COLORREF colour)
{
  // Is this a standard colour?
  for (int i = 0; i < 11; i++)
  {
    if (m_colours[i] == colour)
      return i+BLACK_COLOUR;
  }

  // Is this a default colour?
  if (m_defaultFore == colour)
    return 16;
  if (m_defaultBack == colour)
    return 17;

  // Is this colour already in the table?
  for (int i = 0; i < NON_STD_COLS; i++)
  {
    if (m_nonStdColours[i] == colour)
      return i+18;
  }

  // Find a free colour index
  int index = -1;
  while (index == -1)
  {
    if (colour_in_use(m_nonStdIndex+18) == 0)
    {
      m_nonStdColours[m_nonStdIndex] = colour;
      index = m_nonStdIndex+18;
    }

    m_nonStdIndex++;
    if (m_nonStdIndex >= NON_STD_COLS)
      m_nonStdIndex = 0;
  }
  return index;
}

// Adjust a foreground colour for the given style
COLORREF FrotzApp::AdjustForeColour(COLORREF colour, int style)
{
  if (h_version != V6)
  {
    if (style & BOLDFACE_STYLE)
    {
      int r = GetRValue(colour)+0x30;
      if (r > 0xFF)
        r = 0xFF;
      int g = GetGValue(colour)+0x30;
      if (g > 0xFF)
        g = 0xFF;
      int b = GetBValue(colour)+0x30;
      if (b > 0xFF)
        b = 0xFF;
      return RGB(r,g,b);
    }
  }
  return colour;
}

// Convert from 5-bit RGB to a true colour
COLORREF FrotzApp::RGB5ToTrue(unsigned short five)
{
  int r = five&0x001F;
  int g = (five&0x03E0)>>5;
  int b = (five&0x7C00)>>10;
  return RGB((r<<3)|(r>>2),(g<<3)|(g>>2),(b<<3)|(b>>2));
}

// Convert from a true colour to 5-bit RGB
unsigned short FrotzApp::TrueToRGB5(COLORREF colour)
{
  int r = GetRValue(colour)>>3;
  int g = GetGValue(colour)>>3;
  int b = GetBValue(colour)>>3;
  return r|(g<<5)|(b<<10);
}

// Rescale colour components so that the colour can be expressed
// exactly in 5-bit RGB
COLORREF FrotzApp::Make5BitColour(COLORREF colour)
{
  int r = GetRValue(colour)>>3;
  int g = GetGValue(colour)>>3;
  int b = GetBValue(colour)>>3;
  return RGB((r<<3)|(r>>2),(g<<3)|(g>>2),(b<<3)|(b>>2));
}

// Get whether to pause on exit
bool FrotzApp::GetExitPause(void)
{
  return m_exitPause;
}

// Set whether to pause on exit
void FrotzApp::SetExitPause(bool pause)
{
  m_exitPause = pause;
}

// Get whether to use the Quetzal save format
bool FrotzApp::GetUseQuetzal(void)
{
  return m_quetzal;
}

// Set whether to use the Quetzal save format
void FrotzApp::SetUseQuetzal(bool quetzal)
{
  m_quetzal = quetzal;
}

// Get whether to notify when entering full screen mode
bool FrotzApp::GetNotifyFullScreen(void)
{
  return m_notifyFull;
}

// Set whether to notify when entering full screen mode
void FrotzApp::SetNotifyFullScreen(bool notify)
{
  m_notifyFull = notify;
}

// Get whether to use fast or slow scrolling
bool FrotzApp::GetFastScrolling(void)
{
  return m_fastScroll;
}

// Get whether to show [More] prompts
bool FrotzApp::GetShowMorePrompts(void)
{
  return m_morePrompts;
}

// Set whether line input is in progress
void FrotzApp::SetLineInput(bool input)
{
  m_lineInput = input;
}

// Set up the Blorb resource file
void FrotzApp::SetBlorbFile(void)
{
  if (m_blorbMap == NULL)
  {
    // Create the filename from the story filename
    CString blorbFileName(story_name);
    {
      int ext = blorbFileName.ReverseFind('.');
      if (ext > 0)
      {
        char* blorbExts[] = { ".blb", ".blorb", NULL };

        for (int i = 0; blorbExts[i] != NULL; i++)
        {
          blorbFileName = blorbFileName.Left(ext);
          blorbFileName += blorbExts[i];

          m_blorbFile = fopen(blorbFileName,"rb");
          if (m_blorbFile != NULL)
          {
            if (bb_create_map(m_blorbFile,&m_blorbMap) == bb_err_None)
              break;
            CloseBlorbFile();
          }
        }
      }
    }
  }

  // Initialize values that can be loaded from the Blorb file
  GameInfo emptyInfo;
  m_gameInfo = emptyInfo;

  // Load data from the Blorb file
  if (m_blorbMap != NULL)
  {
    FrotzGfx::LoadPaletteInfo(m_blorbMap);
    LoadMetadata();
  }

  // Update the window title
  ((FrotzFrameWnd*)m_pMainWnd)->DelayUpdateFrameTitle();

  // Show the "About this game" dialog
  if ((m_gameInfo.ifid.IsEmpty() == FALSE) && CheckGameId())
  {
    theWnd->ClearDisplay();
    OnIdle(0);

    AboutGameDialog dialog(AfxGetMainWnd());
    dialog.DoModal();
  }
}

// Set up the Blorb resource file, given a file handle
void FrotzApp::SetBlorbZCode(FILE* file, long* size)
{
  if (m_blorbMap == NULL)
  {
    if (bb_create_map(file,&m_blorbMap) == bb_err_None)
    {
      // Look for an executable chunk
      bb_result_t result;
      if (bb_load_resource(m_blorbMap,bb_method_FilePos,&result,bb_ID_Exec,0) == bb_err_None)
      {
        unsigned int id = m_blorbMap->chunks[result.chunknum].type;
        if (id == bb_make_id('Z','C','O','D'))
        {
          // If this is a Z-code game, set the file pointer and return
          fseek(file,result.data.startpos,SEEK_SET);
          *size = result.length;
          return;
        }
        else if (id == bb_make_id('G','L','U','L'))
        {
          // Tell the user to use Windows Glulxe instead
          ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
            CResString(IDS_BLORB_GLULX),CResString(IDS_FATAL),MB_ICONERROR|MB_OK);
          throw AbortFrotz();
        }
      }

      // Tell the user that there was no game in the Blorb file
      ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
        CResString(IDS_BLORB_NOEXEC),CResString(IDS_FATAL),MB_ICONERROR|MB_OK);
      throw AbortFrotz();
    }

    // This is not a Blorb file, so go back to the start
    fseek(file,0,SEEK_SET);
    *size = -1;
  }
}

// Close the Blorb resource file
void FrotzApp::CloseBlorbFile(void)
{
  if (m_blorbMap)
    bb_destroy_map(m_blorbMap);
  m_blorbMap = NULL;
  if (m_blorbFile)
    fclose(m_blorbFile);
  m_blorbFile = NULL;
}

// Returns true if a Blorb resource file has been loaded
bool FrotzApp::GotBlorbFile(void)
{
  return (m_blorbMap != NULL);
}

// Returns the Blorb map
bb_map_t* FrotzApp::GetBlorbMap(void)
{
  return m_blorbMap;
}

// Load IF metadata
void FrotzApp::LoadMetadata(void)
{
  bb_result_t result;

  unsigned int id_Fspc = bb_make_id('F','s','p','c');
  if (bb_load_chunk_by_type(m_blorbMap,bb_method_Memory,&result,id_Fspc,0) == bb_err_None)
  {
    unsigned char* data = (unsigned char*)result.data.ptr;
    m_gameInfo.cover = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
    bb_unload_chunk(m_blorbMap,result.chunknum);
  }

  unsigned int id_IFmd = bb_make_id('I','F','m','d');
  if (bb_load_chunk_by_type(m_blorbMap,bb_method_Memory,&result,id_IFmd,0) != bb_err_None)
    return;

  CString meta((const char*)result.data.ptr,result.length);
  bb_unload_chunk(m_blorbMap,result.chunknum);

#ifdef _DEBUG
  theWnd->OpenClipboard();
  ::EmptyClipboard();
  HGLOBAL metaClip = ::GlobalAlloc(GMEM_MOVEABLE,meta.GetLength()+1);
  void* metaPtr = ::GlobalLock(metaClip);
  strcpy((char*)metaPtr,meta);
  ::GlobalUnlock(metaClip);
  ::SetClipboardData(CF_TEXT,metaClip);
  ::CloseClipboard();
  TRACE("Copied iFiction XML to the clipboard.\n");
#endif

  CComPtr<IXMLDOMDocument> doc;
  if (FAILED(doc.CoCreateInstance(CLSID_DOMDocument)))
    return;

  VARIANT_BOOL success = 0;
  CStreamOnCString metaStream(meta);
  if (doc->load(CComVariant(&metaStream),&success) != S_OK)
    return;

  CString version = StrFromXML(doc,L"/ifindex/@version");
  if (version == "0.9")
  {
    m_gameInfo.ifid = StrFromXML(doc,L"/ifindex/story/id/uuid");
    m_gameInfo.title = StrFromXML(doc,L"/ifindex/story/title");
    m_gameInfo.headline = StrFromXML(doc,L"/ifindex/story/headline");
    m_gameInfo.author = StrFromXML(doc,L"/ifindex/story/author");
    m_gameInfo.year = StrFromXML(doc,L"/ifindex/story/year");

    m_gameInfo.description = StrFromXML(doc,L"/ifindex/story/description");
    m_gameInfo.description.Replace(127,'\r');

    if (m_gameInfo.cover == -1)
    {
      CString cover = StrFromXML(doc,L"/ifindex/story/coverpicture");
      if (cover.IsEmpty() == FALSE)
        m_gameInfo.cover = atoi(cover);
    }
    m_gameInfo.coverFormatWrong = true;
  }
  else
  {
    m_gameInfo.ifid = StrFromXML(doc,L"/ifindex/story/identification/ifid");
    m_gameInfo.title = StrFromXML(doc,L"/ifindex/story/bibliographic/title");
    m_gameInfo.headline = StrFromXML(doc,L"/ifindex/story/bibliographic/headline");
    m_gameInfo.author = StrFromXML(doc,L"/ifindex/story/bibliographic/author");
    m_gameInfo.year = StrFromXML(doc,L"/ifindex/story/bibliographic/firstpublished");
    m_gameInfo.series = StrFromXML(doc,L"/ifindex/story/bibliographic/series");
    m_gameInfo.seriesNumber = StrFromXML(doc,L"/ifindex/story/bibliographic/seriesnumber");

    {
      CComPtr<IXMLDOMNode> node;
      CComBSTR path(L"/ifindex/story/bibliographic/description");
      if (SUCCEEDED(doc->selectSingleNode(path,&node)) && (node != NULL))
      {
        CComPtr<IXMLDOMNodeList> childList;
        if (SUCCEEDED(node->get_childNodes(&childList)))
        {
          CComPtr<IXMLDOMNode> childNode;
          while (SUCCEEDED(childList->nextNode(&childNode)))
          {
            if (childNode == NULL)
              break;

            DOMNodeType type;
            if (SUCCEEDED(childNode->get_nodeType(&type)))
            {
              switch (type)
              {
              case NODE_TEXT:
                {
                  CComBSTR text;
                  childNode->get_text(&text);

                  CString unformatted(text.m_str), formatted, token;
                  int pos = 0;
                  token = unformatted.Tokenize(" \t\r\n",pos);
                  while (token.IsEmpty() == FALSE)
                  {
                    if (formatted.IsEmpty() == FALSE)
                      formatted.AppendChar(' ');
                    formatted.Append(token);
                    token = unformatted.Tokenize(" \t\r\n",pos);
                  }

                  m_gameInfo.description.Append(formatted);
                }
                break;
              case NODE_ELEMENT:
                {
                  CComBSTR name;
                  childNode->get_nodeName(&name);
                  if (name == L"br")
                    m_gameInfo.description.Append("\r\r");
                }
                break;
              }
            }

            childNode.Release();
          }
        }
      }
    }

    if (m_gameInfo.cover == -1)
    {
      CString cover = StrFromXML(doc,L"/ifindex/story/zcode/coverpicture");
      if (cover.IsEmpty() == FALSE)
        m_gameInfo.cover = atoi(cover);
    }
  }
}

// Check for an already known game
bool FrotzApp::CheckGameId(void)
{
  if (m_gameInfo.ifid.IsEmpty())
    return false;

  // Does a key exist for this game? If not, create it
  CRegKey key;
  DWORD disposition = 0;
  if (key.Create(GetSectionKey("Known Games"),m_gameInfo.ifid,REG_NONE,REG_OPTION_NON_VOLATILE,
    KEY_READ|KEY_WRITE,NULL,&disposition) != ERROR_SUCCESS)
  {
    return false;
  }

  switch (m_iFiction)
  {
  case Show_iF_Never:
    return false;
  case Show_iF_First_Time:
    return (disposition == REG_CREATED_NEW_KEY);
  case Show_iF_Always:
    return true;
  default:
    return false;
  }
}

// Get a string from an XML node
CString FrotzApp::StrFromXML(IXMLDOMDocument* doc, LPCWSTR path)
{
  CComPtr<IXMLDOMNode> node;
  if (SUCCEEDED(doc->selectSingleNode(CComBSTR(path),&node)) && (node != NULL))
  {
    CComBSTR text;
    node->get_text(&text);
    return text.m_str;
  }
  return "";
}

// Get the iFiction metadata
const FrotzApp::GameInfo& FrotzApp::GetGameInfo(void)
{
  return m_gameInfo;
}

// If true, running one of Infocom's V6 games
bool FrotzApp::IsInfocomV6(void)
{
  switch (story_id)
  {
  case ARTHUR:
  case JOURNEY:
  case SHOGUN:
  case ZORK_ZERO:
    return true;
  }
  return false;
}

// If true, the Tandy header flag should be set
bool FrotzApp::IsTandyBitSet(void)
{
  return m_tandy;
}

// Get the scaling for graphics
int FrotzApp::GetGfxScaling(void)
{
  return m_gfxScale;
}

// Copy the user name into the interpreter fields
void FrotzApp::CopyUsername(void)
{
  CString name = m_username.Left(8);
  for (int i = 0; i < 8; i++)
    h_user_name[i] = 0;
  for (int i = 0; i < name.GetLength(); i++)
    h_user_name[i] = name[i];
}

// Add a character to the scrollback buffer
void FrotzApp::ScrollbackChar(unsigned short c)
{
  m_scrollback.Add(c);
}

// Remove characters from the scrollback buffer
void FrotzApp::ScrollbackRemove(int remove)
{
  int size = m_scrollback.GetSize() - remove;
  if (size < 0)
    size = 0;
  m_scrollback.SetSize(size,8192);
}

// Speak text added to the scrollback buffer
void FrotzApp::SpeakText(void)
{
  if (m_spokenIndex < m_scrollback.GetSize())
  {
    if (m_speak && TextToSpeech::GetSpeechEngine().IsAvailable())
    {
      // Make sure that the speech engine is set up
      TextToSpeech::GetSpeechEngine().Initialize(m_speechVoice,m_speechRate);

      // Copy the text to be spoken
      CStringW text;
      text.Preallocate(m_scrollback.GetSize()-m_spokenIndex+1);
      for (int i = m_spokenIndex; i < m_scrollback.GetSize(); i++)
      {
        unsigned short c = m_scrollback.GetAt(i);
        switch (c)
        {
        case L'[':
        case L']':
        case L'{':
        case L'}':
        case L'<':
        case L'>':
          continue;
        default:
          text.AppendChar(c);
        }
      }
      TextToSpeech::GetSpeechEngine().Speak(text);
    }
    m_spokenIndex = m_scrollback.GetSize();
  }
}

// Initialize and run the interpreter
BOOL FrotzApp::InitInstance()
{
  ::CoInitialize(NULL);
  AfxInitRichEdit2();

  // Set the registry key names
  SetRegistryKey("David Kinder");
  free((void*)m_pszProfileName);
  m_pszProfileName = strdup("Frotz");

  // Set the application name
  free((void*)m_pszAppName);
  m_pszAppName = strdup(CResString(IDS_TITLE));

  // Read in settings
  ReadSettings();

  // Register file types
  RegisterFileTypes();

  // Load international resources, if needed
  LoadInternationalResources();

  // Get a game to run
  if (PromptForGame(true))
  {
    // Run the interpreter until it exits or aborts
    ExitStatus status = Restarting;
    while (status == Restarting)
    {
      Initialize();
      option_save_quetzal = m_quetzal ? 1 : 0;
      status = RunInterpreter();
      reset_memory();
      FrotzGfx::ClearCache();
      FrotzSound::Stop(0);
      CloseBlorbFile();
    }

    // Write out settings
    if (status != Aborted)
      WriteSettings();
  }

  FrotzSound::ShutDown();
  return FALSE;
}

int FrotzApp::ExitInstance()
{
  ::CoUninitialize();
  return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers
/////////////////////////////////////////////////////////////////////////////

// Load a new game
void FrotzApp::OnFileNew() 
{
  if (PromptForGame(false))
  {
    StoreWindowSize();
    throw RestartFrotz();
  }
}

// Load a saved game
void FrotzApp::OnFileOpen() 
{
  if (m_lineInput)
    theWnd->InputString("restore\r");
}

void FrotzApp::OnUpdateFileOpen(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(m_lineInput ? TRUE : FALSE);
}

// Save the current game
void FrotzApp::OnFileSave() 
{
  if (m_lineInput)
    theWnd->InputString("save\r");
}

void FrotzApp::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(m_lineInput ? TRUE : FALSE);
}

void FrotzApp::OnFileStop()
{
  if (istream_replay)
    replay_close();
}

void FrotzApp::OnUpdateFileStop(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(istream_replay ? TRUE : FALSE);
}

void FrotzApp::OnViewOptions()
{
  OptionsDialog options(IDS_OPTIONS,AfxGetMainWnd());
  options.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  // Show the current display settings
  OptionsDisplayPage display;
  options.AddPage(&display);
  display.m_propFontName = m_propFontName;
  display.m_fixedFontName = m_fixedFontName;
  display.m_fontSize.Format("%d",m_fontSize);
  display.m_v6Scale.Format("%d",m_v6scale);
  display.m_textColour.SetCurrentColour(m_defaultFore);
  display.m_backColour.SetCurrentColour(m_defaultBack);
  display.m_fastScroll = m_fastScroll;
  display.m_morePrompts = m_morePrompts;
  display.m_leftMargin = option_left_margin;
  display.m_rightMargin = option_right_margin;

  // Show the current interpreter settings
  OptionsInterpreterPage interpreter;
  options.AddPage(&interpreter);
  interpreter.m_interpreter = h_interpreter_number-1;
  interpreter.m_reportErrors = err_report_mode;
  interpreter.m_ignore = option_ignore_errors ? TRUE : FALSE;
  interpreter.m_expand = option_expand_abbreviations ? TRUE : FALSE;
  interpreter.m_tandy = m_tandy;
  interpreter.m_wrapScript = option_script_cols == 0 ? FALSE : TRUE;
  interpreter.m_username = m_username;

  // Show the current startup settings
  OptionsStartupPage startup;
  options.AddPage(&startup);
  startup.m_register = m_register;
  startup.m_iFiction = m_iFiction;

  // Show the current speech settings
  OptionsSpeechPage speech;
  if (TextToSpeech::GetSpeechEngine().IsAvailable())
    options.AddPage(&speech);
  speech.m_speak = m_speak;
  speech.m_voice = m_speechVoice;
  speech.m_rate = m_speechRate;

  // Show the options dialog
  if (options.DoModal() == IDOK)
  {
    // Get the new font size
    int fontSize = 0;
    sscanf(display.m_fontSize,"%d",&fontSize);
    if (fontSize < 8)
      fontSize = 8;
    if (fontSize > 28)
      fontSize = 28;

    // Check if the font settings have changed
    bool fontUpdate = false;
    if (m_propFontName != display.m_propFontName)
      fontUpdate = true;
    if (m_fixedFontName != display.m_fixedFontName)
      fontUpdate = true;
    if (m_fontSize != fontSize)
      fontUpdate = true;

    // Check if the margin settings have changed
    bool marginUpdate = false;
    if (option_left_margin != display.m_leftMargin)
      marginUpdate = true;
    if (option_right_margin != display.m_rightMargin)
      marginUpdate = true;

    // Update the display settings
    sscanf(display.m_v6Scale,"%d",&m_v6scale);
    m_defaultFore = Make5BitColour(display.m_textColour.GetCurrentColour());
    m_defaultBack = Make5BitColour(display.m_backColour.GetCurrentColour());
    hx_fore_colour = TrueToRGB5(GetDefaultColour(true));
    hx_back_colour = TrueToRGB5(GetDefaultColour(false));
    m_fastScroll = display.m_fastScroll ? true : false;
    m_morePrompts = display.m_morePrompts ? true : false;
    option_left_margin = display.m_leftMargin;
    option_right_margin = display.m_rightMargin;

    // Update the interpreter settings
    h_interpreter_number = interpreter.m_interpreter+1;
    err_report_mode = interpreter.m_reportErrors;
    option_ignore_errors = interpreter.m_ignore ? 1 : 0;
    option_expand_abbreviations = interpreter.m_expand ? 1 : 0;
    m_tandy = interpreter.m_tandy ? true : false;
    option_script_cols = interpreter.m_wrapScript ? 80 : 0;
    m_username = interpreter.m_username.Left(8);
    CopyUsername();

    // Update the startup settings
    m_register = startup.m_register ? true : false;
    m_iFiction = (Show_iFiction)startup.m_iFiction;

    // Update the speech settings
    bool speechUpdate = false;
    if (speech.m_voice != m_speechVoice)
      speechUpdate = true;
    if (speech.m_rate != m_speechRate)
      speechUpdate = true;
    m_speak = (speech.m_speak ? true : false);
    m_speechVoice = speech.m_voice;
    m_speechRate = speech.m_rate;
    if (speechUpdate)
      TextToSpeech::GetSpeechEngine().Update(m_speechVoice,m_speechRate);

    // Update the font settings, if changed
    if (fontUpdate)
    {
      m_propFontName = display.m_propFontName;
      m_fixedFontName = display.m_fixedFontName;
      m_fontSize = fontSize;

      if (theWnd->CreateFonts() == false)
        throw AbortFrotz();

      FrotzWnd::TextSettings savedText = theWnd->GetTextSettings();
      theWnd->ApplyTextSettings(FrotzWnd::TextSettings(0,FIXED_WIDTH_FONT));
      h_font_width = (zbyte)theWnd->GetCharWidth('0');
      h_font_height = (zbyte)theWnd->GetFontHeight();
      theWnd->ApplyTextSettings(savedText);
    }
    if (fontUpdate || marginUpdate)
      theWnd->ResizeDisplay();

    // Update the current foreground and background colours
    if (theWnd->GetTextSettings().foreDefault)
      theWnd->GetTextSettings().fore = theApp.GetDefaultColour(true);
    if (theWnd->GetTextSettings().backDefault)
      theWnd->GetTextSettings().back = theApp.GetDefaultColour(false);
    theWnd->ApplyTextSettings();

    if (h_version == V3)
    {
      if (IsTandyBitSet())
        h_config |= CONFIG_TANDY;
      else
        h_config &= ~CONFIG_TANDY;
    }

    // Write changes back into the game header
    if (zmp != NULL)
    {
      if (h_version == V3)
        SET_BYTE(H_CONFIG,h_config)
      if ((h_version >= V3) && (h_user_name[0] != 0))
      {
        for (int i = 0; i < 8; i++)
          storeb((zword)(H_USER_NAME+i),h_user_name[i]);
      }
      if (h_version >= V4)
        SET_BYTE(H_INTERPRETER_NUMBER,h_interpreter_number)
      if (h_version >= V5)
      {
        set_header_extension(HX_FORE_COLOUR,hx_fore_colour);
        set_header_extension(HX_BACK_COLOUR,hx_back_colour);
      }
    }
  }

  // Update any pending input line
  theWnd->InputType(FrotzWnd::Input::Reset);
}

void FrotzApp::OnViewScrollback()
{
  // Convert the scrollback buffer to ASCII
  int len = m_scrollback.GetSize();
  char* text = new char[len+1];
  ::ZeroMemory(text,len+1);
  if (::WideCharToMultiByte(CP_ACP,0,(LPCWSTR)m_scrollback.GetData(),len,text,len,NULL,NULL) > 0)
  {
    ScrollbackDialog dialog(text);
    dialog.DoModal();
  }
  delete[] text;
}

void FrotzApp::OnUpdateAppAboutGame(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_gameInfo.ifid.IsEmpty() == FALSE);
}

void FrotzApp::OnAppAboutGame()
{
  AboutGameDialog dialog(AfxGetMainWnd());
  dialog.DoModal();
}

void FrotzApp::OnAppAbout()
{
  AboutDialog dialog(AfxGetMainWnd());
  dialog.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// String class, loads string from resources in constructor
/////////////////////////////////////////////////////////////////////////////

CResString::CResString(UINT id)
{
  LoadString(id);
}
