/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz window class
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FrotzApp.h"
#include "FrotzFrameWnd.h"
#include "FrotzGfx.h"
#include "FrotzSound.h"
#include "FrotzWnd.h"

extern "C"
{
void resize_screen(void);
void restart_header(void);
zword unicode_tolower(zword c);
void get_window_colours(zword win, zbyte* fore, zbyte* back);
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int FrotzWnd::m_historyLimit = 20;

FrotzWnd::FrotzWnd() : m_fontSize(0,0)
{
  m_fontType = TextFont;
  Initialize();
}

FrotzWnd::~FrotzWnd()
{
  m_bitmap.DeleteBitmap();
  m_cursorBitmap.DeleteBitmap();
}

BEGIN_MESSAGE_MAP(FrotzWnd, CWnd)
  //{{AFX_MSG_MAP(FrotzWnd)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
  ON_WM_SIZE()
  ON_WM_TIMER()
  ON_WM_LBUTTONDOWN()
  ON_WM_MBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  //}}AFX_MSG_MAP
  ON_MESSAGE(WM_SOUND_NOTIFY, OnSoundNotify)
END_MESSAGE_MAP()

// Initialize the window
void FrotzWnd::Initialize(void)
{
  m_allowResize = true;
  m_mapColorChanger.RemoveAll();
  m_pendingText.RemoveAll();
  m_input.RemoveAll();
  m_mouseClick = 0;
  m_lastMenu = 0;
  m_inputHistory.RemoveAll();
  m_lastOutput.SetSize(0,256);
  m_lastInput.SetSize(0);
  m_lastOver = 0;
  m_menus.RemoveAll();
  m_buildMenus = false;
}

bool FrotzWnd::Create(FrotzFrameWnd* parent, int dpi)
{
  CRect size;
  parent->GetClientRect(size);
  m_wndSize = size.Size();

  // Create the window
  LPCTSTR wndClass = AfxRegisterWndClass(CS_OWNDC,::LoadCursor(NULL,IDC_ARROW));
  if (!CreateEx(WS_EX_CLIENTEDGE,wndClass,NULL,WS_CHILD|WS_VISIBLE,
    size,parent,AFX_IDW_PANE_FIRST,NULL))
    return false;

  // Create the display bitmap and fonts
  if (CreateBitmap(CSize(0,0)) == false)
    return false;
  if (CreateFonts(dpi) == false)
    return false;
  return true;
}

// Create the display bitmap
bool FrotzWnd::CreateBitmap(CSize size)
{
  if (m_dc.GetSafeHdc() == 0)
  {
    // Create the device context
    CDC* dc = GetDC();
    m_dc.CreateCompatibleDC(dc);
    ReleaseDC(dc);
    if (m_dc.GetSafeHdc() == 0)
      return false;
    m_dc.SetBkMode(TRANSPARENT);
    m_dc.SetTextAlign(TA_TOP|TA_UPDATECP);
  }

  // Make the size at least as big as the screen, plus a bit
  FrotzApp* app = (FrotzApp*)AfxGetApp();
  CRect screen = app->GetScreenSize(true);
  if (screen.Width()+8 > size.cx)
    size.cx = screen.Width()+8;
  if (screen.Height()+8 > size.cy)
    size.cy = screen.Height()+8;

  // Make the bitmap slightly larger than the display
  m_bitmap.DeleteBitmap();
  if (!m_bitmap.CreateBitmap(m_dc,size.cx,size.cy))
    return false;

  // Initialize the bitmap
  CDibSection::SelectDibSection(m_dc,&m_bitmap);
  m_dc.FillSolidRect(0,0,size.cx,size.cy,app->GetDefaultColour(false));
  return true;
}

// Create the display fonts
bool FrotzWnd::CreateFonts(int dpi)
{
  // Set up a LOGFONT structure
  LOGFONT font;
  ::ZeroMemory(&font,sizeof(LOGFONT));
  font.lfCharSet = ANSI_CHARSET;
  font.lfOutPrecision = OUT_TT_ONLY_PRECIS;
  font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  font.lfQuality = PROOF_QUALITY;
  font.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;

  // Get the font size and names
  CString propFont, fixedFont;
  int fontSize;
  ((FrotzApp*)AfxGetApp())->GetDisplaySettings(propFont,fixedFont,fontSize);

  // If the proportional fonts are already allocated, one of them may be
  // selected into the device context, so select the fixed font before
  // deleting the proportional fonts.
  if (m_fontText.GetSafeHandle())
  {
    SelectFont(m_fontFixed);

    m_fontText.DeleteObject();
    m_fontTextBold.DeleteObject();
    m_fontTextItalic.DeleteObject();
    m_fontTextBoldItalic.DeleteObject();
  }

  // Create the proportional fonts
  strncpy(font.lfFaceName,propFont,LF_FACESIZE);
  font.lfHeight = -MulDiv(fontSize,dpi,72);
  CreateFont(m_fontText,font,FW_NORMAL,FALSE);
  CreateFont(m_fontTextBold,font,FW_BOLD,FALSE);
  CreateFont(m_fontTextItalic,font,FW_NORMAL,TRUE);
  CreateFont(m_fontTextBoldItalic,font,FW_BOLD,TRUE);

  TEXTMETRIC metrics;
  SelectFont(m_fontText);
  m_dc.GetTextMetrics(&metrics);
  m_fontSize.cy = metrics.tmHeight;

  // Since the proportional font is now selected into the device context, it is
  // safe to delete the fixed fonts if they have already been allocated.
  if (m_fontFixed.GetSafeHandle())
  {
    m_fontFixed.DeleteObject();
    m_fontFixedBold.DeleteObject();
    m_fontFixedItalic.DeleteObject();
    m_fontFixedBoldItalic.DeleteObject();
  }

  // Create the fixed width fonts
  strncpy(font.lfFaceName,fixedFont,LF_FACESIZE);
  CreateFont(m_fontFixed,font,FW_NORMAL,FALSE);
  CreateFont(m_fontFixedBold,font,FW_BOLD,FALSE);
  CreateFont(m_fontFixedItalic,font,FW_NORMAL,TRUE);
  CreateFont(m_fontFixedBoldItalic,font,FW_BOLD,TRUE);

  SelectFont(m_fontFixed);
  m_fontSize.cx = GetCharWidth('0');
  m_fontSize.cy = max(metrics.tmHeight,m_fontSize.cy);

  // Create the graphics font bitmap
  if (CreateGfxBitmap() == false)
    return false;

  // Discard any cached linked fonts
  m_textOut.Reset();
  return true;
}

// Create a font
void FrotzWnd::CreateFont(CFont& font, LOGFONT& logfont, LONG weight, BYTE italic)
{
  logfont.lfWeight = weight;
  logfont.lfItalic = italic;
  font.CreateFontIndirect(&logfont);
}

// Get the current settings
FrotzWnd::TextSettings& FrotzWnd::GetTextSettings(void)
{
  return m_current;
}

// Set the display according to the given settings
void FrotzWnd::ApplyTextSettings(const TextSettings& settings)
{
  m_current = settings;
  ApplyTextSettings();
}

// Set the display according to current settings
void FrotzWnd::ApplyTextSettings(void)
{
  FrotzApp* app = (FrotzApp*)AfxGetApp();

  m_fontType = TextFont;
  if (m_current.font == GRAPHICS_FONT)
  {
    SelectFont(m_fontFixed);
    m_fontType = GraphicsFont;
  }
  else if ((m_current.font == FIXED_WIDTH_FONT) || (m_current.style & FIXED_WIDTH_STYLE))
  {
    switch (m_current.style & (BOLDFACE_STYLE|EMPHASIS_STYLE))
    {
    case BOLDFACE_STYLE:
      SelectFont(m_fontFixedBold);
      break;
    case EMPHASIS_STYLE:
      SelectFont(m_fontFixedItalic);
      break;
    case BOLDFACE_STYLE|EMPHASIS_STYLE:
      SelectFont(m_fontFixedBoldItalic);
      break;
    default:
      SelectFont(m_fontFixed);
      break;
    }
    m_fontType = FixedFont;
  }
  else
  {
    switch (m_current.style & (BOLDFACE_STYLE|EMPHASIS_STYLE))
    {
    case BOLDFACE_STYLE:
      SelectFont(m_fontTextBold);
      break;
    case EMPHASIS_STYLE:
      SelectFont(m_fontTextItalic);
      break;
    case BOLDFACE_STYLE|EMPHASIS_STYLE:
      SelectFont(m_fontTextBoldItalic);
      break;
    default:
      SelectFont(m_fontText);
      break;
    }
  }

  if (m_current.style & REVERSE_STYLE)
  {
    m_dc.SetTextColor(m_current.back);
    m_dc.SetBkColor(m_current.fore);
  }
  else
  {
    m_dc.SetTextColor(app->AdjustForeColour(m_current.fore,m_current.style));
    m_dc.SetBkColor(m_current.back);
  }
}

// Add an input unicode key value
void FrotzWnd::InputUnicode(unsigned int c)
{
  switch (c)
  {
  case 8:
    m_input.AddTail(Input(Input::ZcodeKey,ZC_BACKSPACE));
    break;
  case 13:
    m_input.AddTail(Input(Input::ZcodeKey,ZC_RETURN));
    break;
  case 27:
    m_input.AddTail(Input(Input::ZcodeKey,ZC_ESCAPE));
    break;
  }
  if (((FrotzApp*)AfxGetApp())->IsValidChar((unsigned short)c))
    m_input.AddTail(Input(Input::ZcodeKey,c));
}

// Add a string of input key press
void FrotzWnd::InputString(const char* s)
{
  while (*s != '\0')
    InputUnicode(*s++);
}

// Add an virtual key press
bool FrotzWnd::InputVirtualKey(unsigned int c)
{
  // Is the Ctrl key down?
  bool ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
  switch (c)
  {
  case VK_UP:
    m_input.AddTail(Input(Input::ZcodeKey,ZC_ARROW_UP,ctrl));
    return true;
  case VK_DOWN:
    m_input.AddTail(Input(Input::ZcodeKey,ZC_ARROW_DOWN,ctrl));
    return true;
  case VK_LEFT:
    m_input.AddTail(Input(Input::ZcodeKey,ZC_ARROW_LEFT,ctrl));
    return true;
  case VK_RIGHT:
    m_input.AddTail(Input(Input::ZcodeKey,ZC_ARROW_RIGHT,ctrl));
    return true;
  case VK_DELETE:
  case VK_HOME:
  case VK_END:
  case VK_TAB:
    m_input.AddTail(Input(Input::VirtualKey,c));
    return true;
  case VK_CANCEL: // Break (Ctrl+Pause)
    TextToSpeech::GetSpeechEngine().Destroy();
    return false;
  }

  // Function keys
  if ((c >= VK_F1) && (c <= VK_F12))
  {
    m_input.AddTail(Input(Input::ZcodeKey,c-VK_F1+ZC_FKEY_MIN));
    return true;
  }

  // Numeric keypad
  if ((c >= VK_NUMPAD0) && (c <= VK_NUMPAD9))
  {
    m_input.AddTail(Input(Input::ZcodeKey,c-VK_NUMPAD0+ZC_NUMPAD_MIN));
    return true;
  }
  return false;
}

// Add an input Z-code key press
void FrotzWnd::InputZcodeKey(unsigned int c)
{
  m_input.AddTail(Input(Input::ZcodeKey,c));
}

// Add an input mouse press
void FrotzWnd::InputMouse(bool dblclick, POINT p)
{
  // Double clicks are only recognized for V6
  if (h_version != V6)
    dblclick = false;

  m_input.AddTail(Input(Input::ZcodeKey,dblclick ?
    ZC_DOUBLE_CLICK : ZC_SINGLE_CLICK,false,p.x,p.y));
}

// Add an input menu selection
void FrotzWnd::InputMenu(unsigned short menu)
{
  m_lastMenu = menu;
  InputZcodeKey(ZC_MENU_CLICK);
}

// Add an input type to the input queue
void FrotzWnd::InputType(Input::Type type)
{
  m_input.AddTail(Input(type,0));
}

// Add input from the input history
bool FrotzWnd::InputFromHistory(int history)
{
  POSITION pos = m_inputHistory.FindIndex(history);
  if (pos != NULL)
  {
    UnicodeString& str = m_inputHistory.GetAt(pos);
    for (int i = 0; i < str.GetSize(); i++)
      InputUnicode(str.GetAt(i));
    return true;
  }
  return false;
}

// Handle a mouse click
void FrotzWnd::MouseClick(CPoint point)
{
  if (m_mouseClick + ::GetDoubleClickTime() > ::GetTickCount())
    InputMouse(true,point);
  else
    InputMouse(false,point);
  m_mouseClick = ::GetTickCount();
}

// Get the next input
bool FrotzWnd::GetNextInput(Input& input)
{
  if (m_input.IsEmpty())
    return false;
  input = m_input.RemoveHead();
  return true;
}

// Wait for input
void FrotzWnd::WaitForInput(void)
{
  while (m_input.IsEmpty())
    ((FrotzApp*)AfxGetApp())->MessagePump();
}

// Draw the cursor
void FrotzWnd::DrawCursor(bool on)
{
  CPoint p = GetTextPoint();
  int w = GetCharWidth('0');
  int h = GetFontHeight();

  if (on)
  {
    // Save what is under the cursor
    m_cursorDc.BitBlt(0,0,w,h,&m_dc,p.x,p.y,SRCCOPY);

    // Draw the cursor
    FillSolid(CRect(p.x,p.y,p.x+w,p.y+h),m_current.fore);
  }
  else
  {
    // Restore what was under the cursor
    m_dc.BitBlt(p.x,p.y,w,h,&m_cursorDc,0,0,SRCCOPY);
  }
}

// Draw the current input line
void FrotzWnd::DrawInput(unsigned short* buffer, int pos, const CPoint& point, int width, bool cursor)
{
  int height = GetFontHeight();

  // Remove any previous input
  m_lastInputSize = CSize(width,height);
  FillBackground(CRect(point.x,point.y,point.x+width,point.y+height));

  // Display the input
  SetTextPoint(point);
  WriteText(buffer,wcslen((LPCWSTR)buffer));

  if (cursor)
  {
    int x = point.x + GetTextWidth(buffer,pos);
    int cx = GetCharWidth('0');
    if (*(buffer+pos) != 0)
      cx = GetCharWidth(*(buffer+pos));

    // Invert colours
    COLORREF fore = m_dc.GetTextColor();
    m_dc.SetTextColor(m_dc.GetBkColor());
    m_dc.SetBkColor(fore);

    // Draw a cursor
    m_dc.MoveTo(x,point.y);
    CRect rect(x,point.y,x+cx,point.y+height);
    if (*(buffer+pos) != 0)
    {
      if (m_fontType == TextFont)
        m_textOut.TextOut(m_dc.GetSafeHdc(),0,0,(LPCWSTR)buffer+pos,1,rect,true);
      else
        ::ExtTextOutW(m_dc.GetSafeHdc(),0,0,ETO_OPAQUE,rect,(LPCWSTR)buffer+pos,1,NULL);
    }
    else
      ::ExtTextOutW(m_dc.GetSafeHdc(),0,0,ETO_OPAQUE,rect,NULL,0,NULL);

    // Put colours back
    m_dc.SetBkColor(m_dc.GetTextColor());
    m_dc.SetTextColor(fore);
  }

  // Update the window
  Invalidate();
}

// Erase the last input rectangle
void FrotzWnd::EraseLastInputRect(const CPoint& point)
{
  FillBackground(CRect(point,m_lastInputSize));
}

// Store an input line in the history
void FrotzWnd::AddToInputHistory(unsigned short* buffer)
{
  if (*buffer != 0)
  {
    // Add the new entry to the history
    m_inputHistory.AddHead(UnicodeString(buffer));

    // If the input history is full, remove the oldest entry
    while (m_inputHistory.GetCount() > m_historyLimit)
      m_inputHistory.RemoveTail();
  }
}

// Store the last input line
void FrotzWnd::SetLastInput(unsigned short* buffer)
{
  int len = wcslen((LPCWSTR)buffer);
  m_lastInput.SetSize(0,len);
  for (int i = 0; i < len; i++)
    m_lastInput.Add(buffer[i]);
}

// Attempt to correct the case of an input line
void FrotzWnd::RecaseInput(unsigned short* buffer)
{
  int pos = 0;
  pos += RecaseString(buffer+pos,m_lastInput);
  pos += RecaseString(buffer+pos,m_lastOutput);
}

// Correct the case of part of a string
int FrotzWnd::RecaseString(unsigned short* buffer, UnicodeString& match)
{
  int len = match.GetSize();
  if ((int)wcslen((LPCWSTR)buffer) >= len)
  {
    if (CompareUnicode(buffer,match.GetData(),len))
    {
      memcpy(buffer,match.GetData(),sizeof(unsigned short) * len);
      return len;
    }
  }
  return 0;
}

// Compare two Unicode strings
bool FrotzWnd::CompareUnicode(unsigned short* s1, unsigned short* s2, int len)
{
  for (int i = 0; i < len; i++)
  {
    if (unicode_tolower(s1[i]) != unicode_tolower(s2[i]))
      return false;
    if (s1[i] == 0)
      return true;
  }
  return true;
}

// Add an output character to the pending text
void FrotzWnd::AddOutput(unsigned short c, bool status)
{
  m_pendingText.Add(c);
}

// Flush any pending text
void FrotzWnd::FlushText(void)
{
  if (m_pendingText.GetSize() > 0)
  {
    // Write the output string to the bitmap
    int len = m_pendingText.GetSize();
    WriteText(m_pendingText.GetData(),len);

    // Store the output string
    m_lastOutput.SetSize(0,len);
    for (int i = 0; i < len; i++)
      m_lastOutput.Add(m_pendingText[i]);

    m_pendingText.RemoveAll();
  }
}

// Flush any display changes
void FrotzWnd::FlushDisplay(void)
{
  FlushText();
  Invalidate();
  ((FrotzApp*)AfxGetApp())->MessagePump();
}

// Clear the display
void FrotzWnd::ClearDisplay(void)
{
  FrotzApp* app = (FrotzApp*)AfxGetApp();

  m_dc.FillSolidRect(CRect(CPoint(0,0),m_bitmap.GetSize()),
    app->GetDefaultColour(false));
  Invalidate();
  app->MessagePump();
}

// Get the output text position
CPoint FrotzWnd::GetTextPoint(void)
{
  return m_dc.GetCurrentPosition();
}

// Set the output text position
void FrotzWnd::SetTextPoint(POINT point)
{
  m_dc.MoveTo(point);
}

// Scroll the bitmap
void FrotzWnd::Scroll(LPCRECT rect, int units)
{
  m_dc.BitBlt(rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,
    &m_dc,rect->left,rect->top+units,SRCCOPY);

  PurgeColorChangers(rect);
}

// Fill a rectangle with the background colour
void FrotzWnd::FillBackground(LPCRECT rect)
{
  m_dc.ExtTextOut(0,0,ETO_OPAQUE,rect,NULL,0,NULL);

  PurgeColorChangers(rect);
}

// Fill a rectangle with the given colour
void FrotzWnd::FillSolid(LPCRECT rect, COLORREF colour)
{
  COLORREF back = m_dc.SetBkColor(colour);
  m_dc.ExtTextOut(0,0,ETO_OPAQUE,rect,NULL,0,NULL);
  m_dc.SetBkColor(back);
}

// Draw a bitmap graphic
void FrotzWnd::DrawGraphic(FrotzGfx* gfx, CPoint point)
{
  double erf = CalcScalingERF();

  if (gfx->ApplyPalette())
    RedrawColorChangers(erf);

  gfx->Paint(m_bitmap,point,erf);

  if (gfx->IsColorChanger())
    m_mapColorChanger[gfx] = point;
}

// Get the size of the bitmap graphic after scaling
CSize FrotzWnd::GetGraphicSize(FrotzGfx* gfx)
{
  double erf = CalcScalingERF();
  return gfx->GetSize(erf);
}

// Get the colour of a pixel
COLORREF FrotzWnd::GetPixel(POINT p)
{
  return m_dc.GetPixel(p);
}

// Write an ASCII string
void FrotzWnd::WriteText(const char* text)
{
  int len = strlen(text);
  CSize size = m_dc.GetTextExtent(text,len);
  size.cy = m_fontSize.cy;

  CRect rect(GetTextPoint(),size);
  m_dc.ExtTextOut(0,0,ETO_OPAQUE,rect,text,len,NULL);
}

// Write a Unicode string
void FrotzWnd::WriteText(const unsigned short* text, int len)
{
  if (len == 0)
    return;

  switch (m_fontType)
  {
  case TextFont:
    {
      CSize size = m_textOut.GetTextExtent(m_dc.GetSafeHdc(),(LPCWSTR)text,len);
      size.cy = m_fontSize.cy;

      // Get the text rectangle, and adjust for any overhang (e.g. an italic font)
      CRect rect(GetTextPoint(),size);
      rect.left += m_lastOver;
      m_lastOver = GetOverhang(text[len-1]);
      rect.right += m_lastOver;

      m_textOut.TextOut(m_dc.GetSafeHdc(),0,0,
        (LPCWSTR)text,len,rect,!m_current.backTransparent);
    }
    break;
  case FixedFont:
    {
      CSize size(0,0);
      ::GetTextExtentPoint32W(m_dc.GetSafeHdc(),(LPCWSTR)text,len,&size);
      size.cy = m_fontSize.cy;

      CRect rect(GetTextPoint(),size);
      ::ExtTextOutW(m_dc.GetSafeHdc(),0,0,
        m_current.backTransparent ? 0 : ETO_OPAQUE,rect,(LPCWSTR)text,len,NULL);
    }
    break;
  case GraphicsFont:
    for (int i = 0; i < len; i++)
      WriteGfxSymbol(text[i]);
    break;
  }
}

// Get the width of a Unicode string
int FrotzWnd::GetTextWidth(const unsigned short* text, int len)
{
  if (len > 0)
  {
    switch (m_fontType)
    {
    case TextFont:
      return m_textOut.GetTextExtent(m_dc.GetSafeHdc(),(LPCWSTR)text,len).cx;
    case FixedFont:
      {
        CSize size(0,0);
        if (::GetTextExtentPoint32W(m_dc.GetSafeHdc(),(LPCWSTR)text,len,&size))
          return size.cx;
      }
      break;
    }
  }
  return 0;
}

// Get the width of a Unicode character
int FrotzWnd::GetCharWidth(unsigned short c)
{
  return GetTextWidth(&c,1);
}

// Get the height of the font
int FrotzWnd::GetFontHeight(void)
{
  return m_fontSize.cy;
}

// Check if a character has a glyph
bool FrotzWnd::HasGlyph(int font, unsigned short c)
{
  bool hasGlyph = false;
  switch (font)
  {
  case TEXT_FONT:
    {
      CFont* prevFont = m_dc.SelectObject(&m_fontText);
      hasGlyph = m_textOut.CanOutput(m_dc.GetSafeHdc(),c);
      m_dc.SelectObject(prevFont);
    }
    break;
  case FIXED_WIDTH_FONT:
    {
      CFont* prevFont = m_dc.SelectObject(&m_fontFixed);
      WORD idx[1] = { 0xFFFF };
      WCHAR wc = c;
      if (::GetGlyphIndicesW(m_dc.GetSafeHdc(),&wc,1,idx,GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR)
        hasGlyph = (idx[0] != 0xFFFF);
      m_dc.SelectObject(prevFont);
    }
    break;
  case GRAPHICS_FONT:
    hasGlyph = ((c >= 32) && (c < 128));
    break;
  }
  return hasGlyph;
}

// Get the overhang for a character, such as is seen on italic fonts
int FrotzWnd::GetOverhang(unsigned short c)
{
  ABCFLOAT abc;
  if (m_textOut.GetCharABCWidth(m_dc.GetSafeHdc(),c,abc))
  {
    if (abc.abcfC < 0)
      return (int)(-1.0*abc.abcfC);
  }
  return 0;
}

// Reset the current character overhang
void FrotzWnd::ResetOverhang(void)
{
  m_lastOver = 0;
}

// Create the graphics font bitmap
bool FrotzWnd::CreateGfxBitmap(void)
{
  if (m_gfxDc.GetSafeHdc() == 0)
  {
    // Create the device contexts
    CDC* dc = GetDC();
    m_gfxDc.CreateCompatibleDC(dc);
    m_charDc.CreateCompatibleDC(dc);
    m_cursorDc.CreateCompatibleDC(dc);
    ReleaseDC(dc);
    if ((m_gfxDc.GetSafeHdc() == 0) || (m_charDc.GetSafeHdc() == 0))
      return false;
    if (m_cursorDc.GetSafeHdc() == 0)
      return false;

    // Load the bitmap
    if (!m_gfxBitmap.Attach(::LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_GFXFONT))))
      return false;
    m_gfxDc.SelectObject(m_gfxBitmap);
  }

  // Create a character monochrome bitmap
  ::DeleteObject(m_charBitmap.Detach());
  if (!m_charBitmap.CreateBitmap(m_fontSize.cx,m_fontSize.cy,1,1,NULL))
    return false;
  m_charDc.SelectObject(m_charBitmap);
  m_charDc.SetStretchBltMode(BLACKONWHITE);

  // Create a cursor background bitmap
  m_cursorBitmap.DeleteBitmap();
  if (!m_cursorBitmap.CreateBitmap(m_cursorDc,m_fontSize.cx+8,m_fontSize.cy+8))
    return false;
  CDibSection::SelectDibSection(m_cursorDc,&m_cursorBitmap);
  return true;
}

