#if !defined(AFX_POPUP_H__6CDEC48B_CA86_4255_9FAF_568533840B37__INCLUDED_)
#define AFX_POPUP_H__6CDEC48B_CA86_4255_9FAF_568533840B37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Popup.h : header file
//
//include "label.h"


/////////////////////////////////////////////////////////////////////////////
// CPopup dialog

class CPopup : public CEdit

{

	static	DLGTEMPLATE tmpl; 
	static	DWORD		padarr[3];

// Construction
public:

	void	Hide();
	void	Show();
	void	Move(CPoint screen);
	void	SetText(const TCHAR *str);
	void	Show(const TCHAR *str, CPoint screen);

	void	Create() { xCreate(); } ;

	CString	GetText();
	
	CPopup();   // standard constructor
	CPopup::CPopup(int rc);

	int		padding, idletime, tipped;
	int		rcid;

	COLORREF	bgcolor;

protected:

	void		xCreate();
	void		xPopAutoSize();
	void		xPopAutoPos();
	void		xInitVars();

	CPoint		here;
	CSize		size;

	CWnd		*parent;
	int			first;
	
	CString		str;
	CFont		m_font, m_fontb;
	LOGFONT		m_lf;

// Dialog Data
	//{{AFX_DATA(CPopup)
	//enum { IDD = IDD_TIP_DLG };
	//CLabel	m_str;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopup)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPopup)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	//afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUP_H__6CDEC48B_CA86_4255_9FAF_568533840B37__INCLUDED_)
