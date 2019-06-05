/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz dialog classes
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FrotzApp.h"
#include "FrotzDialogs.h"
#include "FrotzGfx.h"
#include "FrotzSound.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// The single application instance
extern FrotzApp theApp;

/////////////////////////////////////////////////////////////////////////////
// Call-back function for streaming into rich edit controls
/////////////////////////////////////////////////////////////////////////////

static DWORD CALLBACK RichStreamCB(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
  CFile* pFile = (CFile*)dwCookie;
  *pcb = pFile->Read(pbBuff,cb);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// RichEdit control for About dialogs
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CRichInfo, CRichEditCtrl)

BEGIN_MESSAGE_MAP(CRichInfo, CRichEditCtrl)
  ON_WM_SETFOCUS()
  ON_WM_SETCURSOR()
END_MESSAGE_MAP()

void CRichInfo::OnSetFocus(CWnd*)
{
  // Don't accept the focus ...
}

BOOL CRichInfo::OnSetCursor(CWnd*, UINT, UINT)
{
  // Don't let the cursor change ...
  return TRUE;
}

void CRichInfo::PreSubclassWindow()
{
  SetBackgroundColor(FALSE,GetSysColor(COLOR_3DFACE));

  // Set the control to word wrap the text
  SetTargetDevice(NULL,0);

  // Notify the parent window of the control's required size
  SetEventMask(ENM_REQUESTRESIZE);

  CRichEditCtrl::PreSubclassWindow();
}

void CRichInfo::SetText(int format, const CString& text)
{
  CMemFile inFile((BYTE*)((LPCTSTR)text),text.GetLength());

  EDITSTREAM stream;
  stream.dwCookie = (DWORD)&inFile;
  stream.pfnCallback = RichStreamCB;
  StreamIn(format,stream);
}

/////////////////////////////////////////////////////////////////////////////
// About This Game dialog
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(AboutGameDialog, BaseDialog)

AboutGameDialog::AboutGameDialog(CWnd* pParent)
  : BaseDialog(AboutGameDialog::IDD, pParent), m_dpi(96), m_headingEnd(0)
{
}

AboutGameDialog::~AboutGameDialog()
{
}

void AboutGameDialog::DoDataExchange(CDataExchange* pDX)
{
  BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDOK, m_ok);
}

BEGIN_MESSAGE_MAP(AboutGameDialog, BaseDialog)
  ON_WM_PAINT()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

BOOL AboutGameDialog::OnInitDialog()
{
  BaseDialog::OnInitDialog();
  m_dpi = DPI::getWindowDPI(this);

  CWaitCursor wc;
  const FrotzApp::GameInfo& gameInfo = theApp.GetGameInfo();

  // Initialize the rich edit text control
  if (m_info.SubclassDlgItem(IDC_INFO,this) == FALSE)
    return FALSE;
  m_info.SetText(SF_TEXT,gameInfo.description);

  // Add the title, author, etc.
  CString heading;
  heading.Append(gameInfo.title);
  heading.AppendChar('\r');
  if (gameInfo.seriesNumber.IsEmpty() == FALSE)
  {
    heading.Append(gameInfo.series);
    heading.Append(" #");
    heading.Append(gameInfo.seriesNumber);
    heading.AppendChar('\r');
  }
  if (gameInfo.headline.IsEmpty() == FALSE)
  {
    heading.Append(gameInfo.headline);
    heading.AppendChar('\r');
  }
  heading.Append(gameInfo.author);
  if (gameInfo.year.IsEmpty() == FALSE)
  {
    heading.Append(" (");
    heading.Append(gameInfo.year);
    heading.AppendChar(')');
  }
  heading.Append("\r\r");
  
  m_info.SetSel(0,0);
  CHARFORMAT format;
  format.cbSize = sizeof format;
  format.dwMask = CFM_BOLD;
  format.dwEffects = CFE_BOLD;
  m_info.SetSelectionCharFormat(format);
  m_info.ReplaceSel(heading);
  CHARRANGE headingSel;
  m_info.GetSel(headingSel);
  m_headingEnd = headingSel.cpMax;

  // Set the dialog title
  SetWindowText(gameInfo.title);

  // Get the initial position of the rich edit, used for spacing
  CRect initRect;
  m_info.GetWindowRect(initRect);
  ScreenToClient(initRect);

  // Get the cover art
  FrotzGfx* coverGfx = NULL;
  if (gameInfo.cover != -1)
    coverGfx = FrotzGfx::Get(gameInfo.cover,theApp.GetBlorbMap(),gameInfo.coverFormatWrong);

  CRect screen = theApp.GetScreenSize(true);
  if (coverGfx != NULL)
  {
    // Choose a size for the cover art
    CSize size = coverGfx->GetSize(1.0);
    double ratio = (double) screen.Width() / ((double) size.cx * 3.0);
    size = coverGfx->GetSize(ratio);

    // Resize the cover art
    CWindowDC dc(this);
    if (!m_coverBitmap.CreateBitmap(dc,size.cx,size.cy))
      return FALSE;
    coverGfx->Paint(m_coverBitmap,CPoint(0,0),ratio);

    m_coverRect = CRect(initRect.TopLeft(),size);
  }
  else
    m_coverRect = CRect(initRect.TopLeft(),CSize(screen.Width()/3,screen.Width()/3));

  // Resize the rich edit control
  CRect infoRect = m_coverRect;
  if (coverGfx != NULL)
    infoRect.OffsetRect(m_coverRect.right,0);
  m_info.MoveWindow(infoRect);

  // Get the size of the OK button
  CRect okRect;
  m_ok.GetClientRect(okRect);

  // Resize the OK button
  okRect.MoveToXY(
    initRect.left+((infoRect.right+initRect.left)/2)-(okRect.Width()/2),
    m_coverRect.bottom+infoRect.top);
  m_ok.MoveWindow(okRect,FALSE);

  // Get the initial dialog size
  CRect dlgCRect, dlgWRect;
  GetClientRect(dlgCRect);
  GetWindowRect(dlgWRect);
  int dlgX = dlgWRect.Width()-dlgCRect.Width();
  int dlgY = dlgWRect.Height()-dlgCRect.Height();

  // Resize the dialog
  MoveWindow(0,0,
    infoRect.right+initRect.left+dlgX,
    okRect.bottom+initRect.top+dlgY,FALSE);
  CenterWindow();
  return TRUE;
}