// Write a graphics font symbol
void FrotzWnd::WriteGfxSymbol(short symbol)
{
  CPoint pos = m_dc.GetCurrentPosition();
  if ((symbol >= 32) && (symbol < 128))
  {
    int y = (symbol-32) / 32;
    int x = (symbol-32) - (y*32);
    m_charDc.StretchBlt(0,0,m_fontSize.cx,m_fontSize.cy,
      &m_gfxDc,x*8,y*8,8,8,SRCCOPY);
    m_dc.BitBlt(pos.x,pos.y,m_fontSize.cx,m_fontSize.cy,
      &m_charDc,0,0,SRCCOPY);
  }
  m_dc.MoveTo(pos+CSize(m_fontSize.cx,0));
}

// Add a new menu
void FrotzWnd::AddNewMenu(int menu, const unsigned short* text)
{
  // Expand the menus array
  if (menu+1 > m_menus.GetSize())
    m_menus.SetSize(menu+1);

  // Convert the menu to ASCII and add to the array
  UnicodeString str(text);
  m_menus[menu].RemoveAll();
  m_menus[menu].Add(str.ToAscii());
  m_buildMenus = true;
}

// Add a menu item to the bottom of an existing menu
void FrotzWnd::AddMenuItem(int menu, const unsigned short* text)
{
  UnicodeString str(text);
  m_menus[menu].Add(str.ToAscii());
  m_buildMenus = true;
}

