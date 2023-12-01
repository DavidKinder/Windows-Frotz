/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz frame window class
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "MenuBar.h"
#include "Resource.h"

#define DARKMODE_REGISTRY "Software\\David Kinder\\Frotz"

class FrotzWnd;

class FrotzFrameWnd : public MenuBarFrameWnd
{
  DECLARE_DYNAMIC(FrotzFrameWnd)

public:
  FrotzFrameWnd();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(FrotzFrameWnd)
  protected:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
  virtual void GetMessageString(UINT nID, CString& rMessage) const;
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~FrotzFrameWnd();

  // Create the frame
  bool Create(bool toolbar, bool statusbar);
  // Get the client window
  FrotzWnd* GetClientWnd(void);
  // Update the game defined menus
  void UpdateMenus(CArray<CStringArray,CStringArray&>& menus);
  // Remove any game defined menus
  void ResetMenus(void);
  // Set the currently active modal dialog
  void SetModalDialog(CWnd* dialog);

  // Show the HTMLHelp format help file
  virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);

// Generated message map functions
protected:
  //{{AFX_MSG(FrotzFrameWnd)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnDestroy();
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
  afx_msg LRESULT OnInputLangChange(WPARAM wParam, LPARAM lParam);
  afx_msg void OnUpdateTime(CCmdUI* pCmdUI);
  afx_msg void OnUpdateZcode(CCmdUI* pCmdUI);
  afx_msg void OnEditPaste();
  afx_msg void OnHotDebug();
  afx_msg void OnHotHelp();
  afx_msg void OnHotPlayback();
  afx_msg void OnHotQuit();
  afx_msg void OnHotRecord();
  afx_msg void OnHotRestart();
  afx_msg void OnHotSeed();
  afx_msg void OnHotUndo();
  afx_msg void OnFullscreen();
  afx_msg void OnReadLineStart();
  afx_msg void OnReadLineEnd();
  afx_msg void OnReadLineDel();
  afx_msg void OnReadLineBack();
  afx_msg void OnReadLineForward();
  afx_msg void OnReadLineRubout();
  afx_msg void OnReadLineKill();
  afx_msg void OnHelpLink(UINT nID);
  afx_msg void OnHelpTranslateLink(UINT nID);
  //}}AFX_MSG
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

  DECLARE_MESSAGE_MAP()

protected:
  CRect GetDefaultSize(void);

protected:
  FrotzWnd* m_clientWnd;
  CWnd* m_modalDialog;
  DarkModeStatusBar m_statusBar;
  CRect m_normalSize;
  UINT m_codePage;
  int m_dpi;
};