void AboutGameDialog::OnPaint()
{
  if (m_coverBitmap.GetBits() != NULL)
  {
    CPaintDC paintDC(this);

    CDC memDC;
    memDC.CreateCompatibleDC(&paintDC);
    CDibSection::SelectDibSection(memDC,&m_coverBitmap);

    paintDC.BitBlt(
      m_coverRect.left,m_coverRect.top,m_coverRect.Width(),m_coverRect.Height(),
      &memDC,0,0,SRCCOPY);
  }
  else
    Default();
}

LRESULT AboutGameDialog::OnDpiChanged(WPARAM wparam, LPARAM)
{
  Default();

  int newDpi = (int)HIWORD(wparam);
  if (m_dpi != newDpi)
  {
    // Rescale the cover art
    if (m_coverBitmap.GetBits() != NULL)
    {
      m_info.GetWindowRect(m_coverRect);
      ScreenToClient(m_coverRect);
      m_coverRect.OffsetRect(-m_coverRect.Width(),0);
      m_coverRect.OffsetRect(-(m_coverRect.left/2),0);

      const FrotzApp::GameInfo& gameInfo = theApp.GetGameInfo();
      if (gameInfo.cover != -1)
      {
        FrotzGfx* coverGfx = FrotzGfx::Get(gameInfo.cover,theApp.GetBlorbMap(),gameInfo.coverFormatWrong);
        if (coverGfx != NULL)
        {
          m_coverBitmap.DeleteBitmap();

          CSize size = coverGfx->GetSize(1.0);
          double ratioX = (double)m_coverRect.Width() / (double)size.cx;
          double ratioY = (double)m_coverRect.Height() / (double)size.cy;
          double ratio = (ratioX > ratioY) ? ratioY : ratioX;

          CWindowDC dc(this);
          if (m_coverBitmap.CreateBitmap(dc,m_coverRect.Width(),m_coverRect.Height()))
          {
            m_coverBitmap.FillSolid(::GetSysColor(COLOR_3DFACE));
            coverGfx->Paint(m_coverBitmap,CPoint(0,0),ratio);
          }
        }
      }
    }

    // Apply formatting
    if (m_info.GetSafeHwnd())
    {
      m_info.SetSel(0,m_headingEnd);
      CHARFORMAT format;
      format.cbSize = sizeof format;
      format.dwMask = CFM_BOLD;
      format.dwEffects = CFE_BOLD;
      m_info.SetSelectionCharFormat(format);
      m_info.SetSel(0,0);
    }

    m_dpi = newDpi;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// About dialog
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(AboutDialog, BaseDialog)

AboutDialog::AboutDialog(CWnd* pParent)
  : BaseDialog(AboutDialog::IDD, pParent), m_dpi(96)
{
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::DoDataExchange(CDataExchange* pDX)
{
  BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LOGO, m_logo);
  DDX_Control(pDX, IDC_BORDER, m_border);
  DDX_Control(pDX, IDOK, m_ok);
}

BEGIN_MESSAGE_MAP(AboutDialog, BaseDialog)
  ON_NOTIFY(EN_REQUESTRESIZE, IDC_INFO, OnResizeInfo)
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

BOOL AboutDialog::OnInitDialog()
{
  BaseDialog::OnInitDialog();
  m_dpi = DPI::getWindowDPI(this);

  // Subclass the rich edit text control
  if (m_info.SubclassDlgItem(IDC_INFO,this) == FALSE)
    return FALSE;

  CRect dlgRect;
  GetWindowRect(dlgRect);

  // Load the bitmap and center it in the dialog
  m_logo.SetBitmap(::LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_FROTZ)));
  CRect logoRect;
  m_logo.GetWindowRect(logoRect);
  ScreenToClient(logoRect);
  int w = logoRect.Width();
  logoRect.left = (dlgRect.Width()-w)/2;
  logoRect.right = logoRect.left+w;
  m_logo.MoveWindow(logoRect,FALSE);

  // Move the controls down to make room for the bitmap
  CRect rect;
  m_border.GetWindowRect(rect);
  ScreenToClient(rect);
  rect.OffsetRect(0,logoRect.Height());
  m_border.MoveWindow(rect,FALSE);
  m_info.GetWindowRect(rect);
  ScreenToClient(rect);
  rect.OffsetRect(0,logoRect.Height());
  m_info.MoveWindow(rect,FALSE);
  m_ok.GetWindowRect(rect);
  ScreenToClient(rect);
  rect.OffsetRect(0,logoRect.Height());
  m_ok.MoveWindow(rect,FALSE);

  // Resize the dialog
  dlgRect.bottom += logoRect.Height();
  MoveWindow(dlgRect,FALSE);

  // Load the text into the rich edit text control
  SetInfoText();
  return TRUE;
}