// Remove an existing menu
void FrotzWnd::RemoveMenu(int menu)
{
  if (menu < m_menus.GetSize())
    m_menus[menu].RemoveAll();
  m_buildMenus = true;
}

// Update the menus, if needed
void FrotzWnd::UpdateMenus(void)
{
  if (m_buildMenus)
  {
    ((FrotzFrameWnd*)AfxGetMainWnd())->UpdateMenus(m_menus);
    m_buildMenus = false;
  }
}

// Get the code for the last clicked menu
unsigned short FrotzWnd::GetMenuClick(void)
{
  return m_lastMenu;
}

// Set if resizing is allowed
void FrotzWnd::SetAllowResize(bool allow)
{
  m_allowResize = allow;
}

// Resize the display and redraw
void FrotzWnd::ResizeDisplay(void)
{
  if (m_allowResize == false)
    return;

  CRect wndRect;
  GetClientRect(wndRect);
  CSize newSize = wndRect.Size();

  // Notify the game that the display needs refreshing
  if (h_version == V6)
    h_flags |= REFRESH_FLAG;

  // Update the game's header
  h_screen_width = (zword)newSize.cx;
  h_screen_height = (zword)newSize.cy;
  zword screen_cols = h_screen_width / h_font_width;
  if (screen_cols > 255)
    screen_cols = 255;
  h_screen_cols = (zbyte)screen_cols;
  zword screen_rows = h_screen_height / h_font_height;
  if (screen_rows > 255)
    screen_rows = 255;
  h_screen_rows = (zbyte)screen_rows;
  if (zmp != NULL)
  {
    resize_screen();
    restart_header();
  }

  // Is the window now bigger than the bitmap?
  CSize bmapSize = m_bitmap.GetSize();
  if ((newSize.cx > bmapSize.cx) || (newSize.cy > bmapSize.cy))
  {
    CreateBitmap(newSize);
    bmapSize = m_bitmap.GetSize();
  }

  // Erase the bitmap that is outside of the window
  int x = (newSize.cx < m_wndSize.cx) ? newSize.cx : m_wndSize.cx;
  int y = (newSize.cy < m_wndSize.cy) ? newSize.cy : m_wndSize.cy;
  COLORREF background = GetBackColour();
  FillSolid(CRect(x,0,bmapSize.cx,bmapSize.cy),background);
  FillSolid(CRect(0,y,bmapSize.cx,bmapSize.cy),background);

  // Redraw the bitmap
  Invalidate();

  // Take sure that the input code is notified
  InputType(Input::Reset);
}

