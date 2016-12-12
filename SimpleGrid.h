#if !defined(AFX_CSimpleGrid_H__D7F8E078_7CAF_433D_82EF_99F635B77B04__INCLUDED_)
#define AFX_CSimpleGrid_H__D7F8E078_7CAF_433D_82EF_99F635B77B04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CSimpleGrid.h : header file
//

#include <afxmt.h>

#include "popup.h"

#define CELL_MAGIC	0x1234

/////////////////////////////////////////////////////////////////////////////
// CSimpleGrid window

class CSimpleGrid : public CWnd

{
	typedef struct _cell
		{
		int		magic;
		int		type;
		int		cellcol;
		int		cellrow;
		int		bold;
		int		italic;
		int		ww,hh;
		int		color;
		int		trans;
		int		check;
		int		state;
		int		dclick;
		int		nosel;
		void	*content;
		} cell;
	
public:

	static	int		change_message;
	static	int		check_message;
	static	int		dblclick_message;

// Construction

public:

	// Getters
	void	GetStr(int qrow, int qcol, CString &pstr);
	int		GetRowCount();
	int		GeColCount();
	int		GetCurrRow();
	void	SetCurrRow(int newrow);

	void	BoldCell(int qrow, int qcol, int flag);

	// Setters
	void	SetBGcolor(int col);
	int		GetCheck(int qrow, int qcol);
	void	SetCheck(int qrow, int qcol, int val);
	
	void	AddStr(int qrow, int qcol, const TCHAR *str, int inval = true);
	void	Clear();


	void	InvalidateCell(int row, int col);
	void	InvalidateGrid();
	void	InvalidateRow(int row);
	
	// Operations
	void	AutoSize();
	
// Attributes
public:

	int		first_bold, allowpopup, rowhigh;
	int		maxwidth;
	int		head_size;

protected:

	CCriticalSection lock;
	int		reenter;

	LOGFONT		m_lf;

	int	rows, cols;
	int	startrow, startcol;
	int	vrows, vcols;
	int xpendauto, xbackcolor;

// Operations
public:

	CSimpleGrid();
	virtual ~CSimpleGrid();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimpleGrid)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:

protected:

	cell	*RowCol2Cell(int row, int col);

	int		wasbold;
	int		currrow, currcol;
	
	int		init_row, init_col;
	int		cell_www, cell_hhh;

	int		capped, fired;
	int		sw, sh;

	int		xmaxrow, xmaxcol;

	cell	*cccc;
	
	//BCMenu	hrcmenu;
	//BCMenu	*hrclickmenu;

	RECT	charrc; 

	CPoint	mouse, capmouse;

	CPopup	*ptip;

	CPtrArray	xrowarr;
	
	CFont m_font, m_fontb;

	CScrollBar  vs, hs;

	void Resized();
	void Pt2Cell(CPoint point, int *rowp, int *colp, CRect *rc);
	void SetCurr(CPoint  point);
	void ReAlloc(int newrow, int newcol);
	void FirstPaint();	
	
	int		init_alloc;
	int		firstpaint;

	// Generated message map functions
protected:
	//{{AFX_MSG(CSimpleGrid)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSimpleGrid_H__D7F8E078_7CAF_433D_82EF_99F635B77B04__INCLUDED_)
