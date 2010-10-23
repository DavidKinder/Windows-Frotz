/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz application class
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "ColourButton.h"
#include "Dialogs.h"
#include "Dib.h"
#include "Resource.h"

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

protected:
  static DWORD CALLBACK StreamInCB(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
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
  DECLARE_MESSAGE_MAP()

protected:
  CRichInfo m_info;
  CButton m_ok;

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
  DECLARE_MESSAGE_MAP()

public:
  virtual BOOL OnInitDialog();
  afx_msg void OnResizeInfo(NMHDR *pNMHDR, LRESULT *pResult);

protected:
  CStatic m_logo;
  CRichInfo m_info;
  CStatic m_border;
  CButton m_ok;
};

class OptionsDialog : public CPropertySheet
{
public:
  OptionsDialog(UINT caption, CWnd* parentWnd);

protected:
  DECLARE_MESSAGE_MAP()

  void ChangeDialogFont(CWnd* wnd, CFont* font);

  RECT m_page;
  LOGFONT m_logFont;
  CFont m_font;

public:
  virtual BOOL OnInitDialog();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
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
  CString m_v6Scale;
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
  ScrollbackDialog(const char* text, CWnd* pParent = NULL);
  virtual ~ScrollbackDialog();

// Dialog Data
  enum { IDD = IDD_SCROLLBACK };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  DECLARE_MESSAGE_MAP()

public:
  virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnCopy();

protected:
  const char* m_text;
  int m_textTop;
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