void AboutDialog::OnResizeInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
  REQRESIZE *pReqResize = reinterpret_cast<REQRESIZE *>(pNMHDR);

  // Work out the change in height of the info text box
  CRect rect;
  m_info.GetWindowRect(rect);
  ScreenToClient(rect);
  int offset = CRect(pReqResize->rc).Height() - rect.Height();

  // Resize the info text box
  rect.bottom += offset;
  m_info.MoveWindow(rect,FALSE);

  // Resize the bounding rectangle
  m_border.GetWindowRect(rect);
  ScreenToClient(rect);
  rect.bottom += offset;
  m_border.MoveWindow(rect,FALSE);

  // Move the OK button
  m_ok.GetWindowRect(rect);
  ScreenToClient(rect);
  rect.OffsetRect(0,offset);
  m_ok.MoveWindow(rect,FALSE);

  // Resize the dialog
  GetWindowRect(rect);
  rect.bottom += offset;
  MoveWindow(rect,FALSE);

  *pResult = 0;
}

LRESULT AboutDialog::OnDpiChanged(WPARAM wparam, LPARAM)
{
  Default();

  int newDpi = (int)HIWORD(wparam);
  if (m_dpi != newDpi)
  {
    if (m_info.GetSafeHwnd())
      SetInfoText();

    m_dpi = newDpi;
  }
  return 0;
}

void AboutDialog::SetInfoText(void)
{
  CString aboutText;
  aboutText.LoadString(IDS_ABOUT_INFO);
  aboutText.Replace("%ver%","1.21");
  aboutText.Replace("%year%","2019");
  m_info.SetText(SF_RTF,aboutText);
}

/////////////////////////////////////////////////////////////////////////////
// Options dialog
/////////////////////////////////////////////////////////////////////////////

#define WM_RESIZEPAGE WM_APP+1

OptionsDialog::OptionsDialog(UINT caption, CWnd* parentWnd) : CPropertySheet(caption,parentWnd)
{
  GetFontDialog getFont(m_logFont,IDD_ABOUTGAME,parentWnd);
  getFont.DoModal();

  m_dpi = 96;
  m_fontHeightPerDpi = (double)m_logFont.lfHeight / (double)m_dpi;
}

BEGIN_MESSAGE_MAP(OptionsDialog, CPropertySheet)
  ON_WM_HELPINFO()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_MESSAGE(WM_RESIZEPAGE, OnResizePage)  
END_MESSAGE_MAP()

BOOL OptionsDialog::OnInitDialog() 
{
  CPropertySheet::OnInitDialog();

  m_dpi = DPI::getWindowDPI(this);
  m_fontHeightPerDpi = (double)m_logFont.lfHeight / (double)m_dpi;

  // Enable context sensitive help
  ModifyStyleEx(0,WS_EX_CONTEXTHELP);

  // Create the font to use
  m_font.CreateFontIndirect(&m_logFont);

  // Set the font for the property pages
  ChangeDialogFont(this,&m_font,0.0);
  CPropertyPage* page = GetActivePage();
  for (int i = 0; i < GetPageCount(); i++)
  {
    SetActivePage(i);
    CPropertyPage* page = GetActivePage();
    DPI::disableDialogDPI(page);
    ChangeDialogFont(page,&m_font,0.0);
  }
  SetActivePage(page);

  // Resize the property page
  CTabCtrl* tab = GetTabControl();
  tab->GetWindowRect(&m_page);
  ScreenToClient(&m_page);
  tab->AdjustRect(FALSE,&m_page);
  page->MoveWindow(&m_page);

  return TRUE;
}

BOOL OptionsDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
  NMHDR* pnmh = (LPNMHDR) lParam;
  if (pnmh->code == TCN_SELCHANGE)
    PostMessage(WM_RESIZEPAGE);
  return CPropertySheet::OnNotify(wParam, lParam, pResult);
}

BOOL OptionsDialog::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  if (message == WM_ACTIVATE)
  {
    if ((lParam != 0) && !::IsWindow((HWND)lParam))
    {
      if (pResult != NULL)
        *pResult = 1;
      return TRUE;
    }
  }
  return CPropertySheet::OnWndMsg(message,wParam,lParam,pResult);
}

