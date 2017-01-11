
/* =====[ GrayWin.cpp ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the GrayWin.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  10/29/2008  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

// GrayWin.cpp: implementation of the CGrayDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GrayDlg.h"
#include "mxpad.h"
#include "wingdi.h"

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[] = _T(__FILE__);
#define new DEBUG_NEW
#endif

#pragma comment(lib, "msimg32.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrayDlg::CGrayDlg(CWnd* pParent /*=NULL*/)
	: PersistDlg(CGrayDlg::IDD, pParent)
{
	InitVars();
}

CGrayDlg::CGrayDlg(UINT nIDTemplate, CWnd* pParentWnd)
	: PersistDlg(nIDTemplate, pParentWnd)

{
	InitVars();
}

CGrayDlg::~CGrayDlg()

{
	//if(!destroyed)
	//	P2N(_T("CGrayDlg::~CGrayDlg() Not destroyed! %p %s\r\n"), this, m_title);

	//P2N(_T("CGrayDlg::~CGrayDlg() %p %s\r\n"), this, m_title);
}

BEGIN_MESSAGE_MAP(CGrayDlg, PersistDlg)
	//{{AFX_MSG_MAP(CGrayDlg)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_SIZE()	
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CGrayDlg::OnDestroy()

{
	destroyed = true;
	//P2N(_T("CGrayDlg::OnDestroy() IDD=%d %s\r\n"), xidd, windowname);
	PersistDlg::OnDestroy();
}

BOOL CGrayDlg::OnInitDialog() 

{
	PersistDlg::OnInitDialog();
	return true;
}

void CGrayDlg::OnPaint() 

{
	CPaintDC dc(this); // device context for painting
	
	RECT rc; GetClientRect(&rc);
	int save2 = dc.SaveDC();

	// Draw header
	if(!noheader)
		{
		CDC dcMem; dcMem.CreateCompatibleDC(&dc);

		int save = dcMem.SaveDC();

		HBITMAP bm =::CreateCompatibleBitmap(dc.GetSafeHdc(),
				titlerect.Width(), titlerect.Height());
		HBITMAP old = (HBITMAP) SelectObject(dcMem, bm);

		CRect rc(0, 0, titlerect.Width(), titlerect.Height()); 
	
		if(GetActiveWindow() == this)		
			dcMem.FillSolidRect(&rc, dheadcol);
		else
			dcMem.FillSolidRect(&rc, headcol);

		// Alpha blend header
		BLENDFUNCTION m_bf; INITBLEND(m_bf, 150);

		AlphaBlend(dc.GetSafeHdc(),
			titlerect.left, titlerect.top,
				titlerect.Width(), titlerect.Height(),
					dcMem, 0, 0, 
						titlerect.Width(), titlerect.Height(), m_bf);

		//dc.SetTextColor(m_crText);
		dc.SetBkMode(TRANSPARENT);

		// Draw Header
		CRect rcc(titlerect); rcc.top += 2; rcc.left += 2;
		
		CFont* oldFont = dc.SelectObject(&m_font);

		dc.DrawText(m_title, m_title.GetLength(), rcc, DT_LEFT);
		dc.SelectObject(oldFont);

		dcMem.RestoreDC(save);

		SelectObject(dcMem, old);

		DeleteObject(bm);




			if(!noclosebox)
		{
		// Draw corner close	
		if(hiclose)
			dc.FillSolidRect(closerect, RGB(96, 96, 96));
		else
			dc.FillSolidRect(closerect, RGB(32, 32, 32));

		CPen pen, *oldpen;
		pen.CreatePen(PS_SOLID, 2, RGB(255,255,255));
		oldpen = (CPen*)dc.SelectObject(pen);

		// Draw cross
		dc.MoveTo( closerect.left + 2, closerect.top + 2);
		dc.LineTo( closerect.right - 2, closerect.bottom -2);

		dc.MoveTo( closerect.left + 2, closerect.bottom - 2);
		dc.LineTo( closerect.right - 2, closerect.top + 2);

		}

		if(!nominbox)
			{
			// Draw minimize box	
			if(himin)
				dc.FillSolidRect(minrect, RGB(96, 96, 96));
			else
				dc.FillSolidRect(minrect, RGB(32, 32, 32));

			CPen pen, *oldpen;
			pen.CreatePen(PS_SOLID, 2, RGB(255,255,255));
			oldpen = (CPen*)dc.SelectObject(pen);

			// Draw underline
			dc.MoveTo( minrect.left + 2, minrect.bottom - 3);
			dc.LineTo( minrect.right - 3, minrect.bottom -3);
			}

		if(!noupbox)
			{
			// Draw minimize box
			if(hiup)
				dc.FillSolidRect(uprect, RGB(96, 96, 96));
			else
				dc.FillSolidRect(uprect, RGB(32, 32, 32));

			CPen pen, *oldpen;
			pen.CreatePen(PS_SOLID, 2, RGB(255,255,255));
			oldpen = (CPen*)dc.SelectObject(pen);

			// Draw underline
			dc.MoveTo( uprect.left + 2, uprect.top + 3);
			dc.LineTo( uprect.right - 3, uprect.top + 3);
			}
		}



	dc.RestoreDC(save2);

	PersistDlg::OnPaint();

	// Do not call CGrayDlg::OnPaint() for painting messages
}

