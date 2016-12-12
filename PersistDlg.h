
/* =====[ PersistDlg.h ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the PersistDlg.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  11/7/2007  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#if !defined(AFX_PERSISTDLG_H__3B62E52E_E649_4715_9316_A96327899DD1__INCLUDED_)
#define AFX_PERSISTDLG_H__3B62E52E_E649_4715_9316_A96327899DD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PersistDlg.h : header file
//

#include "support.h"
#include "xPtrArr.h"

#define PERSIST_MAGIC 0x2345432

#ifndef INITBLEND

#define INITBLEND(aa, bb)							\
		aa.BlendOp	= AC_SRC_OVER;					\
		aa.BlendFlags = 0;							\
		aa.SourceConstantAlpha = min(bb, 255);		\
		aa.AlphaFormat = 0;							\

#endif


/////////////////////////////////////////////////////////////////////////////
// PersistDlg dialog

class PersistDlg : public CDialog

{
	static CString section;
	static	int inmodal;

	static	DLGTEMPLATE PersistDlg::tmpl; 
	static	DWORD		PersistDlg::padarr[3];

public:

	static	CxPtrArr modals;
	static	int resetpositions;

public:

	void	MoveToPos(int xx = 0, int yy = 0);
	
	int		oldAlpha;
	int		want_x, want_y;
	int		want_w, want_h;
	int		noremember;
	int		modal;
	
	CWnd	*parent;
	CString windowname;

protected:

	int		m_centerdlg;
	int		bAlpha;
	int		pre_want_x, pre_want_y;
	int		pre_want_w, pre_want_h;
	
	int		showstate, 	pre_showstate;
	int		posload, rdown;
	int		destroyed, shown;
	int		xdrag, ydrag;

	lpfnSetLayeredWindowAttributes g_pSetLayeredWindowAttributesx;

	int		xidd;
	int		magic;
	
	CPoint oldpoint;
	
public:
	
	////////////////////////////////////////////////
	// Functions

	void	SetCenterFlag(int flag);
	void	SetAlpha(int alpha);

	~PersistDlg();

	virtual	int		DoModal();
	
	virtual	void	Hide(int ret = IDOK);
	virtual	void	Show(int center = false);

	PersistDlg(CWnd* pParent = NULL);   // standard constructor
	PersistDlg(UINT nIDTemplate, CWnd* pParentWnd = NULL);

// Dialog Data
	//{{AFX_DATA(PersistDlg)
	enum { IDD = 1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PersistDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

protected:

	void	xCreate();

//private:
	void	InitVars();

protected:
	// Generated message map functions
	//{{AFX_MSG(PersistDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERSISTDLG_H__3B62E52E_E649_4715_9316_A96327899DD1__INCLUDED_)
