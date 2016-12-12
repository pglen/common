#if !defined(AFX_XBUTTON_H__F953B6E3_3796_4207_B3CB_FFE589D8822C__INCLUDED_)
#define AFX_XBUTTON_H__F953B6E3_3796_4207_B3CB_FFE589D8822C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// xButton.h : header file
//

#include "Popup.h"

/////////////////////////////////////////////////////////////////////////////
// CxButton window

class CxButton : public CButton
{
// Construction
public:
	CxButton();

// Attributes
public:

// Operations
public:

	CxButton& SetTooltip(const CString& strText);
	CxButton& SetTooltip(const char *strText);
	CxButton& SetTooltip(int id);

	CPopup		tip;

protected:

	// Tooltip related
	CString		tiptext;
	CPoint		mouse, capmouse;
	
	int			tiptime, capped, fired, tipcnt, maxtipcnt;

	void		xShowTip();
	void		xHideTip();
	void		xInitTip();

	BOOL		OnEraseBkgnd(CDC* pDC);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CxButton)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	void Some();
	virtual ~CxButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CxButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint(void);
		//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XBUTTON_H__F953B6E3_3796_4207_B3CB_FFE589D8822C__INCLUDED_)