//////////////////////////////////////////////////////////////////////////

void CGrayDlg::OnLButtonDown(UINT nFlags, CPoint point) 

{
	//P2N(_T("CGrayDlg::OnLButtonDown %d %d\r\n"), point.x, point.y);

	RECT rc; GetClientRect(&rc);
	
	int sendnc = false;

	// In header?
	if(titlerect2.PtInRect(point))		
		{
		//P2N(_T("CGrayDlg::OnLButtonDown Click in Header box\r\n"));
	
		// Assume move in header
		sendnc = true;

		// Did we hit an active button?
		if(!noclosebox)
			if(closerect2.PtInRect(point) == 0)
				{				
				}
			else
				sendnc = false;

		if(!nominbox)
			if(minrect2.PtInRect(point) == 0)
				{				
				}
			else
				sendnc = false;

		if(!noupbox)
			if(uprect2.PtInRect(point) == 0)
				{				
				}
			else
				sendnc = false;
		}

	PersistDlg::OnLButtonDown(nFlags, point);

	//P2N(_T("CGrayDlg::OnLButtonDown sendnc=%d nomove=%d \r\n"), sendnc, nomove);
	
	if(sendnc && !nomove)
		{
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));	
		//P2N(_T("CGrayDlg::OnLButtonDown sendnc \r\n"));
		}
	
}

void CGrayDlg::OnLButtonUp(UINT nFlags, CPoint point) 

{
	RECT rc; GetClientRect(&rc);
	
	//P2N(_T("CGrayDlg::OnLButtonUp x=%d y=%d\r\n"), point.x, point.y);	

	if(!noclosebox && closerect2.PtInRect(point) )
		{
		//P2N(_T("CGrayDlg::OnLButtonUp - Click in Close Box\r\n"));	
		PersistDlg::OnLButtonUp(nFlags, point);
		BeforeClose(IDCANCEL);
		//Hide(IDCANCEL);
		return;
		}

	if(!nominbox && minrect2.PtInRect(point))
		{
		PersistDlg::OnLButtonUp(nFlags, point);		
		Minimize();
		return;
		}

	if(!noupbox && uprect2.PtInRect(point))
		{
		PersistDlg::OnLButtonUp(nFlags, point);		
		Panelize();
		return;
		}

	PersistDlg::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

void CGrayDlg::OnSize(UINT nType, int cx, int cy) 

{
	PersistDlg::OnSize(nType, cx, cy);
	
	// Evaluate head/closerect
	RECT	rc3; GetClientRect(&rc3);

	closerect.top = rc3.top + 2;
	closerect.left = rc3.right - (titleheight + 2);
	closerect.bottom = closerect.top + titleheight;
	closerect.right = closerect.left + titleheight;

	// Hittest with generocity
	closerect2 = closerect; closerect2.InflateRect(2, 2);

	// Fill in minrect dimentions
	minrect.top = rc3.top + 2;
	minrect.left = rc3.right - 2 * (titleheight + 2);
	minrect.bottom = minrect.top + titleheight;
	minrect.right = minrect.left + titleheight;

	// Hittest with generocity
	minrect2 = minrect; minrect2.InflateRect(2, 2);

	titlerect.top = 2;
	titlerect.left = 2;
	titlerect.bottom = titlerect.top + (titleheight + 1);
	titlerect.right = rc3.right - 2;	

	// Hittest with generocity
	titlerect2 = titlerect;	titlerect2.InflateRect(2,2);

	uprect.top = 2;
	uprect.left = rc3.right - 3 * (titleheight + 2) - 10;
	uprect.bottom = uprect.top + (titleheight + 1);
	uprect.right = uprect.left + titleheight; 	

	// Hittest with generocity
	uprect2 = uprect;	uprect2.InflateRect(2,2);	
}


void CGrayDlg::OnSetFocus(CWnd* pOldWnd) 

{
	//P2N(_T("CGrayDlg::OnSetFocus\r\n"));
	PersistDlg::OnSetFocus(pOldWnd);
}

void CGrayDlg::OnKillFocus(CWnd* pNewWnd) 

{
	//P2N(_T("CGrayDlg::OnKillFocus\r\n"));
	PersistDlg::OnKillFocus(pNewWnd);
}

void CGrayDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 

{
	//P2N(_T("CGrayDlg::OnActivate %d\r\n"), nState);
	PersistDlg::OnActivate(nState, pWndOther, bMinimized);	
	InvalidateRect(titlerect);
	//Invalidate();
}

//////////////////////////////////////////////////////////////////////////

void CGrayDlg::InitVars()

{
	nstate = 0;
	
	LOGFONT		m_lf;

	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), 
											sizeof(m_lf), &m_lf);
	m_lf.lfHeight = -12; m_font.CreateFontIndirect(&m_lf);

	dheadcol	= RGB(64, 64, 64);	headcol		= RGB(128, 128, 128);
	titleheight = 16;	
	nof4exit = nominbox = noclosebox = noheader = nomove = false;
	m_tracking = himin = hiup = hiclose = false;
	noupbox = nominbox = true;
	
	//bAlpha = 255;
}