HWND WINAPI AfxHtmlHelp(HWND hWnd, LPCTSTR szHelpFilePath, UINT nCmd, DWORD_PTR dwData);

BOOL OptionsDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
  static DWORD helpIds[] =
  {
    IDC_PROP_FONT,1,
    IDC_FIXED_FONT,2,
    IDC_FONT_SIZE,3,
    IDC_TEXT_COLOUR,5,
    IDC_BACK_COLOUR,6,
    IDC_TERP_NUMBER,7,
    IDC_ERRORS,8,
    IDC_IGNORE_RUNTIME,9,
    IDC_EXPAND,10,
    IDC_TANDY,11,
    IDC_FAST_SCROLL,12,
    IDC_LEFT_MARGIN,13,
    IDC_RIGHT_MARGIN,14,
    IDC_MORE_PROMPT,15,
    IDC_REGISTER_FILETYPES,16,
    IDC_WRAP_SCRIPT,17,
    IDC_SHOW_IFICTION,18,
    IDC_USERNAME,19,
    IDC_SPEAK,20,
    IDC_VOICE,21,
    IDC_SPEECH_RATE,22,
    0,0
  };

  if (pHelpInfo->iContextType == HELPINFO_WINDOW)
  {
    // Is there a help topic for this control?
    DWORD* id = helpIds;
    while (*id != 0)
    {
      if (pHelpInfo->iCtrlId == *id)
      {
        CString helpFile(AfxGetApp()->m_pszHelpFilePath);
        helpFile.Append("::/options.txt");

        // Show the help popup
        AfxHtmlHelp((HWND)pHelpInfo->hItemHandle,helpFile,
          HH_TP_HELP_WM_HELP,(DWORD_PTR)helpIds);
        return TRUE;
      }
      id += 2;
    }
  }
  return TRUE;
}

LRESULT OptionsDialog::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  int newDpi = (int)HIWORD(wparam);
  if (m_dpi != newDpi)
  {
    if (GetTabControl() != NULL)
    {
      // Use the top-left corner of the suggested window rectangle
      CRect windowRect;
      GetWindowRect(windowRect);
      windowRect.left = ((LPRECT)lparam)->left;
      windowRect.top = ((LPRECT)lparam)->top;
      MoveWindow(windowRect,TRUE);

      // Update the font
      m_logFont.lfHeight = (long)(m_fontHeightPerDpi * newDpi);
      CFont oldFont;
      oldFont.Attach(m_font.Detach());
      m_font.CreateFontIndirect(&m_logFont);

      // Update the dialog to use the new font
      double scaleDpi = (double)newDpi / (double)m_dpi;
      ChangeDialogFont(this,&m_font,scaleDpi);
      CPropertyPage* page = GetActivePage();
      for (int i = 0; i < GetPageCount(); i++)
      {
        SetActivePage(i);
        CPropertyPage* page = GetActivePage();
        ChangeDialogFont(page,&m_font,scaleDpi);
      }
      SetActivePage(page);

      // Resize the property page
      CTabCtrl* tab = GetTabControl();
      tab->GetWindowRect(&m_page);
      ScreenToClient(&m_page);
      tab->AdjustRect(FALSE,&m_page);
      page->MoveWindow(&m_page);
    }

    m_dpi = newDpi;
  }
  return 0;
}

LONG OptionsDialog::OnResizePage(UINT, LONG)
{
  CPropertyPage* page = GetActivePage();
  page->MoveWindow(&m_page);
  return 0;
}

void OptionsDialog::ChangeDialogFont(CWnd* wnd, CFont* font, double scale)
{
  CRect windowRect;

  double scaleW = 1.0, scaleH = 1.0;
  if (scale > 0.0)
  {
    scaleW = scale;
    scaleH = scale;
  }
  else
  {
    TEXTMETRIC tmOld, tmNew;
    CDC* dc = wnd->GetDC();
    CFont* oldFont = dc->SelectObject(wnd->GetFont());
    dc->GetTextMetrics(&tmOld);
    dc->SelectObject(font);
    dc->GetTextMetrics(&tmNew);
    dc->SelectObject(oldFont);
    wnd->ReleaseDC(dc);

    scaleW = (double)tmNew.tmAveCharWidth / (double)tmOld.tmAveCharWidth;
    scaleH = (double)(tmNew.tmHeight+tmNew.tmExternalLeading) /
             (double)(tmOld.tmHeight+tmOld.tmExternalLeading);
  }

  // Calculate new dialog window rectangle
  CRect clientRect, newClientRect, newWindowRect;

  wnd->GetWindowRect(windowRect);
  wnd->GetClientRect(clientRect);
  long xDiff = windowRect.Width() - clientRect.Width();
  long yDiff = windowRect.Height() - clientRect.Height();

  newClientRect.left = newClientRect.top = 0;
  newClientRect.right = (long)(clientRect.right * scaleW);
  newClientRect.bottom = (long)(clientRect.bottom * scaleH);

  newWindowRect.left = windowRect.left - (newClientRect.right - clientRect.right)/2;
  newWindowRect.top = windowRect.top - (newClientRect.bottom - clientRect.bottom)/2;
  newWindowRect.right = newWindowRect.left + newClientRect.right + xDiff;
  newWindowRect.bottom = newWindowRect.top + newClientRect.bottom + yDiff;

  wnd->MoveWindow(newWindowRect);
  wnd->SetFont(font);

  CWnd* childWnd = wnd->GetWindow(GW_CHILD);
  while (childWnd)
  {
    childWnd->SetFont(font);
    childWnd->GetWindowRect(windowRect);

    CString strClass;
    ::GetClassName(childWnd->GetSafeHwnd(),strClass.GetBufferSetLength(32),31);
    strClass.MakeUpper();
    if (strClass == "COMBOBOX")
    {
      CRect rect;
      childWnd->SendMessage(CB_GETDROPPEDCONTROLRECT,0,(LPARAM)&rect);
      windowRect.right = rect.right;
      windowRect.bottom = rect.bottom;
    }

    wnd->ScreenToClient(windowRect);
    windowRect.left = (long)(windowRect.left * scaleW);
    windowRect.right = (long)(windowRect.right * scaleW);
    windowRect.top = (long)(windowRect.top * scaleH);
    windowRect.bottom = (long)(windowRect.bottom * scaleH);
    childWnd->MoveWindow(windowRect);

    childWnd = childWnd->GetWindow(GW_HWNDNEXT);
  }
}