// Calculate the ERF for scaling pictures
double FrotzWnd::CalcScalingERF(void)
{
  FrotzApp* app = (FrotzApp*)AfxGetApp();
  bb_resolution_t* res = bb_get_resolution(app->GetBlorbMap());
  if (res == NULL)
  {
    if (story_id != BEYOND_ZORK)
      return 1.0;

    double erfx = (double)m_wndSize.cx / 320.0;
    double erfy = (double)m_wndSize.cy / 200.0;
    return (erfx < erfy) ? erfx : erfy;
  }

  double erfx = (double)m_wndSize.cx / (double)res->px;
  double erfy = (double)m_wndSize.cy / (double)res->py;
  return (erfx < erfy) ? erfx : erfy;
}

// Select a font into the device context
void FrotzWnd::SelectFont(CFont& font)
{
  // Select into the window device context to make sure
  // that font anti-aliasing occurs, if at all possible
  CDC* dc = GetDC();
  dc->SelectObject(font);
  ReleaseDC(dc);

  m_dc.SelectObject(font);
}

// Get the current background colour
COLORREF FrotzWnd::GetBackColour(void)
{
  zbyte fore, back;
  get_window_colours(0,&fore,&back);

  FrotzApp* app = (FrotzApp*)AfxGetApp();
  if (back == 1)
    return app->GetDefaultColour(false);
  return app->GetColour(back);
}

