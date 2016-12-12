
/* =====[ label.h ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the label.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  10/27/2008  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#if !defined(AFX_LABEL_H__A4EABEC5_2E8C_11D1_B79F_00805F9ECE10__INCLUDED_)
#define AFX_LABEL_H__A4EABEC5_2E8C_11D1_B79F_00805F9ECE10__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Label.h : header file
//

//#include "Popup.h"

#ifndef TSIZEOF
// Sizeof variable in TCHAR
#define TSIZEOF(xx)      (sizeof (xx) / sizeof (TCHAR))
// Convert to / from textsize
#define TXT2BYTE(xx)      ((xx) * sizeof (TCHAR))
#define BYTE2TXT(xx)      ((xx) / sizeof (TCHAR))
#endif


#define STRTMP	1024

/////////////////////////////////////////////////////////////////////////////
// CLabel window


class CLabel : public CStatic

{
	DECLARE_DYNAMIC(CLabel)

	static LRESULT CALLBACK CLabel::MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam);    // second message parameter

	enum FlashType {None, Text, Background };


// Construction
public:

	CLabel();
	virtual ~CLabel();

	void GetWindowText( CString& rString ) ;

	CLabel&		SetBkColor(COLORREF crBkgnd);
	COLORREF	GetBkColor();
	
	CLabel& SetTextColor(COLORREF crText);

	CLabel& SetText(const CString& strText, int recalc = false);
	CLabel& SetTextP(const TCHAR * strText, ...);

	CLabel& SetTooltip(const CString& strText);
	CLabel& SetTooltip(const TCHAR *strText);

	CLabel& SetFontName(const CString& strFont);
	CLabel& SetFontSize(int nSize);

	CLabel& SetFontBold(BOOL bBold = true);
	CLabel& SetFontUnderline(BOOL bSet = true);
	CLabel& SetFontItalic(BOOL bSet = true);
	
	CLabel& SetSunken(BOOL bSet = true);
	CLabel& SetBorder(BOOL bSet = true);
	
	CLabel& FlashText(BOOL bActivate);
	CLabel& FlashBk(BOOL bActivate);
	
	CLabel& SetLink(BOOL bLink);
	CLabel& SetLinkCursor(HCURSOR hCursor);
	
	CLabel& operator =(const TCHAR *str);

	operator const TCHAR * ();
	
	const TCHAR *CLabel::GetText();

protected: 

	CSize		textsize;

	int			m_inited, m_transparent;
	int			m_center, m_right, m_vcenter;
	int			m_frame, m_edge, m_modframe;

	CString		m_text, m_backup;
		
	//CPopup		tip;
	CString		tiptext;
	CPoint		mouse, capmouse;
	
	int			tiptime, capped, fired, tipcnt, maxtipcnt;

	void		xShowTip();
	void		xHideTip();
	void		xInitTip();


	////////////////////////////////////////////////

	void		xInitLab();
	void		xReDraw();
	void		xReDrawParent();
	void		xReconstructFont();
	void		xCalcSize();

	COLORREF	m_bkcolor, m_crText;
	
	HBRUSH		m_hBrush, m_hwndBrush;
	
	LOGFONT		m_lf;
	CFont		m_font;

	BOOL		m_border;
	BOOL		m_bState;
	BOOL		m_bTimer;
	BOOL		m_bLink;
	FlashType	m_Type;
	HCURSOR		m_hCursor;

	CRect		orig;

public:
	
	void SetWindowText(CString &str);
	void SetWindowText(const TCHAR *str);

	void DumpExStyles();
	void DumpSS();
	void DumpStyles();

	// Implementation
		
	void SetTrans(int flag = true);
	void CenterText(int flag = true);
	void CenterTextV(int flag = true);
	void GetText(CString &str);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLabel)
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	//}}AFX_VIRTUAL


	// Generated message map functions
protected:
	//{{AFX_MSG(CLabel)
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint(void);
	afx_msg	void OnMouseMove(UINT nFlags, CPoint point);

	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LABEL_H__A4EABEC5_2E8C_11D1_B79F_00805F9ECE10__INCLUDED_)
