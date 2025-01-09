/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz application class
/////////////////////////////////////////////////////////////////////////////

#pragma once

extern "C"
{
typedef int f_bool;
#define bool f_bool
#include "frotz.h"
#undef bool

#include "blorb.h"
}

#define NON_STD_COLS 238
#define PREFS_VERSION 1

class FrotzApp : public CWinApp
{
public:
  FrotzApp();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(FrotzApp)
  public:
  virtual BOOL InitInstance();
  virtual int ExitInstance();
  //}}AFX_VIRTUAL

// Implementation

  //{{AFX_MSG(FrotzApp)
  afx_msg void OnFileNew();
  afx_msg void OnFileOpen();
  afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
  afx_msg void OnFileSave();
  afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
  afx_msg void OnFileStop();
  afx_msg void OnUpdateFileStop(CCmdUI *pCmdUI);
  afx_msg void OnViewOptions();
  afx_msg void OnViewScrollback();
  afx_msg void OnUpdateAppAboutGame(CCmdUI *pCmdUI);
  afx_msg void OnAppAboutGame();
  afx_msg void OnAppAbout();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:
  // Exception thrown to restart the interpreter
  class RestartFrotz
  {
  };

  // Enumeration for return values from RunInterpreter()
  enum ExitStatus
  {
    Exited,
    Restarting
  };

  // Initialize the application
  void Initialize(void);
  // Run the interpreter core
  ExitStatus RunInterpreter(void);

  // Read in settings
  void ReadSettings(void);
  // Write out settings
  void WriteSettings(void);
  // Get the default font
  CString GetDefaultFont(void);
  // Get the default fixed width font
  CString GetDefaultFixedFont(void);
  // Get the display settings
  void GetDisplaySettings(CString& propName, CString& fixedName, int& size);
  // Register file types
  void RegisterFileTypes(void);
  // Load an international version of resources
  void LoadInternationalResources(void);
  // Get a link URL from the international resources
  LPCSTR GetInternationalLink(UINT id);

  // Open a file dialog to prompt the user for a game
  bool PromptForGame(bool initial);
  // Get the filename of the game to run
  LPCTSTR GetGameFileName(void);
  // Create the main window
  void CreateMainWindow(void);
  // Store the size of the main window
  void StoreWindowSize(void);
  // Store the state of the control bars
  void StoreBarState(bool toolbar, bool statusbar);
  // Process messages
  void MessagePump(void);
  // Get the elapsed time for this game
  CTimeSpan GetElapsedTime(void);
  // Check if a character is printable
  bool IsValidChar(unsigned short c);
  // Get the size of the screen
  CRect GetScreenSize(bool full);

  // Get a default colour
  COLORREF GetDefaultColour(bool fore);
  // Get a colour
  COLORREF GetColour(int colour);
  // Get an index for a non-standard colour
  int GetColourIndex(COLORREF colour);
  // Adjust a foreground colour for the given style
  COLORREF AdjustForeColour(COLORREF colour, int style);
  // Convert from 5-bit RGB to a true colour
  static COLORREF RGB5ToTrue(unsigned short five);
  // Convert from a true colour to 5-bit RGB
  static unsigned short TrueToRGB5(COLORREF colour);
  // Rescale colour components so that the colour can be expressed
  // exactly in 5-bit RGB
  static COLORREF Make5BitColour(COLORREF colour);

  // Get whether to pause on exit
  bool GetExitPause(void);
  // Set whether to pause on exit
  void SetExitPause(bool pause);

  // Get whether to use the Quetzal save format
  bool GetUseQuetzal(void);
  // Set whether to use the Quetzal save format
  void SetUseQuetzal(bool quetzal);

  // Get whether to notify when entering full screen mode
  bool GetNotifyFullScreen(void);
  // Set whether to notify when entering full screen mode
  void SetNotifyFullScreen(bool notify);

  // Get whether to use fast or slow scrolling
  bool GetFastScrolling(void);
  // Get whether to show [More] prompts
  bool GetShowMorePrompts(void);

  // Set whether line input is in progress
  void SetLineInput(bool input);

  // Set up the Blorb resource file
  void SetBlorbFile(void);
  // Set up the Blorb resource file at the Z-code game
  void SetBlorbZCode(FILE* file, long* size);
  // Close the Blorb resource file
  void CloseBlorbFile(void);
  // Returns true if a Blorb resource file has been loaded
  bool GotBlorbFile(void);
  // Returns the Blorb map
  bb_map_t* GetBlorbMap(void);

  struct GameInfo
  {
    GameInfo() : cover(-1), coverFormatWrong(false)
    {
    }

    int cover;
    CString ifid;
    CString title;
    CString headline;
    CString description;
    CString author;
    CString year;
    CString series;
    CString seriesNumber;

    bool coverFormatWrong;
  };

  // Load IF metadata
  void LoadMetadata(void);
  // Check for an already known game
  bool CheckGameId(void);
  // Get the iFiction metadata
  const GameInfo& GetGameInfo(void);

  // If true, running one of Infocom's V6 games
  bool IsInfocomV6(void);
  // If true, the Tandy header flag should be set
  bool IsTandyBitSet(void);
  // Copy the user name into the interpreter fields
  void CopyUsername(void);

  // Add a character to the scrollback buffer
  void ScrollbackChar(unsigned short c);
  // Remove characters from the scrollback buffer
  void ScrollbackRemove(int remove);
  // Speak text added to the scrollback buffer
  void SpeakText(void);

  // Check if the interpreter wants to restart
  void CheckRestart(void);

protected:
  // Get a string from an XML node
  CString StrFromXML(IXMLDOMDocument* doc, LPCWSTR path);

  HINSTANCE m_translate;

  CString m_filename;
  bool m_register;
  bool m_tandy;
  bool m_quetzal;
  CString m_username;

  CRect m_wndSize;
  int m_wndState;
  bool m_toolBar;
  bool m_statusBar;
  bool m_notifyFull;
  CTime m_startTime;

  CString m_propFontName;
  CString m_fixedFontName;
  int m_fontSize;

  COLORREF m_defaultFore;
  COLORREF m_defaultBack;
  COLORREF m_colours[11];
  COLORREF m_nonStdColours[NON_STD_COLS];
  int m_nonStdIndex;

  bool m_exitPause;
  bool m_lineInput;

  bool m_fastScroll;
  bool m_morePrompts;

  FILE* m_blorbFile;
  bb_map_t* m_blorbMap;

  enum Show_iFiction
  {
    Show_iF_Never = 0,
    Show_iF_First_Time = 1,
    Show_iF_Always = 2
  };
  Show_iFiction m_iFiction;
  GameInfo m_gameInfo;

  CArray<unsigned short,unsigned short> m_scrollback;

  bool m_speak;
  CString m_speechVoice;
  int m_speechRate;
  int m_spokenIndex;

  bool m_wantRestart;
};

class CResString : public CString
{
public:
  CResString(UINT id);
};

class CResStringW : public CStringW
{
public:
  CResStringW(UINT id);
};