// Redraw 'color changer' pictures
void FrotzWnd::RedrawColorChangers(double erf)
{
  POSITION pos = m_mapColorChanger.GetStartPosition();
  while (pos != NULL)
  {
    FrotzGfx* gfx;
    CPoint point;
    m_mapColorChanger.GetNextAssoc(pos,gfx,point);

    gfx->Paint(m_bitmap,point,erf);
  }
}

// Remove 'color changer' pictures contained in the rectangle
void FrotzWnd::PurgeColorChangers(LPCRECT rect)
{
  POSITION pos = m_mapColorChanger.GetStartPosition();
  while (pos != NULL)
  {
    FrotzGfx* gfx;
    CPoint point;
    m_mapColorChanger.GetNextAssoc(pos,gfx,point);

    if (PtInRect(rect, point))
      m_mapColorChanger.RemoveKey(gfx);
  }
}

// Constructor for text settings
FrotzWnd::TextSettings::TextSettings()
{
  style = 0;
  font = TEXT_FONT;
  fore = RGB(0,0,0);
  back = RGB(0,0,0);
  foreDefault = false;
  backDefault = false;
  backTransparent = false;
}

// Constructor for text settings
FrotzWnd::TextSettings::TextSettings(int s, int f)
{
  style = s;
  font = f;
  fore = RGB(0,0,0);
  back = RGB(0,0,0);
  foreDefault = false;
  backDefault = false;
  backTransparent = false;
}

