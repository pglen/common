#if !defined(AFX_LED_H__885B0787_4BB4_4BAA_B1F8_D1A5A87597B7__INCLUDED_)
#define AFX_LED_H__885B0787_4BB4_4BAA_B1F8_D1A5A87597B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Led.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLed window

class CLed : public CStatic
{
// Construction
public:
	CLed();
	int bgcolor;
// Attributes
public:

	void	DelayedInval();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLed)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetState(int ss);
	virtual ~CLed();

	// Generated message map functions
protected:

	int state, fired;
	int	lightcol, lightcol2, lightcol3;
	int	bodycol;
	
	//{{AFX_MSG(CLed)
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LED_H__885B0787_4BB4_4BAA_B1F8_D1A5A87597B7__INCLUDED_)