//////////////////////////////////////////////////////////////////////////
// Filter alt keys for this dialog

int CGrayDlg::IsDlgKey(MSG *pMsg)

{
	int ret = 0;
	
	if(	pMsg->message == WM_SYSKEYDOWN ||
			pMsg->message == WM_SYSKEYUP /* ||
				pMsg->message == WM_SYSTCHAR */)
		{

		//P2N(_T("CGrayDlg::IsDlgKey\r\n"));

		CWnd *win, *win2;
		win = win2 = GetNextDlgTabItem(NULL);

		while(true)
			{
			CString text;
			win2->GetWindowText(text);

			int idx = text.Find(_T("&"));
			if(idx >= 0)
				{
				//P2N(_T("Dialog text: '%s' %c %c \r\n"), 
				//		text, toupper(text[idx+1]), toupper(pMsg->wParam));
				
				if(toupper(text[idx+1]) == toupper(pMsg->wParam) ) 
					{
					ret = true;	
					break;
					}
				}
			win2 = GetNextDlgTabItem(win2);

			// Back to the beginning of list
			if(win2 == win)
				break;
			}
		//P2N(_T("CGrayDlg::IsDlgKey %d %s\r\n"), ret, mxpad.num2msg(pMsg->message));
		}

	return ret;
}

BOOL CGrayDlg::PreTranslateMessage(MSG* pMsg) 