// Constructor for input
FrotzWnd::Input::Input()
{
  type = VirtualKey;
  in = 0;
  mousex = 0;
  mousey = 0;
}

// Constructor for input
FrotzWnd::Input::Input(Type t, int i, bool m, int x, int y)
{
  type = t;
  in = i;
  modify = m;
  mousex = x;
  mousey = y;
}

// Constructor
FrotzWnd::UnicodeString::UnicodeString(const unsigned short* str)
{
  int len = wcslen((LPCWSTR)str);
  SetSize(0,len);
  for (int i = 0; i < len; i++)
    Add(str[i]);
}

// Copy constructor
FrotzWnd::UnicodeString::UnicodeString(const UnicodeString& str)
{
  Copy(str);
}

// Copy a unicode string
FrotzWnd::UnicodeString& FrotzWnd::UnicodeString::operator=(const UnicodeString& str)
{
  Copy(str);
  return *this;
}

// Convert to an ASCII string
CString FrotzWnd::UnicodeString::ToAscii(void)
{
  int len = GetSize();
  CString str;
  ::WideCharToMultiByte(CP_ACP,0,(LPCWSTR)GetData(),len,
    str.GetBufferSetLength(len),len,NULL,NULL);
  str.ReleaseBuffer(len);
  return str;
}

