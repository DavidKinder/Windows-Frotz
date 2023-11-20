/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz application class
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Resource.h"

#include "ColourButton.h"
#include "DarkMode.h"
#include "Dialogs.h"
#include "Dib.h"

class CRichInfo : public CRichEditCtrl
{
  DECLARE_DYNAMIC(CRichInfo)

protected:
  DECLARE_MESSAGE_MAP()

  virtual void PreSubclassWindow();

public:
  afx_msg void OnSetFocus(CWnd* pOldWnd);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

  void SetText(int format, const CString& text);
};

class AboutGameDialog : public BaseDialog
{
  DECLARE_DYNAMIC(AboutGameDialog)

public:
  AboutGameDialog(CWnd* pParent = NULL);   // standard constructor
  virtual ~AboutGameDialog();

// Dialog Data
  enum { IDD = IDD_ABOUTGAME };

protected:
  //{{AFX_MSG(FrotzWnd)
  afx_msg void OnPaint();
  //}}AFX_MSG
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  DECLARE_MESSAGE_MAP()

protected:
  CRichInfo m_info;
  DarkModeButton m_ok;

  int m_dpi;
  int m_headingEnd;
  CRect m_coverRect;
  CDibSection m_coverBitmap;
};

class AboutDialog : public BaseDialog
{
  DECLARE_DYNAMIC(AboutDialog)

public:
  AboutDialog(CWnd* pParent = NULL);   // standard constructor
  virtual ~AboutDialog();

// Dialog Data
  enum { IDD = IDD_ABOUT };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  DECLARE_MESSAGE_MAP()

public:
  virtual BOOL OnInitDialog();
  afx_msg void OnResizeInfo(NMHDR *pNMHDR, LRESULT *pResult);

protected:
  void SetInfoText(void);

  int m_dpi;
  CStatic m_logo;
  CRichInfo m_info;
  DarkModeGroupBox m_border;
  DarkModeButton m_ok;
};

class OptionsDialog : public CPropertySheet
{
public:
  OptionsDialog(UINT caption, CWnd* parentWnd);

protected:
  DECLARE_MESSAGE_MAP()

  void ChangeDialogFont(CWnd* wnd, CFont* font, double scale);

  RECT m_page;
  LOGFONT m_logFont;
  CFont m_font;

  int m_dpi;
  double m_fontHeightPerDpi;

public:
  virtual BOOL OnInitDialog();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LONG OnResizePage(UINT, LONG);
};

class OptionsDisplayPage : public CPropertyPage
{
public:
  OptionsDisplayPage();

// Dialog Data
  enum { IDD = IDD_OPTIONS_DISPLAY };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
  virtual BOOL OnInitDialog();
  virtual void OnOK();

private:
  // Called when enumerating fonts, and populates the font drop down lists in the dialog
  static int CALLBACK ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param);

  CComboBox m_propFont;
  CComboBox m_fixedFont;

public:
  CString m_propFontName;
  CString m_fixedFontName;
  CString m_fontSize;
  ColourButton m_textColour;
  ColourButton m_backColour;
  BOOL m_fastScroll;
  BOOL m_morePrompts;
  int m_leftMargin;
  int m_rightMargin;
};

class OptionsInterpreterPage : public CPropertyPage
{
public:
  OptionsInterpreterPage();

// Dialog Data
  enum { IDD = IDD_OPTIONS_INTERPRETER };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
  int m_interpreter;
  int m_reportErrors;
  BOOL m_expand;
  BOOL m_tandy;
  BOOL m_ignore;
  BOOL m_wrapScript;
  CString m_username;
};

class OptionsStartupPage : public CPropertyPage
{
public:
  OptionsStartupPage();

// Dialog Data
  enum { IDD = IDD_OPTIONS_STARTUP };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
  virtual BOOL OnInitDialog();

public:
  BOOL m_register;
  int m_iFiction;
};

class OptionsSpeechPage : public CPropertyPage
{
public:
  OptionsSpeechPage();

// Dialog Data
  enum { IDD = IDD_OPTIONS_SPEECH };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
  virtual BOOL OnInitDialog();

private:
  CComboBox m_voiceCtrl;
  CSliderCtrl m_rateCtrl;
  CString m_defaultVoice;

public:
  CString m_voice;
  BOOL m_speak;
  int m_rate;
};

class ScrollbackDialog : public BaseDialog
{
  DECLARE_DYNAMIC(ScrollbackDialog)

public:
  ScrollbackDialog(LPCWSTR text, int textLen, CWnd* pParent = NULL);
  virtual ~ScrollbackDialog();

// Dialog Data
  enum { IDD = IDD_SCROLLBACK };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LRESULT OnSameSizeAsMain(WPARAM, LPARAM);
  DECLARE_MESSAGE_MAP()

public:
  virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnCopy();

protected:
  LPCWSTR m_text;
  int m_textLen;
  int m_textTop;
  int m_dpi;
  CRichEditCtrl m_edit;
};

class GameFileDialog : public CFileDialog
{
  DECLARE_DYNAMIC(GameFileDialog)

public:
  static bool ShowDialog(CString& path, CWnd* parent);

private:
  GameFileDialog(LPCTSTR lpszFileName, CWnd* pParentWnd);
  void OnInitDone();

  afx_msg void OnUseQuetzal();
  DECLARE_MESSAGE_MAP()

  CResString m_title;
  bool m_quetzal;
};