OptionsDisplayPage::OptionsDisplayPage() : CPropertyPage(OptionsDisplayPage::IDD)
{
  m_fastScroll = FALSE;
  m_morePrompts = FALSE;
  m_leftMargin = 0;
  m_rightMargin = 0;
}

void OptionsDisplayPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_PROP_FONT, m_propFont);
  DDX_Control(pDX, IDC_FIXED_FONT, m_fixedFont);
  DDX_CBString(pDX, IDC_FONT_SIZE, m_fontSize);
  DDX_Check(pDX, IDC_FAST_SCROLL, m_fastScroll);
  DDX_Check(pDX, IDC_MORE_PROMPT, m_morePrompts);
  DDX_Text(pDX, IDC_LEFT_MARGIN, m_leftMargin);
  DDX_Text(pDX, IDC_RIGHT_MARGIN, m_rightMargin);
}

BOOL OptionsDisplayPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // Get all the possible fonts
  CDC* dc = GetDC();
  LOGFONT font;
  ::ZeroMemory(&font,sizeof(LOGFONT));
  font.lfCharSet = ANSI_CHARSET;
  ::EnumFontFamiliesEx(dc->GetSafeHdc(),&font,(FONTENUMPROC)ListFonts,(LPARAM)this,0);
  ReleaseDC(dc);

  // Initialize the font controls
  if (m_propFont.SelectString(-1,m_propFontName) == CB_ERR)
    m_propFont.SetCurSel(0);
  if (m_fixedFont.SelectString(-1,m_fixedFontName) == CB_ERR)
    m_fixedFont.SetCurSel(0);

  // Initialize the colour controls
  m_textColour.SubclassDlgItem(IDC_TEXT_COLOUR,this);
  m_backColour.SubclassDlgItem(IDC_BACK_COLOUR,this);
  return TRUE;
}

// Called when the dialog has been closed with the OK button
void OptionsDisplayPage::OnOK()
{
  // Read the font controls
  m_propFont.GetWindowText(m_propFontName);
  m_fixedFont.GetWindowText(m_fixedFontName);
  CPropertyPage::OnOK();
}

// Called when enumerating fonts, populates the font drop down lists in the dialog
int CALLBACK OptionsDisplayPage::ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param)
{
  OptionsDisplayPage* page = (OptionsDisplayPage*)param;

  // Only allow scaleable fonts (TrueType, etc.)
  bool allow = false;
  if (fontType & TRUETYPE_FONTTYPE)
    allow = true;
  else if (!(fontType & RASTER_FONTTYPE))
    allow = ((metric->ntmTm.ntmFlags & NTM_PS_OPENTYPE|NTM_TT_OPENTYPE|NTM_TYPE1) != 0);

  if (allow)
  {
    if (font->elfLogFont.lfFaceName[0] != '@')
    {
      page->m_propFont.AddString(font->elfLogFont.lfFaceName);
      if (font->elfLogFont.lfPitchAndFamily & FIXED_PITCH)
        page->m_fixedFont.AddString(font->elfLogFont.lfFaceName);
    }
  }
  return 1;
}

OptionsInterpreterPage::OptionsInterpreterPage() : CPropertyPage(OptionsInterpreterPage::IDD)
{
  m_interpreter = 0;
  m_reportErrors = 0;
  m_expand = FALSE;
  m_tandy = FALSE;
  m_ignore = FALSE;
  m_wrapScript = FALSE;
}

void OptionsInterpreterPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_CBIndex(pDX, IDC_TERP_NUMBER, m_interpreter);
  DDX_CBIndex(pDX, IDC_ERRORS, m_reportErrors);
  DDX_Text(pDX, IDC_USERNAME, m_username);
  DDX_Check(pDX, IDC_EXPAND, m_expand);
  DDX_Check(pDX, IDC_TANDY, m_tandy);
  DDX_Check(pDX, IDC_IGNORE_RUNTIME, m_ignore);
  DDX_Check(pDX, IDC_WRAP_SCRIPT, m_wrapScript);
}

