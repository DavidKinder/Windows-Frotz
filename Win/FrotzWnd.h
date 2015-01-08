/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz window class
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Dib.h"
#include "FrotzApp.h"
#include "Resource.h"
#include "TextOutput.h"

class FrotzFrameWnd;
class FrotzGfx;

class FrotzWnd : public CWnd
{
public:
  FrotzWnd();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(FrotzWnd)
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~FrotzWnd();

// Generated message map functions
protected:
  //{{AFX_MSG(FrotzWnd)
  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnDestroy();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:
  // Data for text settings
  struct TextSettings
  {
    TextSettings();
    TextSettings(int s, int f);

    int style;
    int font;
    COLORREF fore;
    COLORREF back;
    bool foreDefault;
    bool backDefault;
    bool backTransparent;
  };

  // Data for input
  struct Input
  {
    enum Type
    {
      ZcodeKey,
      VirtualKey,
      Reset,
      KillLine,
      RuboutWord
    };

    Input();
    Input(Type t, int i, bool m = false, int x = 0, int y = 0);

    Type type;
    unsigned int in;
    bool modify;
    int mousex;
    int mousey;
  };

  enum Timers
  {
    InputTimer = 1,
    SoundTimer,
    StatusTimer
  };

  enum Fonts
  {
    TextFont,
    FixedFont,
    GraphicsFont
  };

public:
  // Initialize the window
  void Initialize(void);
  // Create the window
  bool Create(FrotzFrameWnd* parent);
  // If necessary, create the display bitmap
  bool CreateBitmap(void);
  // Create the display fonts
  bool CreateFonts(void);
  // Create a font
  void CreateFont(CFont& font, LOGFONT& logfont, LONG weight, BYTE italic);

  // Get the current settings
  TextSettings& GetTextSettings(void);
  // Set the display according to the given settings
  void ApplyTextSettings(const TextSettings& settings);
  // Set the display according to current settings
  void ApplyTextSettings(void);

  // Add an input unicode key value
  void InputUnicode(unsigned int c);
  // Add a string of input key press
  void InputString(const char* s);
  // Add an virtual key press
  bool InputVirtualKey(unsigned int c);
  // Add an input Z-code key press
  void InputZcodeKey(unsigned int c);
  // Add an input mouse press
  void InputMouse(bool dblclick, POINT p);
  // Add an input menu selection
  void InputMenu(unsigned short menu);
  // Add an input type to the input queue
  void InputType(Input::Type type);
  // Add input from the input history
  bool InputFromHistory(int history);

  // Handle a mouse click
  void MouseClick(CPoint point);
  // Get the next input
  bool GetNextInput(Input& input);
  // Wait for input
  void WaitForInput(void);
  // Draw the cursor
  void DrawCursor(bool on);
  // Draw the current input line
  void DrawInput(unsigned short* buffer, int pos,
    const CPoint& point, int width, bool cursor);
  // Store an input line in the history
  void AddToInputHistory(unsigned short* buffer);
  // Store the last input line
  void SetLastInput(unsigned short* buffer);
  // Attempt to correct the case of an input line
  void RecaseInput(unsigned short* buffer);

  // Add an output character to the pending text
  void AddOutput(unsigned short c, bool status);
  // Flush any pending text
  void FlushText(void);
  // Flush any display changes
  void FlushDisplay(void);
  // Clear the display
  void ClearDisplay(void);

  // Get the output text position
  CPoint GetTextPoint(void);
  // Move the output text position
  void SetTextPoint(POINT point);

  // Scroll the bitmap
  void Scroll(int left, int top, int width, int height, int units);
  // Fill a rectangle with the background colour
  void FillBackground(LPCRECT rect);
  // Fill a rectangle with the given colour
  void FillSolid(LPCRECT rect, COLORREF colour);
  // Draw a bitmap graphic
  void DrawGraphic(FrotzGfx* gfx, int x, int y, double r);
  // Get the colour of a pixel
  COLORREF GetPixel(POINT p);

  // Write an ASCII string
  void WriteText(const char* text);
  // Write a Unicode string
  void WriteText(const unsigned short* text, int len);
  // Get the width of a Unicode string
  int GetTextWidth(const unsigned short* text, int len);
  // Get the width of a Unicode character
  int GetCharWidth(unsigned short c);
  // Get the height of the font
  int GetFontHeight(void);
  // Check if a character has a glyph
  bool HasGlyph(int font, unsigned short c);
  // Get the overhang for a character, such as is seen on italic fonts
  int GetOverhang(unsigned short c);
  // Reset the current character overhang
  void ResetOverhang(void);

  // Create the graphics font bitmap
  bool CreateGfxBitmap(void);
  // Write a graphics font symbol
  void WriteGfxSymbol(short symbol);

  // Add a new menu
  void AddNewMenu(int menu, const unsigned short* text);
  // Add a menu item to the bottom of an existing menu
  void AddMenuItem(int menu, const unsigned short* text);
  // Remove an existing menu
  void RemoveMenu(int menu);
  // Update the menus, if needed
  void UpdateMenus(void);
  // Get the code for the last clicked menu
  unsigned short GetMenuClick(void);

  // Set if resizing is allowed
  void SetAllowResize(bool allow);
  // Resize the display and redraw
  void ResizeDisplay(void);
  // Calculate the ERF for scaling pictures
  double CalcScalingERF(void);

  // Wrapper for showing and removing the cursor
  class DrawCursor
  {
  public:
    DrawCursor(FrotzWnd* wnd, bool visible);
    ~DrawCursor();

  protected:
    FrotzWnd* m_wnd;
    bool m_visible;
  };

protected:
  class UnicodeString : public CArray<unsigned short,unsigned short>
  {
  public:
    UnicodeString() {};
    UnicodeString(const unsigned short* str);
    UnicodeString(const UnicodeString& str);
    UnicodeString& operator=(const UnicodeString& str);

    // Convert to an ASCII string
    CString ToAscii(void);
  };

  // Correct the case of part of a string
  int RecaseString(unsigned short* buffer, UnicodeString& match);
  // Compare two Unicode strings
  bool CompareUnicode(unsigned short* s1, unsigned short* s2, int len);

  // Select a font into the device context
  void SelectFont(CFont& font);
  // Get the current background colour
  COLORREF GetBackColour(void);

protected:
  CDC m_dc;
  CDibSection m_bitmap;

  CSize m_wndSize;
  bool m_allowResize;

  CFont m_fontText;
  CFont m_fontTextBold;
  CFont m_fontTextItalic;
  CFont m_fontTextBoldItalic;
  CFont m_fontFixed;
  CFont m_fontFixedBold;
  CFont m_fontFixedItalic;
  CFont m_fontFixedBoldItalic;

  CSize m_fontSize;
  Fonts m_fontType;
  TextOutput m_textOut;

  CDC m_gfxDc;
  CDC m_charDc;
  CDC m_cursorDc;
  CBitmap m_gfxBitmap;
  CBitmap m_charBitmap;
  CDibSection m_cursorBitmap;

  TextSettings m_current;
  UnicodeString m_pendingText;

  CList<Input,const Input&> m_input;
  DWORD m_mouseClick;
  unsigned short m_lastMenu;

  CList<UnicodeString,UnicodeString&> m_inputHistory;
  static int m_historyLimit;

  UnicodeString m_lastOutput;
  UnicodeString m_lastInput;
  int m_lastOver;

  CArray<CStringArray,CStringArray&> m_menus;
  bool m_buildMenus;
};