{
	// Send SYSTEM messages to main window

	//if(mxpad.is_junk(pMsg->message) == 0)
	//	P2N(_T("CGrayDlg::PreTranslateMessage(MSG* pMsg) message=%s\r\n"), mxpad.num2msg(pMsg->message)); 

	//if(IsDialogMessage(pMsg))
	//	return true;

#if 0
	if(	pMsg->message == WM_SYSKEYDOWN ||
			pMsg->message == WM_SYSKEYUP ||
				pMsg->message == WM_SYSTCHAR )
		{
		// Filter keys that do not belong to this one	
		if(!IsDlgKey(pMsg))
			{
			//P2N(_T("CGrayDlg::PreTranslateMessage - Dialog ALT key %d %c\r\n"),
			//	pMsg->wParam, pMsg->wParam);

			// ALT-F4
			if(pMsg->wParam == 115)
				{
				if(!nof4exit)
					Hide(IDCANCEL);
				}
			//return PersistDlg::PreTranslateMessage(pMsg);
			//return 0;
			}
		}
#endif

#if 0

	if(pMsg->message == WM_KEYDOWN)
		{
		P2N(_T("CGrayDlg::PreTranslateMessage\r\n"));

		OnKeyDown(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 		
		//return KeyEvent(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
		return 0;
		}
	if(pMsg->message == WM_KEYUP)
		{
		OnKeyUp(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 		
		//return KeyEvent(WM_KEYUP, pMsg->wParam, pMsg->lParam);
		}
	if(pMsg->message == WM_CHAR)
		{
		OnChar(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		//return KeyEvent(WM_CHAR, pMsg->wParam, pMsg->lParam);
		}

	// -------------------------------------------------------------------

	if(pMsg->message == WM_SYSCHAR)
		{
		OnSysChar(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 		
		//return KeyEvent(WM_SYSTCHAR pMsg->wParam, pMsg->lParam);
		}
	if(pMsg->message == WM_SYSKEYDOWN)
		{
		OnSysKeyDown(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		//return KeyEvent(WM_SYSKEYDOWN, pMsg->wParam, pMsg->lParam);
		}
	if(pMsg->message == WM_SYSKEYUP)
		{
		OnSysKeyUp(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		//return KeyEvent(WM_SYSKEYUP, pMsg->wParam, pMsg->lParam);
		}
#endif

	
	return PersistDlg::PreTranslateMessage(pMsg);
}

void CGrayDlg::Show(int center)

{
	//P2N(_T("CGrayDlg::Show() %p '%s' '%s' want_x=%d, want_y=%d\r\n"), 
	//					this, m_title, windowname, want_x, want_y);

	PersistDlg::Show();	
}


void CGrayDlg::SetWindowText(LPCTSTR lpszString)

{
	//P2N(_T("CGrayDlg::SetWindowText '%s'\r\n"), lpszString);

	m_title = lpszString;

	PersistDlg::SetWindowText(lpszString);

	// If it is a window, show changes
	if(IsWindow(m_hWnd))
		InvalidateRect(&titlerect);
}

//////////////////////////////////////////////////////////////////////////

void CGrayDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 

{
	//P2N(_T("CGrayDlg::OnKeyDown %d\r\n"), nChar);
	PersistDlg::OnKeyDown(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////////////

void CGrayDlg::Minimize()

{
	P2N(_T("CGrayDlg::Minimize()\r\n"));

	WINDOWPLACEMENT wp;  GetWindowPlacement(&wp);
	wp.showCmd = SW_MINIMIZE;
	SetWindowPlacement(&wp);
}

void CGrayDlg::Panelize()
{
	P2N(_T("CGrayDlg::Panelize())\r\n"));
}

void CGrayDlg::OnOK()
{
	//P2N(_T("CGrayDlg::OnOK()\r\n"));
	UpdateData();
	BeforeClose(IDOK);
}

void CGrayDlg::BeforeClose(int ret)

{
	//P2N(_T("CGrayDlg::BeforeClose(int ret=%d)\r\n"), ret);
	EndDialog(ret);
}

void CGrayDlg::OnCancel()
{
	//P2N(_T("CGrayDlg::OnCancel()\r\n"));
	UpdateData();
	BeforeClose(IDCANCEL);
}

void CGrayDlg::OnClose() 

{
	//P2N(_T("CGrayDlg::OnClose() \r\n"));
	OnCancel();
}

void CGrayDlg::OnMouseMove(UINT nFlags, CPoint point) 

{
	//P2N(_T("CGrayDlg::OnMouseMove() %d %d \r\n"), point.x, point.y);

	if (!m_tracking) 
		{
        TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT),
                TME_LEAVE, m_hWnd, 0 };

        if (::_TrackMouseEvent(&tme))
			{
            m_tracking = true;
			}
		}

	if(!noclosebox && closerect2.PtInRect(point) )
		{
		if(!hiclose)
			hiclose = true,	InvalidateRect(&closerect2);
		}
	else
		{
		if(hiclose)
			hiclose = false, InvalidateRect(&closerect2);
		}

	if(!nominbox && minrect2.PtInRect(point) )
		{
		if(!himin)
			himin = true,	InvalidateRect(&minrect2);
		}
	else
		{
		if(himin)
			himin = false, InvalidateRect(&minrect2);
		}

	if(!noupbox && uprect2.PtInRect(point) )
		{
		if(!hiup)
			hiup = true,	InvalidateRect(&uprect2);
		}
	else
		{
		if(hiup)
			hiup = false, InvalidateRect(&uprect2);
		}

}

LRESULT CGrayDlg::OnMouseLeave(WPARAM, LPARAM)

{
	hiup = himin = hiclose = false;
	
	InvalidateRect(&closerect2);
    InvalidateRect(&minrect2);
    InvalidateRect(&uprect2);

    m_tracking = false;

	return 0;
}