OptionsStartupPage::OptionsStartupPage() : CPropertyPage(OptionsStartupPage::IDD)
{
  m_register = FALSE;
  m_iFiction = 0;
}

void OptionsStartupPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_CBIndex(pDX, IDC_SHOW_IFICTION, m_iFiction);
  DDX_Check(pDX, IDC_REGISTER_FILETYPES, m_register);
}

BOOL OptionsStartupPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // Remove the file registration checkbox for Vista and beyond
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof osvi;
  ::GetVersionEx(&osvi);
  if (osvi.dwMajorVersion >= 6)
    GetDlgItem(IDC_REGISTER_FILETYPES)->ShowWindow(SW_HIDE);

  return TRUE;
}

OptionsSpeechPage::OptionsSpeechPage() : CPropertyPage(OptionsSpeechPage::IDD)
{
  m_speak = FALSE;
  m_rate = 0;
}

void OptionsSpeechPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_SPEAK, m_speak);
  DDX_Control(pDX, IDC_VOICE, m_voiceCtrl);
  DDX_Control(pDX, IDC_SPEECH_RATE, m_rateCtrl);
  DDX_Slider(pDX, IDC_SPEECH_RATE, m_rate);

  if (pDX->m_bSaveAndValidate)
  {
    int sel = m_voiceCtrl.GetCurSel();
    if (sel != CB_ERR)
      m_voiceCtrl.GetLBText(sel,m_voice);
  }
  else
  {
    if (m_voiceCtrl.SelectString(-1,m_voice) == CB_ERR)
    {
      if (m_voiceCtrl.SelectString(-1,m_defaultVoice) == CB_ERR)
        m_voiceCtrl.SetCurSel(0);
    }
  }
}

BOOL OptionsSpeechPage::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();

  CStringArray voices;
  TextToSpeech::GetSpeechEngine().GetVoices(voices,m_defaultVoice);
  for (int i = 0; i < voices.GetSize(); i++)
    m_voiceCtrl.AddString(voices.GetAt(i));

  m_rateCtrl.SetRange(-10,10,TRUE);
  m_rateCtrl.SetPos(m_rate);
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Scrollback dialog
/////////////////////////////////////////////////////////////////////////////

#define WM_SAMESIZEASMAIN (WM_APP+1)

IMPLEMENT_DYNAMIC(ScrollbackDialog, BaseDialog)

ScrollbackDialog::ScrollbackDialog(LPCWSTR text, int textLen, CWnd* pParent)
  : BaseDialog(ScrollbackDialog::IDD, pParent)
{
  m_text = text;
  m_textLen = textLen;
  m_textTop = 0;
  m_dpi = 96;
}

ScrollbackDialog::~ScrollbackDialog()
{
}

void ScrollbackDialog::DoDataExchange(CDataExchange* pDX)
{
  BaseDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ScrollbackDialog, BaseDialog)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_COPY, OnCopy)
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_MESSAGE(WM_SAMESIZEASMAIN, OnSameSizeAsMain)
END_MESSAGE_MAP()

BOOL ScrollbackDialog::OnInitDialog()
{
  BaseDialog::OnInitDialog();
  m_dpi = DPI::getWindowDPI(this);

  // Subclass the rich edit text control
  if (m_edit.SubclassDlgItem(IDC_TEXT,this) == FALSE)
    return FALSE;

  // Get the relative position of the top of the text control
  CRect size;
  m_edit.GetWindowRect(size);
  ScreenToClient(size);
  m_textTop = size.top;

  // Change the window icon
  SetIcon(::LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_INFOCOM)),TRUE);

  // Get the size of the main window and the display
  AfxGetMainWnd()->GetWindowRect(size);
  CRect screen = theApp.GetScreenSize(true);

  // Resize the dialog, but no bigger than the display
  if ((size.Width() > screen.Width()) && (size.Height() > screen.Height()))
    MoveWindow(screen);
  else
    MoveWindow(size);

  // Set the control to format the text so that it fits
  // into the window
  m_edit.SetTargetDevice(NULL,0);

  // Set the background colour
  m_edit.SetBackgroundColor(FALSE,GetSysColor(COLOR_3DFACE));

  // Set the control to only show plain text, but allow all code pages
  m_edit.SetTextMode(TM_PLAINTEXT|TM_SINGLELEVELUNDO|TM_MULTICODEPAGE);

  // Put the text into the control
  if (m_text != NULL)
  {
    CMemFile inFile((BYTE*)m_text,m_textLen*sizeof(WCHAR));
    EDITSTREAM stream;
    stream.dwCookie = (DWORD)&inFile;
    stream.pfnCallback = RichStreamCB;
    m_edit.StreamIn(SF_TEXT|SF_UNICODE,stream);
  }

  // Scroll the control to the end of the text
  m_edit.SetFocus();
  m_edit.SetSel(-1,-1);
  m_edit.SendMessage(EM_SCROLLCARET);

  return TRUE;
}