void FrotzWnd::OnPaint() 
{
  // Get the size of the display
  CRect client;
  GetClientRect(client);

  CPaintDC dc(this);
  COLORREF back = dc.SetBkColor(GetBackColour());

  // Copy the display bitmap
  dc.BitBlt(0,0,client.Width(),client.Height(),&m_dc,0,0,SRCCOPY);
  dc.SetBkColor(back);
}

BOOL FrotzWnd::OnEraseBkgnd(CDC*) 
{
  return TRUE;
}

void FrotzWnd::OnSize(UINT nType, int cx, int cy) 
{
  if (m_bitmap.GetSafeHandle() != 0)
    ResizeDisplay();

  m_wndSize.cx = cx;
  m_wndSize.cy = cy;
  CWnd::OnSize(nType,cx,cy);
}

void FrotzWnd::OnTimer(UINT nIDEvent) 
{
  switch (nIDEvent)
  {
  case InputTimer:
    m_input.AddTail(Input(Input::ZcodeKey,ZC_TIME_OUT));
    break;
  }
  CWnd::OnTimer(nIDEvent);
}

void FrotzWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
  MouseClick(point);
  CWnd::OnLButtonDown(nFlags,point);
}

void FrotzWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
  MouseClick(point);
  CWnd::OnMButtonDown(nFlags,point);
}

void FrotzWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
  MouseClick(point);
  CWnd::OnRButtonDown(nFlags,point);
}

LRESULT FrotzWnd::OnSoundNotify(WPARAM, LPARAM)
{
  FrotzSound::OnNotify();
  return 0;
}
