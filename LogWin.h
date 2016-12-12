#if !defined(AFX_LOGWIN_H__CD34F076_43EE_4915_8945_D259D158C695__INCLUDED_)
#define AFX_LOGWIN_H__CD34F076_43EE_4915_8945_D259D158C695__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogWin.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogWin window CWnd
//class CLogWin : public CEdit
//class CLogWin : public CStatic

#define	DEFAULT_MAX_LINES 5000

class CLogWin : public CWnd

{
// Construction
public:

	static LRESULT CALLBACK MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam);    // second message parameter


public:
	
	void AddString(const char *str);
	void AddDelimStr(CString *pstr, int lenline = 36);
	void AddNewLine();
	void Clear();
	void printf(const char *Format, ...);

protected:

	char		OutString[1024];

	int			created;
	LOGFONT		m_lf;
	CFont		m_font;
	CStringList strlist;
	int			inited;

	int			lines, in_timer;

	int			prevnl;
	int			bgcolor;
	//int			maxline;

// Attributes
public:

	int			maxlines;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogWin)
	public:
	protected:
	virtual void PreSubclassWindow();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:

	CLogWin();
	void CreateFrom(CWnd *wnd, int id);
	virtual ~CLogWin();

	// Generated message map functions
protected:
	//{{AFX_MSG(CLogWin)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGWIN_H__CD34F076_43EE_4915_8945_D259D158C695__INCLUDED_)