void ScrollbackDialog::OnSize(UINT nType, int cx, int cy)
{
  BaseDialog::OnSize(nType,cx,cy);

  // Resize the edit control
  if (m_edit.GetSafeHwnd() != NULL)
    m_edit.SetWindowPos(NULL,0,m_textTop,cx,cy-m_textTop,SWP_NOZORDER);
}

void ScrollbackDialog::OnCopy()
{
  m_edit.Copy();
}

LRESULT ScrollbackDialog::OnDpiChanged(WPARAM wparam, LPARAM)
{
  int newDpi = (int)HIWORD(wparam);
  if (m_dpi != newDpi)
  {
    m_textTop = MulDiv(m_textTop,newDpi,m_dpi);
    m_dpi = newDpi;
  }

  Default();

  // Same monitor?
  if (DPI::getMonitorRect(this) == DPI::getMonitorRect(AfxGetMainWnd()))
    PostMessage(WM_SAMESIZEASMAIN);
  return 0;
}

LRESULT ScrollbackDialog::OnSameSizeAsMain(WPARAM, LPARAM)
{
  // Resize the dialog to be the same as the main window
  CRect DialogRect;
  AfxGetMainWnd()->GetWindowRect(DialogRect);
  MoveWindow(DialogRect);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// File dialog for loading a new Z-code game
/////////////////////////////////////////////////////////////////////////////

#ifndef __IModalWindow_INTERFACE_DEFINED__

MIDL_INTERFACE("b4db1657-70d7-485e-8e3e-6fcb5a5c1802")
IModalWindow : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE Show(HWND) = 0;
};

#endif // __IModalWindow_INTERFACE_DEFINED__

#ifndef __IShellItem_INTERFACE_DEFINED__

typedef enum _SIGDN
{
  SIGDN_NORMALDISPLAY	= 0,
  SIGDN_PARENTRELATIVEPARSING	= 0x80018001,
  SIGDN_PARENTRELATIVEFORADDRESSBAR	= 0x8001c001,
  SIGDN_DESKTOPABSOLUTEPARSING	= 0x80028000,
  SIGDN_PARENTRELATIVEEDITING	= 0x80031001,
  SIGDN_DESKTOPABSOLUTEEDITING	= 0x8004c000,
  SIGDN_FILESYSPATH	= 0x80058000,
  SIGDN_URL	= 0x80068000
}
SIGDN;

typedef DWORD SICHINTF;

