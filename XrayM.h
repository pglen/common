
/* =====[ XrayM.h ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the XrayM.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  10/31/2008  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#if !defined(AFX_XRAYM_H__07A7BC02_F12C_4376_86D4_89F1AD2BADD8__INCLUDED_)
#define AFX_XRAYM_H__07A7BC02_F12C_4376_86D4_89F1AD2BADD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// XrayM.h : header file
//

#include "label.h"
//#include "transslider.h"
#include "GrayDlg.h"
#include "popup.h"

#define MENU_MAGIC		0x345345
#define MENUITEM_MAGIC	0x342245

// The compiler needs a hint
class CXrayM;

class CXrayMitem 

{
	friend class CXrayM;

public:
	
	CXrayMitem();
	CXrayMitem(const TCHAR *ss);
	~CXrayMitem();
	
	int		command;
	TCHAR	key[32];

protected:

	CXrayM		*submenu;
	
	int			magic, selected;
	int			status, checked, intval;

	CString		*itemstr, *tipstr, *tooltip;

	HBITMAP		bitmap;
	RECT		rect;	

	void	xInitVars();

};

/////////////////////////////////////////////////////////////////////////////
/// CXrayM Dialog. 
///

/// The windows menu got replaced becase of some ugly inconsistancies. <br>
/// Ex: Popup did not inherit color scheme, dialog lifetime 
/// did not allow retrieval after menu disappeared, leaky resource management
/// ...<br>
/// This replacement menu is eazy ... 
/// just call LoadAndShow(Resoure, skip, pointX, pointY) to work it.
///
/// This menu obey/load correct menu font and assume the XrayNotes 
/// theme correctly. This file is open sourced during development.
///

class CXrayM : public CGrayDlg

{
	static		CPtrArray wndarr;
	
public:

	static		int		_dlgbackcol,  _highcol;
	static		void	_FreeLast();
	
public:

	int			magic;

	void		LoadAndShow(int nResource, int skip = 0, int xx = -1, int yy = -1);
	BOOL		LoadMenu(int nResource, int skip = 0);
	BOOL		LoadMenu(LPCTSTR lpszResourceName, int skip = 0);
	void		Show(int xx, int yy, int sub = 0);
	void		Show();
	void		Hide(int special = false);

	void		DelMenuItem(int idx);

	int			GetNumItems();
	void		GetMenuItemText(int idx, CString &txt);
	int			GetMenuItemInt(int idx);
	
	void		SetToolTip(const TCHAR *str, const TCHAR *tipstr);
	void		SetToolTip(int idx, const TCHAR *tipstr);
	void		SetItemKey(int idx, const TCHAR *newstr);
	void		SetItemText(int idx, const TCHAR *newstr);
	void		SetItemInt(int idx, int val);
	void		SetCheck(int idx, int check);
	int			SetItemStatus(int idx, int status);

	CXrayM		*FindSubmenu(const TCHAR *str);
	CXrayM		*FindSubmenu(int idx);
	
	int			FindMenuItem(const TCHAR *str);
	int			FindMenuItem(int command);

	void		Clear();
	int			AppendMenuItem(const TCHAR *str, int command = -1, int status = MF_ENABLED, int checked = false);
	int			AppendSubMenu(const TCHAR *str, CXrayM *submenu, int status = MF_ENABLED);
		
	void		DumpMenu(CXrayM *menu, CString indent = "");
	
	CWnd		*sendto;

	COLORREF	highcol, lowlowcol, lowcol, dlgbackcol, checkcol;
	COLORREF	disabcol, grayedcol, textcolor;

	int			closeonup, ontop;

	// Construction
	CXrayM(CWnd* pParent = NULL);				// standard constructor
	CXrayM(int iddx, CWnd* pParent = NULL); 		

	CXrayM::~CXrayM();

	// Expose this as public, so it can be maniputated directly
	CPtrArray	arr;

	int			limit_mon;

protected:

	void		xAutoPos(int *pxx, int *pyy, int ww, int hh);
	void		xInitVars();
	int			xGotoPrev();
	int			xGotoNext();

	int			xGetSelected();
	int			xOpenSubMenu(CXrayMitem *item, CPoint &point);
	int			xExecItem(CXrayMitem *item);
	int			xIsAnySelected();
	int			xIsItemSelectAble(CXrayMitem *item);
	CWnd		*xSafeGetMainWnd();	
	DWORD		xUnicodeToAnsi(LPWSTR pszW, CString &str);
	void		xScanForAction(CPoint &point, int NoClick);
	void		xMeasureMenu(int *ww, int *hh);
	int			xGetMenuTextSize(const TCHAR *str, SIZE &size);
	
	CPopup		xtip;

	BOOL		xloaded;
	int			xoldloop, xfired, xtextheight, gap, leftgap, topgap;

	POINT	xpp[4];

	CXrayM	*subopen, *mparent;
	CPoint	xlastmouse, capmouse;
	
	CFont	m_fontMenu;
	
// Dialog Data
	//{{AFX_DATA(CXrayM)
	enum { IDD = 0 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXrayM)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CXrayM)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XRAYM_H__07A7BC02_F12C_4376_86D4_89F1AD2BADD8__INCLUDED_)
