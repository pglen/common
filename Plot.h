
/* =====[ Plot.h ]========================================================== 
                                                                             
   Description:     The daco project, implementation of the Plot.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  3/11/2010  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#if !defined(AFX_PLOT_H__596E6728_5FF8_4D09_BC8A_E784A01707F2__INCLUDED_)
#define AFX_PLOT_H__596E6728_5FF8_4D09_BC8A_E784A01707F2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Plot.h : header file
//

#include <afxmt.h>
#include "intarr.h"
#include "xraym.h"

#define PLOT_MAGIC 0x232345

/////////////////////////////////////////////////////////////////////////////
// CPlot window

class CPlot : public CStatic
{

	// Construction
public:
	CPlot();

// Implementation
public:

	void	SetMag(double mm);
	double	GetMag();	
	void	ClearAll();
	void	AddMarker();
	void	AddLine(int hight);
	void	AddLine(double val);
	void	AddIntArr(CIntArr *parr);
	CIntArr * GetData() { return & xint_arr; };

	virtual ~CPlot();

	int		magic;

protected:

	double	xmag;

	int		xcformat, xhor_gap, xver_gap, lastmove, ytimer;
	int		xold_max, xscroll, xstep, xinited, xchanged, xtimed;
	
	int		xwasdouble;

	unsigned int lastclock;
	
	CCriticalSection arrloc;
	
	CString xlabel;
	CPoint	xoldpoint, lastpoint, downpoint;
	CIntArr	xint_arr, xmark_arr;
	
	CIntArr xarr, varr;

//////////////////////////////////////////////////////////////////////////
	
	void	xAddCommas(CString &str);
	void	xCreateMenu(CXrayM *pmenu);
	void	xAuto();
	void	xCopy();
	void	xPaste(int noclear = false);
	void	xSmooth(int fact = 1);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlot)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlot)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLOT_H__596E6728_5FF8_4D09_BC8A_E784A01707F2__INCLUDED_)