MIDL_INTERFACE("43826d1e-e718-42ee-bc55-a1e261c37bfe")
IShellItem : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE BindToHandler(IBindCtx*, REFGUID, REFIID, void**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetParent(IShellItem**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDisplayName(SIGDN, LPOLESTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetAttributes(SFGAOF, SFGAOF*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Compare(IShellItem*, SICHINTF, int*) = 0;
};

#endif // __IShellItem_INTERFACE_DEFINED__

#ifndef __IFileDialog_INTERFACE_DEFINED__

typedef struct _COMDLG_FILTERSPEC
{
  LPCWSTR pszName;
  LPCWSTR pszSpec;
}
COMDLG_FILTERSPEC;

typedef enum _FDAP
{
  FDAP_BOTTOM = 0,
  FDAP_TOP = 1
}
FDAP;

typedef DWORD CDCONTROLSTATEF;

typedef interface IFileDialogEvents IFileDialogEvents;
typedef interface IShellItemFilter IShellItemFilter;

MIDL_INTERFACE("42f85136-db7e-439c-85f1-e4075d135fc8")
IFileDialog : public IModalWindow
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetFileTypes(UINT, const COMDLG_FILTERSPEC*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFileTypeIndex(UINT) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFileTypeIndex(UINT*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Advise(IFileDialogEvents*, DWORD*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Unadvise(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetOptions(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetOptions(DWORD*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDefaultFolder(IShellItem*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFolder(IShellItem*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFolder(IShellItem**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCurrentSelection(IShellItem**) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFileName(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFileName(LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetTitle(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetOkButtonLabel(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFileNameLabel(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetResult(IShellItem**) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddPlace(IShellItem*, FDAP) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDefaultExtension(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE Close(HRESULT) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetClientGuid(REFGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE ClearClientData(void) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFilter(IShellItemFilter*) = 0;
};

MIDL_INTERFACE("e6fdd21a-163f-4975-9c8c-a69f1ba37034")
IFileDialogCustomize : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE EnableOpenDropDown(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddMenu(DWORD, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddPushButton(DWORD, LPCWSTR pszLabel) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddComboBox(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddRadioButtonList(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddCheckButton(DWORD, LPCWSTR, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddEditBox(DWORD, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddSeparator(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddText(DWORD, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetControlLabel(DWORD, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetControlState(DWORD, CDCONTROLSTATEF*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetControlState(DWORD, CDCONTROLSTATEF) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetEditBoxText(DWORD, WCHAR**) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetEditBoxText(DWORD, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCheckButtonState(DWORD, BOOL*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetCheckButtonState(DWORD, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddControlItem(DWORD, DWORD, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE RemoveControlItem(DWORD, DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE RemoveAllControlItems(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetControlItemState(DWORD, DWORD, CDCONTROLSTATEF*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetControlItemState(DWORD, DWORD, CDCONTROLSTATEF) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetSelectedControlItem(DWORD, DWORD*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetSelectedControlItem(DWORD, DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE StartVisualGroup(DWORD, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE EndVisualGroup(void) = 0;
  virtual HRESULT STDMETHODCALLTYPE MakeProminent(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetControlItemText(DWORD, DWORD, LPCWSTR) = 0;
};

class DECLSPEC_UUID("DC1C5A9C-E88A-4dde-A5A1-60F82A20AEF7") FileOpenDialog;

#endif // __IFileDialog_INTERFACE_DEFINED__

IMPLEMENT_DYNAMIC(GameFileDialog, CFileDialog)

GameFileDialog::GameFileDialog(LPCTSTR lpszFileName, CWnd* pParentWnd)
: CFileDialog(TRUE,NULL,lpszFileName,OFN_HIDEREADONLY|OFN_ENABLETEMPLATE,
    CResString(IDS_ZCODE_FILTER),pParentWnd,0),
  m_title(IDS_ZCODE_TITLE)
{
  m_ofn.lpstrTitle = m_title;
  m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_LOADGAME);
  m_ofn.Flags |= OFN_ENABLESIZING;
  m_quetzal = theApp.GetUseQuetzal();
}

BEGIN_MESSAGE_MAP(GameFileDialog, CFileDialog)
  ON_BN_CLICKED(IDC_QUETZAL, OnUseQuetzal)
END_MESSAGE_MAP()

void GameFileDialog::OnUseQuetzal()
{
  m_quetzal = ((CButton*)GetDlgItem(IDC_QUETZAL))->GetCheck() == BST_CHECKED;
}

void GameFileDialog::OnInitDone()
{
  ((CButton*)GetDlgItem(IDC_QUETZAL))->SetCheck(m_quetzal ? TRUE : FALSE);
  CFileDialog::OnInitDone();
}

bool GameFileDialog::ShowDialog(CString& path, CWnd* parent)
{
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof osvi;
  ::GetVersionEx(&osvi);

  // For Windows Vista and above, use the new file dialog
  if (osvi.dwMajorVersion >= 6)
  {
    // Create a file dialog object
    CComPtr<IFileDialog> dialog;
    if (FAILED(dialog.CoCreateInstance(__uuidof(FileOpenDialog))))
      return false;

    // Set the dialog's title
    CStringW title(CResString(IDS_ZCODE_TITLE));
    dialog->SetTitle(title);

    // Set the dialog's file filter
    COMDLG_FILTERSPEC filters[8];
    int filterCount = 0;
    CStringW filter(CResString(IDS_ZCODE_FILTER));
    LPWSTR filterPtr1 = filter.GetBuffer(), filterPtr2 = filterPtr1;
    int mode = 0;
    while ((*filterPtr2 != '\0') && (filterCount < (sizeof filters/sizeof filter[0])))
    {
      switch (*filterPtr2)
      {
      case '|':
        if (mode == 2)
        {
          filters[filterCount++].pszSpec = filterPtr1;
          mode = 0;
        }
        else
        {
          filters[filterCount].pszName = filterPtr1;
          mode = 2;
        }
        *filterPtr2 = '\0';
        filterPtr1 = filterPtr2+1;
        break;

      case '(':
        if (mode == 0)
        {
          *(filterPtr2-1) = '\0';
          mode = 1;
        }
      }
      filterPtr2++;
    }
    filter.ReleaseBuffer();
    dialog->SetFileTypes(filterCount,filters);

    // Set the initial file name for the dialog
    CStringW pathStr(path);
    int pos = pathStr.ReverseFind(L'\\');
    if (pos >= 0)
      dialog->SetFileName(pathStr.Mid(pos+1));

    // Add the 'use quetzal' check box
    CComQIPtr<IFileDialogCustomize> custom(dialog);
    CStringW checkText(CResString(IDS_QUETZAL));
    custom->AddCheckButton(0,checkText,theApp.GetUseQuetzal() ? TRUE : FALSE);

    // Show the file dialog
    if (FAILED(dialog->Show(parent->GetSafeHwnd())))
      return false;

    // Record whether the user has changed the 'use quetzal' check box
    BOOL quetzal = FALSE;
    custom->GetCheckButtonState(0,&quetzal);
    theApp.SetUseQuetzal((quetzal == TRUE));

    // Get the path to the selected file
    CComPtr<IShellItem> result;
    if (FAILED(dialog->GetResult(&result)))
      return false;
    LPOLESTR resultName;
    if (FAILED(result->GetDisplayName(SIGDN_FILESYSPATH,&resultName)))
      return false;
    path = resultName;
    ::CoTaskMemFree(resultName);
    return true;
  }
  else
  {
    GameFileDialog dialog(path,AfxGetMainWnd());
    if (dialog.DoModal() == IDOK)
    {
      theApp.SetUseQuetzal(dialog.m_quetzal);
      path = dialog.GetPathName();
      return true;
    }
  }
  return false;
}
