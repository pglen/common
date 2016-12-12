
/* =====[ waveEdit.h ]========================================== 
                                                                             
   Description:     The daco project, implementation of the waveEdit.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  8/24/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#if !defined(AFX_WAVEEDIT_H__29427B20_09BD_11D6_90D8_00B04C390A3E__INCLUDED_)
#define AFX_WAVEEDIT_H__29427B20_09BD_11D6_90D8_00B04C390A3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment(lib, "winmm.lib")

#include "PlaySound.h"
#include "xraym.h"

// Just to keep it consistant
#define CWaveEdit CwaveEdit

/////////////////////////////////////////////////////////////////////////////
// CwaveEdit window

class CwaveEdit : public CStatic

{
	
public:

	void	PostHere(HWND wnd) {postto = wnd;}

	void	AddUndo();
	void	ZeroCross();

	int		SaveWave(const TCHAR *fname);
	int		OpenWave(const TCHAR *fname, int mruflag = true);
	
	void	Save();
	
	void	PlayBuff();
	void	WaitForEnd();

	void	ClearBuff();
	void	Cut();
	void	Paste();
	void	Copy();
	void	Del();
	
	void	Crop();
	void	SelAll();
	void	Norm(int maxfact = 10);
	void	Magnify(double factor);

	void	ClearUndo();
	void	Undo();
	
	void	AppendBuff(char *buff, int len, int undo = false);
	
	void	SetBuff(void* buff, int len);
	void	SetSel(int beg, int end);
	void	SetStartPos(int pos);
	
	void	GetSelBuff(TCHAR **pbuff, int *plen, int zcross = false);
	void	GetSel(int *ssel, int *esel);
	char	*GetBuff();
	int		GetBuffLen();
	int		GetStartLen();

	const TCHAR *	 GetFileName();
	const TCHAR*	 GetFullName();
	
	//////////////////////////////////////////////////////////////////////////

	CwaveEdit();
	virtual ~CwaveEdit();
	
	CString wavename, wavefname, oldpath;

	int		msgflag, msgval;

	CPlaySound		*pplaysnd;
	
// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CwaveEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual	void PreSubclassWindow();
	//}}AFX_VIRTUAL

	char	**soundbuff2;
	int		*soundlen2;

	int		right, bgcolor, selcolor, ipcolor, gridcol;
	int		changed;

protected:

	CMutex mx;	
	HWND	postto;
	int		cformat;
	
	//int		in_append;
	
	CXrayM		*lastmenu2;

	double	magnify;

	CPtrArray		undoptr;
	CDWordArray		undolen;

	CString			section, laststr;

	double	mag;
	
	LOGFONT	m_lf;
	CFont	m_font;

	CString	lastinfo, commstr;

	int		lasttimeout, lastpos, wleft;
	int		skip, fired, in_play, undolim;
	int		old_offset, old_marker, marker, in_mark; 
	int		startpos, first;
	int		oldx, oldrx, currpos;
	int		old_startsel, old_endsel, startsel, endsel;
	int		startsel2, endsel2;
	int		startlen, endlen, ipwleft;
	int		markstart, markend;
	int		markmove, moveoffset;
	int		nsize, playoffset, soundlen;

	char	*soundbuff;

	CDC			memDC;
	CBitmap		bm2;

	void	xShowInfo(CString &str, int timeout);
	void	xShowInfo(const TCHAR *str, int timeout);
	void	xFrameSel();
	void	xMyKeyDown(UINT wParam, UINT lParam);
	void	xMySysKeyDown(UINT wParam, UINT lParam);
	void	xDelayedInval();
	void	xPostSelChange();
	void	xMarkerOff();
	void	xMarker(int pos);
	int		xIsOKToGo();
	int		xIsPlaySnd();
	int		xIsNear(int x1,  int x2, int dist = 5);

	void	xConvCoord();
	void	xCreateMenu(CXrayM *pmenu);

	// Generated message map functions

protected:

	//{{AFX_MSG(CwaveEdit)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRcliclkPlay();
	afx_msg void OnRcliclkSaveFile();
	afx_msg void OnRcliclkSaveFile2();
	afx_msg void OnRcliclkSelectall();
	afx_msg void OnRcliclkCropselected();
	afx_msg void OnRcliclkSave();
	afx_msg void OnRcliclkOpenFile();
	afx_msg void OnRcliclkCutselected();
	afx_msg void OnRclickZcross();
	afx_msg void OnRclickCopy();
	afx_msg void OnRclickPaste();	
	afx_msg void OnRclickNewPaste();	
	afx_msg void OnRclickUndo();	
	afx_msg void OnRclickNorm();	
	afx_msg void OnRclickMRU(UINT cmd);	
	afx_msg void OnRclickDel();	
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEEDIT_H__29427B20_09BD_11D6_90D8_00B04C390A3E__INCLUDED_)
