
/* =====[ XrayM.cpp ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the XrayM.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  11/20/2008  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include "winuser.h"
#include <afxmt.h>

#include "XrayM.h"
#include "MxPad.h"
#include "misc.h"
#include "TextMode.h"

#ifdef _DEBUG
#undef THIS_FILE
//static TCHAR THIS_FILE[] = _T(__FILE__);
static const char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define MAXMENUDEPTH 12

//////////////////////////////////////////////////////////////////////////
// Static declarations

CPtrArray CXrayM::wndarr;
	
int	CXrayM::_dlgbackcol = 0;  //RGB(64, 64, 64); 
int CXrayM::_highcol = 0;     //RGB(128, 128, 128);

//////////////////////////////////////////////////////////////////////////

void	CXrayM::_FreeLast()

{
	//TRACE(_T("Freeing last xray menu %d items\r\n"), wndarr.GetSize());

	for(int loopm = wndarr.GetSize() - 1; loopm >= 0; loopm--)
		{
		CXrayM *item = (CXrayM *)wndarr[loopm];
		//ASSERT(item->magic == MENU_MAGIC);
	
		//TRACE(_T("Freeing last xray menu %p\r\n"), item);

		delete item;
		wndarr.RemoveAt(loopm);
		}
}

/////////////////////////////////////////////////////////////////////////////
// CXrayMitem dialog

void CXrayMitem::xInitVars()

{
	submenu = NULL;
	command = status = 	checked = 0;

	selected = 0;
	SetRect(&rect, 0,0,0,0);

	magic = MENUITEM_MAGIC;
	memset(key, 0, sizeof(key));

	itemstr = new CString;
	tipstr = new CString;
	tooltip = new CString;
}

CXrayMitem::CXrayMitem()

{
	xInitVars();	
}

CXrayMitem::CXrayMitem(const TCHAR *ss)

{
	xInitVars();
	*tipstr = *itemstr = ss;
}

CXrayMitem::~CXrayMitem()

{
	//P2N(_T("CXrayMitem::~CXrayMitem()\r\n"));
	delete tooltip;
	delete itemstr;
	delete tipstr;
}

CXrayM::~CXrayM()

{
	Clear();

	// Remove from STATIC stale list if still there
	for(int loopm = wndarr.GetSize() - 1; loopm >= 0; loopm--)
		{
		CXrayM *item = (CXrayM *)wndarr[loopm];
		//ASSERT(item->magic == MENU_MAGIC);
		
		if(item == this)
			{
			//P2N(_T("CXrayM::~CXrayM() matching item in static arr\r\n"));
			wndarr.RemoveAt(loopm);
			}
		}
}

//////////////////////////////////////////////////////////////////////////
// CXrayM dialog

CXrayM::CXrayM(CWnd* pParent /*=NULL*/)
	: CGrayDlg(CXrayM::IDD, pParent)
{
	parent = pParent;

	//P2N(_T("CXrayM::CXrayM %p\r\n"), this);

	xInitVars();
}

CXrayM::CXrayM(int iddx, CWnd* pParent)
	: CGrayDlg(iddx, pParent)

{
	//P2N(_T("CXrayM::CXrayM this=%p, parent=%p\r\n"), this, pParent);

	parent = pParent;

	xInitVars();
	xidd = iddx;
}

//////////////////////////////////////////////////////////////////////////

void	CXrayM::xInitVars()

{
	bAlpha = 230;
	
	titleheight = 10;
	topgap = 16;
	leftgap = 24;
	gap = 4;
	xtextheight = 12;

	noremember = true;
		
	ontop = false;

	// Set colors
	//dlgbackcol	= RGB(192, 192, 192);
	//highcol	= RGB(150,150,150);

	disabcol	= RGB(200, 200, 200);
	grayedcol	= RGB(150, 150, 150);

	// Get them from global static
	dlgbackcol	= _dlgbackcol;
	highcol		= _highcol;

	// Was not set it, get system values
	if(dlgbackcol == 0 && highcol == 0)
		{
		dlgbackcol	= GetSysColor(COLOR_3DFACE);
		highcol		= GetSysColor(COLOR_HIGHLIGHT);
		}

	lowlowcol	= GetSysColor(COLOR_3DDKSHADOW);
	lowcol		= GetSysColor(COLOR_3DSHADOW);
	textcolor	= GetSysColor(COLOR_MENUTEXT);

	//checkmark	= RGB(128, 128, 128);
	
	subopen		= mparent = NULL;
	closeonup	= xloaded = xfired = false;
	sendto		= NULL;

	magic = MENU_MAGIC;

	limit_mon = true;

	// Add this to the list
	//P2N(_T("Added to global array %p\r\n"), this);
	wndarr.Add((void *)this);

	// Just testing
	//P2N(_T("MF_ENABLED=%d MF_DISABLED=%d MF_GRAYED=%d MF_POPUP=%d\r\n"), 
	//			MF_ENABLED, MF_DISABLED, MF_GRAYED, MF_POPUP);

	//{{AFX_DATA_INIT(CXrayM)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//P2N(_T("CXrayM::xInitVars %p &magic=%p %d\r\n"), this, &magic, (char*)&magic - (char*)this);
	
	//windowname = support.GetFileNameExt(_T(__FILE__));
	windowname = _T(__FILE__);
}

void CXrayM::DoDataExchange(CDataExchange* pDX)

{
	CGrayDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CXrayM)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CXrayM, CGrayDlg)
	//{{AFX_MSG_MAP(CXrayM)
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
/// Append Menu Item to the end of menu

int CXrayM::AppendMenuItem(const TCHAR *str, int command, int status, int checked)

{
	//ASSERT(magic == MENU_MAGIC);
	
	CXrayMitem *item = new CXrayMitem(str); 

	//ASSERT(item->magic == MENUITEM_MAGIC);

	item->status = status;
	item->command = command;
	item->checked = checked;
	arr.Add(item);

	return arr.GetUpperBound();
}

//////////////////////////////////////////////////////////////////////////
/// Append Sub Menu Item to the end of menu

int CXrayM::AppendSubMenu(const TCHAR *str, CXrayM *submenu, int status)

{
	//return 0;

	//ASSERT(magic == MENU_MAGIC);
	
	if(!submenu)
		{
		P2N(_T("CXrayM::AppendSubMenu Must Specify a submenu\r\n"));
		return 0;
		}

	CXrayMitem *item = new CXrayMitem(str); 

	//ASSERT(item->magic == MENUITEM_MAGIC);

	item->status = status;
	item->submenu = submenu;
	arr.Add(item);
	
	return arr.GetUpperBound();
}

//////////////////////////////////////////////////////////////////////////
/// Execute load and show in one step

void CXrayM::LoadAndShow(int nResource, int skip, int xx, int yy)

{
	//ASSERT(magic == MENU_MAGIC);
	
	if(xx == -1 || yy == -1)
		{
		CPoint mouse; GetCursorPos(&mouse);
		xx = mouse.x; yy = mouse.y;
		}

	LoadMenu(MAKEINTRESOURCE(nResource), skip);
	Show(xx, yy);
}

// ///////////////////////////////////////////////////////////////////////////
// CXrayM message handlers

//////////////////////////////////////////////////////////////////////////
/// Show menu at curent mouse

void	CXrayM::Show()

{
	CPoint mouse; GetCursorPos(&mouse);
	Show(mouse.x, mouse.y);
}

//////////////////////////////////////////////////////////////////////////
/// Show menu at specified screen coordinates

void	CXrayM::Show(int xx, int yy, int sub)

{
	//ASSERT(magic == MENU_MAGIC); 
	
	//support.YieldToWinEx();

	CMutex mx;	mx.Lock();

	//P2N(_T("Scanning global array in %p size=%d\r\n"), this, wndarr.GetSize());
	
	// Remove all pending menus from list
	for(int loopm = wndarr.GetSize() - 1; loopm >= 0; loopm--)
		{
		CXrayM *item = (CXrayM *)wndarr[loopm];

		//ASSERT(item->magic == MENU_MAGIC);

		// Only process them after loading
		if(item->xloaded)
			{
			// Only scan top menus
			if(item->mparent == NULL)
				{
				int flag = false;

				if(item->m_hWnd && IsWindow(item->m_hWnd))
					{
					if(item->IsWindowVisible())
						flag = true;
				
					CString str; item->GetMenuItemText(0, str);
					//P2N(_T("Loop=%d ptr=%p flag=%d str=%s\r\n"), loopm, item, flag, str);

					if(!flag && item != this)
						{
						P2N(_T("Removing stale menu from array %p\r\n"), wndarr[loopm]);
						//item->DestroyWindow();

						//item->Clear();
						delete item;
						
						wndarr.RemoveAt(loopm);
						}
					}
				}
			else
				{
				// This is a submenu, remove it from safety list
				//wndarr.RemoveAt(loopm);
				}
			}
		}
	mx.Unlock();
	
	xCreate();

	//P2N(_T("Show CXrayM window parent=%p\r\n"), GetParent());

	// Calc Size
	int ww, hh;	xMeasureMenu(&ww, &hh);
	xtextheight = hh;

	// Count items and separators
	int counti = 0, counts = 0;
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];
	
		//ASSERT(item->magic == MENUITEM_MAGIC);

		if(*item->itemstr == "")
			counts++;
		else
			counti++;
		}

	int www = ww + 2 * gap + 2 * leftgap + xtextheight;

	int hhh = (hh +  2 * (gap)) * (counti) + 
					(gap) * (counts) + 
						2 * topgap;

	//P2N(_T("Menu popup %d over %d %d\r\n"), bigrc.bottom, yy + hhh);

	xAutoPos(&xx, &yy, www, hhh);
		
	// Position and size the window ....
	SetWindowPos(NULL, xx, yy, www,  hhh, SWP_NOZORDER );  

	CGrayDlg::Show();

	if(ontop)
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

void CXrayM::OnKillFocus(CWnd* pNewWnd) 

{
	CGrayDlg::OnKillFocus(pNewWnd);

	// Closes the menu if no submenus are open	
	if(!subopen)
		Hide();
}

BOOL CXrayM::PreTranslateMessage(MSG* pMsg) 

{
	//P2N(_T("CXrayM::PreTranslateMessage %s\r\n"), mxpad.num2msg(pMsg->message));

	// ////////////////////////////////////////////////////////////////////
	// Dispatch messages ourselvs

	if(pMsg->message == WM_CHAR)
		{
		//P2N(_T("WM_CHAR\r\n"));
		//OnChar(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		return 0;
		}
	if(pMsg->message == WM_KEYDOWN)
		{
		//P2N(_T("WM_KEYDOWN char\r\n"));
		OnKeyDown(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		return true;
		}

	if(pMsg->message == WM_KEYUP)
		{
		//OnKeyUp(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		return 0;
		}
	if(pMsg->message == WM_SYSCHAR)
		{
		//OnSysChar(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		return 0;
		}
	if(pMsg->message == WM_SYSKEYDOWN)
		{
		//OnSysKeyDown(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		return 0;
		}
	if(pMsg->message == WM_SYSKEYUP)
		{
		//OnSysKeyUp(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		return 0;
		}

	if(pMsg->message == WM_MOUSEHOVER )
		{
		//P2N(_T("Mouse hover message\r\n"));
		xfired = true;

		TRACKMOUSEEVENT te;

		te.cbSize = sizeof(TRACKMOUSEEVENT);
		te.dwFlags = TME_HOVER | TME_CANCEL ;
		te.hwndTrack = this->m_hWnd;		
		::_TrackMouseEvent(&te);
		
		// See if it is time to pop up a menu
		//OnLButtonDown(0, xlastmouse);

		xScanForAction(xlastmouse, true);
		}

	return CGrayDlg::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
// CMenu

void CXrayM::OnPaint() 

{
	//ASSERT(magic == MENU_MAGIC);

	CGrayDlg::OnPaint();

	CClientDC dc(this); // device context for painting

	CTextMode tm(&dc);
	
	dc.SetBkMode(TRANSPARENT);

	RECT rcorg; GetClientRect(&rcorg);
	RECT rc; CopyRect(&rc, &rcorg);

	rc.top += topgap; rc.left += leftgap; rc.right -= 2 * gap;
	
	CFont* oldFont = dc.SelectObject(&m_fontMenu);
	
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];
	
		//ASSERT(item->magic == MENUITEM_MAGIC);

		item->rect.left  = rcorg.left + gap;
		item->rect.right = rc.right - gap;
		item->rect.top	 = rc.top + gap;
		
		// Separator
		if(*item->itemstr == "")
			{
			rc.top += gap;
			CRect rc2(2 * gap + gap, rc.top, rc.right -  gap, rc.top + 2);
			dc.Draw3dRect(rc2, RGB(20, 20, 20), RGB(120, 120, 120));
			rc.top += gap;
			}
		else
			{
			CRect rc3(rc); 
			//rc3.top += gap;	rc3.bottom = rc3.top + textheight;
			rc3.bottom = rc3.top + xtextheight +  2 * gap;

			rc3.left = rcorg.left + gap; rc3.right = rcorg.right - gap; 

			// Draw background
			if(item->selected)
				if(item->status & MF_GRAYED)
					dc.FillSolidRect(&rc3, lowlowcol);	
				else if(item->status & MF_DISABLED)
					dc.FillSolidRect(&rc3, lowcol);	
				else
					dc.FillSolidRect(&rc3, highcol);
			else
				dc.FillSolidRect(&rc3, dlgbackcol);
			
			//dc.FillSolidRect(&rc3, highcol);	

			// Color text
			if((item->status & MF_GRAYED))
				dc.SetTextColor(grayedcol);
			else if(item->status & MF_DISABLED)
				dc.SetTextColor(disabcol);
			else
				dc.SetTextColor(textcolor);
				
			CString cstr, comm, tabb; comm = cstr = *item->itemstr;
		
			// Get tabbed accel key
			int idx = cstr.Find('\t', 0);	
			if(idx >= 0)
				{
				comm = cstr.Left(idx);
				tabb = cstr.Mid(idx + 1);
				}

			rc.top += gap;

			// Draw dot Box / Check Box

			if(item->checked)
				{
				//CPen ppx; ppx.CreatePen(PS_SOLID, 1, RGB( 0, 0, 0));
				CPen ppx; ppx.CreatePen(0, 1, checkcol);
				CPen *ppp = (CPen*)dc.SelectObject(ppx);
			
				CRect rc4(2*gap, rc.top, 2 * gap + 6, rc.top + 12);
				
				dc.MoveTo(rc4.left, rc4.top + rc4.Height()/2);
				dc.LineTo(rc4.left + rc4.Width()/2, rc4.bottom);
				dc.LineTo(rc4.right, rc4.top);
				//dc.LineTo(rc4.right + 4, rc4.top);
				}
			else
				{
				CRect rc4(2 * gap, rc.top + 6, 2 * gap + 4, rc.top + 10);
				dc.Draw3dRect(rc4, RGB(20, 20, 20), RGB(120, 120, 120));
				}
			
			int hh = dc.DrawText(comm, &rc, DT_LEFT  | DT_TOP);
			rc.right -= 2 * gap;
			dc.DrawText(tabb, &rc, DT_RIGHT | DT_TOP );
			rc.right += 2 * gap;
				
			if(item->submenu)
				{
				CRect rc5(rcorg.right - (3*gap), rc.top + gap/2 ,  rcorg.right -  2 * gap, rc.top + 10);

				xpp[0].x = rc5.left,  xpp[0].y = rc5.top;
				xpp[1].x = rc5.right, xpp[1].y = rc5.top + rc5.Height()/2;
				xpp[2].x = rc5.left,  xpp[2].y = rc5.bottom;
				xpp[3].x = rc5.left,  xpp[3].y = rc5.top;

				CBrush bb; bb.CreateSolidBrush(RGB(60, 60, 60));
				CBrush *cc = (CBrush*)dc.SelectObject(bb);
				CPen ppx; ppx.CreatePen(0,1,RGB( 60, 60, 60));
				CPen *ppp = (CPen*)dc.SelectObject(ppx);
				
				dc.Polygon(xpp, 3);
				
				//dc.SelectObject(ppp);
				}

			rc.top += hh;
			rc.top += gap / 2;
			}
		item->rect.bottom = rc.top;
		}

	dc.SelectObject(oldFont);
	
}

//////////////////////////////////////////////////////////////////////////
/// Hide menu. Closing and releasing resources is automatically done 
/// when a new menu opens.

void CXrayM::Hide(int special)

{
	//P2N(_T("hiding %p\r\n"), this);

	//ASSERT(magic == MENU_MAGIC);

	CGrayDlg::Hide();

	// If we are in a menu we want parent to go
	if(mparent)
		{
		if(IsWindow(mparent->m_hWnd))
			mparent->SetFocus();
		}
	//support.YieldToWinEx();

	// Root window, close all destroy
	//if(!parent)
	//	DestroyWindow();
}

void CXrayM::OnMouseMove(UINT nFlags, CPoint point) 

{
	//P2N(_T("CXrayM::OnMouseMove\r\n"));

	// Hittest	
	int in_item = false;

	// Moved more than hover pixels?
	if	(abs(capmouse.x - point.x) > 5 ||
			abs(capmouse.y - point.y) > 5)
		{
		xtip.Hide();
		}

	xlastmouse = point;
	
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		if(PtInRect(&item->rect, point))
			{
			in_item = true;

			if(xoldloop != loop)
				{
				//P2N(_T("Mouse changed into %s\r\n"), item->str);
				if(subopen)
					{
					subopen->Hide(true);
					subopen = NULL;
					}
				// If not a separator
				if(*item->itemstr != "" )
					{
					for(int loop2 = 0; loop2 < arr.GetSize(); loop2++)
						{
						CXrayMitem *item2 = (CXrayMitem*)arr[loop2];
						item2->selected = false;
						}
					SetFocus();
					xfired = false;
					//tip.Hide();
					item->selected = true;
					InvalidateRect(&item->rect);
					//Invalidate();

					TRACKMOUSEEVENT te;

					if(!xfired)
						{
						te.cbSize = sizeof(TRACKMOUSEEVENT);
						te.dwFlags = TME_LEAVE;
						te.hwndTrack = this->m_hWnd;
						::_TrackMouseEvent(&te);
					
						te.cbSize = sizeof(TRACKMOUSEEVENT);
						te.dwFlags = TME_HOVER;
						te.hwndTrack = this->m_hWnd;
						
						if(item->submenu)
							te.dwHoverTime = 200;
						else
							te.dwHoverTime = 1500;
					
						::_TrackMouseEvent(&te);
						}
					}
				xoldloop = loop;	
				break;
				}
			}
		}

	// Off of all items, make sure it is reflected
	if(!in_item)
		{
		if(xoldloop != -1)
			{
			//P2N(_T("Exited items\r\n"));
			}
		xoldloop = -1;
		}

	CGrayDlg::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

void	CXrayM::xMeasureMenu(int *ww, int *hh)

{
	*ww = 100;	*hh = 100;

	int len = 0;
	CString *str = NULL;

	// Find longest one
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		int len2 = item->itemstr->GetLength();
		if(len2 > len)
			{
			len = len2; 
			str = item->itemstr;
			}
		}

	SIZE ts;

	if(str)
		{
		xGetMenuTextSize(*str, ts);
		*ww = ts.cx;
		*hh = ts.cy;
		}
}

//////////////////////////////////////////////////////////////////////////
// Get main window handle, failover to desktop

CWnd	*CXrayM::xSafeGetMainWnd()

{
	CWnd *pWnd = AfxGetMainWnd();            // Get main window

	// Fail over
	if (pWnd == NULL) 
		pWnd = CWnd::GetDesktopWindow();

	//ASSERT(pWnd);
	pWnd->AssertValid();
	return pWnd;
}

//////////////////////////////////////////////////////////////////////////
/// Obtain the width/height of menu text:	

int		CXrayM::xGetMenuTextSize(const TCHAR *str, SIZE &size)

{
	CWnd *pWnd = xSafeGetMainWnd();
	CDC *pDC = pWnd->GetDC();			// Get device context
	//ASSERT(pDC);
	pDC->AssertValid();

	m_fontMenu.AssertValid();

	CFont*	pOldFont = pDC->SelectObject (&m_fontMenu);
	
	CRect txtrc;		
	pDC->DrawText(str, _tcslen(str),	
				&txtrc, DT_SINGLELINE | DT_LEFT | DT_CALCRECT);

	size.cx = txtrc.right - txtrc.left;
	size.cy = txtrc.bottom - txtrc.top;

	//P2N(_T("Menu Text '%s' size cx=%d cy=%d\r\n"), str, size.cx, size.cy);

	// Select back old font, release the DC
	pDC->SelectObject (pOldFont);	pWnd->ReleaseDC(pDC);		

	return 0;
}


int CXrayM::OnCreate(LPCREATESTRUCT lpCreateStruct) 

{
	if (CGrayDlg::OnCreate(lpCreateStruct) == -1)
		return -1;

	//P2N(_T("CXrayM::OnCreate %p parent=%p desktop=%p appwin=%p\r\n"), this, 
	//				lpCreateStruct->hwndParent, 
	//					CWnd::GetDesktopWindow(), AfxGetMainWnd());

	LOGFONT m_lf; ZeroMemory ((PVOID) &m_lf,sizeof (LOGFONT));

	// Get system menu font
	NONCLIENTMETRICS nm; nm.cbSize = sizeof (NONCLIENTMETRICS);
	(SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
				nm.cbSize,&nm,0)); 

	m_lf =  nm.lfMenuFont; 
	(m_fontMenu.CreateFontIndirect (&m_lf));

	m_fontMenu.AssertValid();
	
	//tip.Create(IDD_DIALOG12, this);
	//tip.Hide();

	return 0;
}

void CXrayM::OnLButtonDown(UINT nFlags, CPoint point) 

{
	//ASSERT(magic == MENU_MAGIC);

	CGrayDlg::OnLButtonDown(nFlags, point);
}

void CXrayM::xScanForAction(CPoint &point, int NoClick) 

{
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		if(PtInRect(&item->rect, point))
			{
			if(! ( (item->status & MF_DISABLED) || (item->status &MF_GRAYED)) )
				{
				if(item->submenu)
					{
					xOpenSubMenu(item, point);
					}
				else
					{
					if(!NoClick)
						{
						//P2N(_T("Cliked on item ID %d %s\r\n"), item->command, item->str);
						
						xExecItem(item);
						}
					else
						{	
						// Show tooltip
						//P2N(_T("Tooltip time for %s: '%s'\r\n"), item->str, item->tipstr);
						capmouse = point;
						// Autosize and autolocate
						CPoint screen = xlastmouse;	ClientToScreen(&screen); 
						screen.x += 12; screen.y += 24;
						xtip.Show(*item->tipstr, screen);						
						}
					}
				}
			break;
			}
		}
}

void CXrayM::OnRButtonDown(UINT nFlags, CPoint point) 

{
	//ASSERT(magic == MENU_MAGIC);
	CGrayDlg::OnRButtonDown(nFlags, point);
}


//////////////////////////////////////////////////////////////////////////
/// Clear all loaded menu items

void CXrayM::Clear()

{
	//P2N(_T("CXrayM::Clear for %p\r\n"), this);
	//DumpMenu(this, "  ");

	//ASSERT(magic == MENU_MAGIC);

	// Free menu memory
	for(int loop = arr.GetUpperBound(); loop >= 0; loop--)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		// Kill submenus of this one
		if(item->submenu)
			{
			//item->submenu->Clear();

			// Remove the deleted item from further links
			for(int loop2 = loop-1; loop2 >= 0; loop2--)
				{
				CXrayMitem *item2 = (CXrayMitem*)arr[loop2];

				if(item2->submenu == item->submenu)
					{
					//P2N(_T("Killed duplicate submenu %p\r\n"), item->submenu);
					item2->submenu = NULL;
					}
				}
			delete item->submenu;
			//item->submenu->DestroyWindow();
			//P2N(_T("Scanning global array %p\r\n"), this);
	
			item->submenu = NULL;
			}
		delete item;
		}
	arr.RemoveAll();
}

/// Set menu Item checked state by index

void CXrayM::SetCheck(int idx, int check)

{
	if(idx < arr.GetSize() && idx >= 0)
		{	
		((CXrayMitem*)arr[idx])->checked = check;
		}
}

/// Diagnostic function. Dump to MxPad

void CXrayM::DumpMenu(CXrayM *menu, CString indent)

{
	P2N(_T("%s--------------------------------------------------------\r\n"), indent);
	P2N(_T("%sMenu %p\r\n"), indent, this);

	for(int loop = 0; loop < menu->arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)menu->arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		P2N(_T("%sstr: '%s'\r\n"), indent, *item->itemstr);
		//P2N(_T("%stip: '%s'\r\n"), indent, item->tipstr);

		P2N(_T("%scom=%-8d sub=%-8x  stat=%-8d  check=%-8d  key=%-8s \r\n"),
					indent, 
						item->command, item->submenu, 
							item->status, item->checked, item->key);
		if(item->submenu)
			{
			indent += "    ";
			DumpMenu(item->submenu, indent);
			indent = indent.Mid(0, indent.GetLength() - 4);
			}
		}
	P2N(_T("%s--------------------------------------------------------\r\n"), indent);
	P2N(_T("%sMenu %p End\r\n"), indent, this);
}


//////////////////////////////////////////////////////////////////////////
/// Return the index of the menu item by command
///

int CXrayM::FindMenuItem(int command)

{
	//ASSERT(magic == MENU_MAGIC);
	
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		if(item->command == command)
			{
			return(loop);
			}
		}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
/// Return the index of the menu item by menu str
///
/// Case sensitive search
///

int CXrayM::FindMenuItem(const TCHAR *str)

{
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		if(_tcsstr(*item->itemstr, str) != NULL)
			{
			//P2N(_T("CXrayM::FindMenuItem loop=%d\r\n"), loop);
			return(loop);
			}
		}
	
	return -1;
}

// ////////////////////////////////////////////////////////////////////////
/// Return the submenu searched by menu str
///
/// Case sensitive search
///

CXrayM *CXrayM::FindSubmenu(const TCHAR *str)

{
	//ASSERT(magic == MENU_MAGIC);
	
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		if(item->submenu)
			{
			if(_tcsstr(*item->itemstr, str) != NULL)
				{
				return(item->submenu);
				}
			}
		}
	return NULL;
}

// ///////////////////////////////////////////////////////////////////////
/// Return the submenu searched by submenu idx

CXrayM *CXrayM::FindSubmenu(int idx)

{
	//ASSERT(magic == MENU_MAGIC);
	
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		if(item->submenu)
			{
			if(idx-- == 0)
				{
				return(item->submenu);
				}
			}
		}
	return NULL;
}

// ///////////////////////////////////////////////////////////////////////
/// Delete menuitem, specified by index.

void	CXrayM::DelMenuItem(int idx)

{
	//ASSERT(magic == MENU_MAGIC);
	
	if(idx < arr.GetSize() && idx >= 0)
		{
		CXrayMitem *item = (CXrayMitem*)arr[idx];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		if(item->submenu)
			{
			//P2N(_T("Del Submenu\r\n"));
			}
		delete item;
		arr.RemoveAt(idx);
		}
}

//////////////////////////////////////////////////////////////////////////
/// Return the number of menu items. 

int CXrayM::GetNumItems()

{
	//ASSERT(magic == MENU_MAGIC);
	
	return(arr.GetSize());
}

//////////////////////////////////////////////////////////////////////////
/// Load a menu from resource, skip number of items from head. (for right click)

BOOL CXrayM::LoadMenu(int nResource, int skip)

{
	return(CXrayM::LoadMenu(MAKEINTRESOURCE(nResource), skip));
}


// ///////////////////////////////////////////////////////////////////////
//
// Function name:   CXrayM::LoadMenu
// Description:     Load menu from resource
// Return type:     BOOL 
// Argument:        LPCTSTR lpszResourceName
// Argument:        int skip
// Caveat:			Submenus deeper than 24 are not loaded
//
// ////////////////////////////////////////////////////////////////////////

/// Load a menu from resource, skip number of items from head.
//
// On VC 2005, even single byte resources are saved as 2 byte ones.
//

BOOL CXrayM::LoadMenu(LPCTSTR lpszResourceName, int skip)

{
	//ASSERT(magic == MENU_MAGIC);	
	//P2N(_T("Loading %d \r\n"), lpszResourceName);

	int menucnt = 0;

	if(xloaded)
		return TRUE;

	Clear();

	//ASSERT_VALID(this);	//ASSERT(lpszResourceName != NULL);
	
	// Find the Menu Resource:
	HINSTANCE	hInst =	 AfxFindResourceHandle(lpszResourceName, RT_MENU);
	HRSRC		hRsrc =  ::FindResource(hInst,lpszResourceName, RT_MENU);

	if (hRsrc == NULL)
		{
		hInst = NULL;
		hRsrc = ::FindResource(hInst, lpszResourceName, RT_MENU);
		}

	if(hRsrc == NULL)
		{	
		CString tmp; 
		tmp.Format(_T("Could not open menu resource %d"), lpszResourceName);
		AfxMessageBox(tmp);
		return FALSE;
		}
	
	// Load the Menu Resource:	
	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if(hGlobal == NULL)
		{
		CString tmp; 
		tmp.Format(_T("Could not load menu resource %d"), lpszResourceName);
		AfxMessageBox(tmp);
		return FALSE;
		}	

	// Get Item template Header, and calculate offset of MENUITEMTEMPLATES:
	MENUITEMTEMPLATEHEADER *pTpHdr = 
			(MENUITEMTEMPLATEHEADER*)LockResource(hGlobal);

	BYTE* pTp=(BYTE*)pTpHdr + 
			(sizeof(MENUITEMTEMPLATEHEADER) + pTpHdr->offset);
	
	//D2N(pTp, 100);

	int seen_popup = 0, count = 0;

	CXrayM *lastclass[MAXMENUDEPTH]; lastclass[seen_popup] = this;
	int		lastend[MAXMENUDEPTH]; memset(lastend, 0, sizeof(lastend));

	// Processing of Menu Item Templates:
	while(true)
		{
		WORD    wFlags = 0, wID  = 0, uFlags;	
		WCHAR	*wstr;

		// Extract flags, ID and command string
		wFlags = *((WORD*)pTp); pTp += sizeof(WORD);
		uFlags = wFlags & ~(WORD)MF_END;

		if(wFlags & MF_POPUP)
			{
			wID = 0;
			}			
		else
			{
			// Step over the ID
			wID = *((WORD*)pTp); pTp += sizeof(WORD);
			}
		wstr = (WCHAR*)pTp;

		// Step over the string
		// On VC 2005, even single byte resources are saved as 2 byte ones.
		pTp += (wcslen(wstr) + 1) * sizeof(WCHAR);	
	
		// Skip not needed entry
		if(skip)
			{
			seen_popup = 0; count = 0;
			if(skip--)
				continue;
			}

		CString str; 
		// Get an ordinary string we are not unicode
		xUnicodeToAnsi(wstr, str);	
		
		//P2N(_T("flags=%-5d id=%-5d end=%d %p '%S' '%s' \r\n"), uFlags, wID, wFlags & MF_END,
		//								lastclass[seen_popup], wstr, str);
		//P2N("str='%s'\r\n", str);

		if(wFlags & MF_POPUP)
			{
			if(seen_popup < sizeof(lastclass) / sizeof(void*))
				{

#ifdef _DEBUG
//#define new new
#endif
				CXrayM *sm = new CXrayM; //ASSERT(sm);

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif
										 //lastclass[seen_popup]->AppendSubMenu(wstr, sm, uFlags);	
				lastclass[seen_popup]->AppendSubMenu(str, sm, uFlags);	
				sm->mparent = lastclass[seen_popup];
					{
					seen_popup++;
					lastclass[seen_popup] = sm;
					}
				
				// Ended theis popup
				if(wFlags & MF_END)
					{
					// Tell the array this menu hes been terminated
					lastend[seen_popup] = true;
					}
				}
			else
				{
				lastclass[seen_popup]->AppendMenuItem(_T("Warning: Exceeded menu stack depth")); 
				//P2N(_T("Exceeded menu stack depth\r\n"));
				}
			}
		else
			{
			//int idx = lastclass[seen_popup]->AppendMenuItem(wstr, wID, uFlags, 
			//		uFlags & MF_CHECKED ? true : false);
			int idx = lastclass[seen_popup]->AppendMenuItem(str, wID, uFlags, 
					uFlags & MF_CHECKED ? true : false);

			// Load tooltip
			if(wID)
				{
				CString ttip, cstr(wstr), addtip, tabb, comm(wstr);

				ttip.LoadString(wID);

				if(str != _T(""))
					{
					// Get tabbed accel key, add to tooltip				
					int sidx = cstr.Find('\t', 0);	
					if(sidx >= 0)
						{
						comm = cstr.Left(sidx);
						tabb = cstr.Mid(sidx + 1);
						if(tabb != "")
							{
							// Set the key item as well
							lastclass[seen_popup]->SetItemKey(idx, tabb);
							addtip.Format(_T("\r\nShortcut key: %s"), tabb);
							}
						}
					}
				CString tipsum(comm + _T("\r\n"));
				if(ttip != _T(""))	tipsum += ttip;
				if(addtip != _T("")) tipsum += addtip;

				lastclass[seen_popup]->SetToolTip(idx, tipsum);

				if(wFlags & MF_END)
					{
					int endit = true;
					for(int loop = seen_popup; loop >= 0; loop--)
						{
						// All seen submenus terminated?
						if(!lastend[seen_popup])
							{
							endit = false;
							}
						}
					if(endit)
						break;

					seen_popup--;				
					}
				}
			}

		// Safety valve
		if(seen_popup < 0)
			break;
		}

	xloaded = true;

	//DumpMenu(this);

	return(TRUE);
}

//////////////////////////////////////////////////////////////////////////
/// Set menu item key by index

void CXrayM::SetItemKey(int idx, const TCHAR *newstr)

{
	if(idx < arr.GetSize() && idx >= 0)
		{	
		_tcsncpy_s(((CXrayMitem*)arr[idx])->key, _TRUNCATE,
					newstr, 
				sizeof( ((CXrayMitem*)arr[idx])->key ));
		}
}

//////////////////////////////////////////////////////////////////////////
/// Set menu item text by index

void CXrayM::SetItemText(int idx, const TCHAR *newstr)

{
	//ASSERT(magic == MENU_MAGIC);
	
	if(idx < arr.GetSize() && idx >= 0)
		{	
		*((CXrayMitem*)arr[idx])->itemstr = newstr;
		}
}

//////////////////////////////////////////////////////////////////////////
/// Set menu item tooltip by index

void CXrayM::SetToolTip(int idx, const TCHAR *tipstr)

{
	//ASSERT(magic == MENU_MAGIC);
	
	if(idx < arr.GetSize() && idx >= 0)
		{	
		*((CXrayMitem*)arr[idx])->tipstr = tipstr;
		}
}

//////////////////////////////////////////////////////////////////////////
/// Set menu item int  by index

void CXrayM::SetItemInt(int idx, int val)

{
	//ASSERT(magic == MENU_MAGIC);
	
	if(idx < arr.GetSize() && idx >= 0)
		{	
		((CXrayMitem*)arr[idx])->intval = val;
		}
}

//////////////////////////////////////////////////////////////////////////
/// Set menu item tooltip by menu string

void CXrayM::SetToolTip(const TCHAR *str, const TCHAR *tipstr)

{
	//ASSERT(magic == MENU_MAGIC);
	
	int idx = FindMenuItem(str);

	if(idx < arr.GetSize() && idx >= 0)
		{	
		*((CXrayMitem*)arr[idx])->tipstr = tipstr;
		}
}

//////////////////////////////////////////////////////////////////////////

void CXrayM::PostNcDestroy() 

{
	CGrayDlg::PostNcDestroy();

	//P2N(_T("CXrayM::PostNcDestroy %p\r\n"), this); 

	if(!mparent)
		delete this;
}

//////////////////////////////////////////////////////////////////////////
/// Get menu item text by index

void	CXrayM::GetMenuItemText(int idx, CString &txt)

{
	//ASSERT(magic == MENU_MAGIC);

	if(idx < arr.GetSize() && idx >= 0)
		{	
		txt = *((CXrayMitem*)arr[idx])->itemstr;
		}
	else
		{
		txt = "Err";
		}
}

//////////////////////////////////////////////////////////////////////////
/// Get menu item int by index

int		CXrayM::GetMenuItemInt(int idx)

{
	int ret = 0;

	//ASSERT(magic == MENU_MAGIC);
	
	if(idx < arr.GetSize() && idx >= 0)
		{	
		ret = ((CXrayMitem*)arr[idx])->intval;
		}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// xUnicodeToAnsi converts the Unicode string pszW to an ANSI string
// and returns the ANSI string through str. 
// If it is Unicode already, just copy it

DWORD   CXrayM::xUnicodeToAnsi(LPWSTR pszW, CString &str)

{
    ULONG cbAnsi, cCharacters;
    DWORD dwError = NOERROR;

    // If input is null then just return an empty string
    if (pszW == NULL)
        {
		str = "";
        return NOERROR;
        }

	if(sizeof(TCHAR) != 1)
		{
		str = pszW;
		return dwError;
		}

    cCharacters = wcslen(pszW) + 1;		// + terminating NULL

    // Determine number of bytes to be allocated for ANSI string. 
    cbAnsi = cCharacters * 2;

    char *ptr =  (char*)str.GetBuffer(cbAnsi); //ASSERT(ptr);

    // Actual conversion here:
    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, ptr,
				cbAnsi, NULL, NULL))
        {
        dwError = GetLastError();
		*ptr = '\0';
        }

	str.ReleaseBuffer();
	return dwError;
}

void CXrayM::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 

{
	//P2N(_T("CXrayM::OnChar(UINT nChar='%c' %d\r\n"), nChar, nChar);	
	CGrayDlg::OnChar(nChar, nRepCnt, nFlags);
}

void CXrayM::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 

{
	int loop;

	//P2N(_T("CXrayM::OnKeyDown(UINT nChar='%c' %d\r\n"), nChar, nChar);

	//tip.Hide();

	TRACKMOUSEEVENT te;

	te.cbSize = sizeof(TRACKMOUSEEVENT);
	te.dwFlags = TME_HOVER | TME_CANCEL ;
	te.hwndTrack = this->m_hWnd;		
	::_TrackMouseEvent(&te);	

	if(nChar >= 'A' && nChar <= 'Z')
		{
		//P2N(_T("CXrayM::OnKeyDown LETTER\r\n"));

		int sel = false, iter = 0, lastsel = xGetSelected();

		if(lastsel == -1)
			loop = 0;
		else 
			loop = lastsel + 1;

		while(true)
			{
			for(; loop < arr.GetSize(); loop++)
				{
				CXrayMitem *item = (CXrayMitem*)arr[loop];
 
				if(xIsItemSelectAble(item))
					{
					if(toupper(*item->itemstr[0]) == (int)nChar)
						{
						//P2N(_T("Matching letter in %s\r\n"), item->itemstr);

						if(lastsel >= 0)
							{
							CXrayMitem *item2 = (CXrayMitem*)arr[lastsel];
							item2->selected = false;
							}
						item->selected = true;
						sel = true;
						Invalidate();
						break;
						}
					}
				}

			if(sel)
				break;
			loop = 0;
			
			// Only span it twice (like flowover from end)
			if(iter++ > 0)
				break;
			}
		}
	else if(nChar >= '0' && nChar <= '9')
		{
		//P2N(_T("CXrayM::OnKeyDown NUMBER\r\n"));

		}
	else switch(nChar)
		{
		case  VK_LEFT:
			// Hide all non top level menus
			if(mparent)
				Hide();
		break;

		case  VK_ESCAPE:
			//P2N(_T("CXrayM::OnKeyDown VK_ESC\r\n"));
			Hide();
		break;
		
		case  VK_RIGHT:
		case  VK_SPACE:
		case  VK_RETURN:
			for(loop = 0; loop < arr.GetSize(); loop++)
				{
				CXrayMitem *item = (CXrayMitem*)arr[loop];

				if(item->selected)
					{
					if(item->submenu)
						{
						CPoint point(xpp[0].x, xpp[0].y);
						xOpenSubMenu(item, point);
						}
					else
						{
						if(nChar != VK_RIGHT)
							xExecItem(item);	break;
						}
					break;
					}
				}
			//P2N(_T("CXrayM::OnKeyDown VK_SPACE VK_RETURN \r\n"));
		break;
		
		case  VK_UP:
			xGotoPrev();
			//P2N(_T("CXrayM::OnKeyDown VK_UP\r\n"));
		break;
		
		case  VK_DOWN:
			xGotoNext();
			//P2N(_T("CXrayM::OnKeyDown VK_DOWN\r\n"));
		break;
		
		case  VK_PRIOR:
			for(loop = 0; loop < 5; loop++)
				xGotoPrev();
			//P2N(_T("CXrayM::OnKeyDown VK_PRIOR\r\n"));
		break;
		
		case  VK_NEXT:
			for(loop = 0; loop < 5; loop++)
				xGotoNext();
			//P2N(_T("CXrayM::OnKeyDown VK_NEXT\r\n"));
		break;
		
		case  VK_HOME:
			//P2N(_T("CXrayM::OnKeyDown VK_HOME\r\n"));
		break;

		case  VK_END:
			//P2N(_T("CXrayM::OnKeyDown VK_END\r\n"));
		break;

		
		//case  VK_LEFT:
		//	P2N(_T("CXrayM::OnKeyDown VK_LEFT\r\n"));
		//break;

		//case  VK_RIGHT:
		//	P2N(_T("CXrayM::OnKeyDown VK_RIGHT\r\n"));
		//break;
		}

	CGrayDlg::OnKeyDown(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////////////	
// Internals

int	CXrayM::xIsItemSelectAble(CXrayMitem *item)

{
	//ASSERT(item->magic == MENUITEM_MAGIC);

	if(item->status & MF_GRAYED || 
		item->status & MF_DISABLED || 
			*item->itemstr == "")
		return false;
	else
		return true;				
}

//////////////////////////////////////////////////////////////////////////

int	CXrayM::xIsAnySelected()

{
	int sel = false;
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];
		if(item->selected)
			{
			sel = true;	break;
			}
		}
	return sel;
}

int	CXrayM::xGetSelected()

{
	int sel = -1;
	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];
		if(item->selected)
			{
			sel = loop;	break;
			}
		}
	return sel;
}

//////////////////////////////////////////////////////////////////////////

int	CXrayM::xExecItem(CXrayMitem *item)

{
	//ASSERT(item->magic == MENUITEM_MAGIC);

	if(!item->command)
		return 0;
 
	//AP2N(_T("Posting message %d \r\n"), item->command);

	if(sendto)
		sendto->PostMessage(WM_COMMAND, 
						((DWORD)item->command), 
							(DWORD)m_hWnd);
	else
		AfxGetMainWnd()->PostMessage(WM_COMMAND, 
						((DWORD)item->command), 
							(DWORD)m_hWnd);

	//support.YieldToWinEx();

	// Hide parents:
	if(mparent)
		mparent->Hide();

	Hide();

	return 0;
}

//////////////////////////////////////////////////////////////////////////

int	CXrayM::xGotoNext()

{
	if(!xIsAnySelected())
		{
		// Nothing selected, select first selectable
		for(int loop = 0; loop < arr.GetSize(); loop++)
			{
			if(xIsItemSelectAble((CXrayMitem*)arr[loop]))
				{
				((CXrayMitem*)arr[loop])->selected = true;
				Invalidate();
				break;
				}
			}
		return 0;
		}

	for(int loop = 0; loop < arr.GetSize(); loop++)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loop];
		if(item->selected)
			{
			// At End?
			if(loop < arr.GetSize() - 1)
				{
				item->selected = false;
				for(int loop2 = loop + 1; loop2 < arr.GetSize(); loop2++)
					{
					CXrayMitem *item2 = (CXrayMitem*)arr[loop2];
					if(xIsItemSelectAble(item2))
						{
						item2->selected = true;
						break;
						}
					}
				Invalidate();
				break;
				}
			}
		}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

int	CXrayM::xGotoPrev()

{
	if(!xIsAnySelected())
		{
		// Select Last
		for(int loop2 = arr.GetSize() - 1; loop2 >= 0; loop2--)
			{
			CXrayMitem *item2 = (CXrayMitem*)arr[loop2];
			if(xIsItemSelectAble(item2))
				{
				item2->selected = true;
				break;
				}
			}
		Invalidate();
		return 0;
		}

	for(int loopm = arr.GetSize() - 1; loopm >= 0; loopm--)
		{
		CXrayMitem *item = (CXrayMitem*)arr[loopm];
		if(item->selected)
			{
			// At the Beginning
			if(loopm > 0)
				{
				item->selected = false;
				for(int loop2 = loopm-1; loop2 >= 0; loop2--)
					{
					CXrayMitem *item2 = (CXrayMitem*)arr[loop2];
					if(xIsItemSelectAble(item2))
						{
						item2->selected = true;
						break;
						}
					}
				Invalidate();
				break;
				}
			else
				{
				if(closeonup)
					Hide();
				}
			}
		}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

int		CXrayM::xOpenSubMenu(CXrayMitem *item, CPoint &point)

{
	//ASSERT(magic == MENU_MAGIC);
	
	//P2N(_T("CXrayM::OpenSubMenu %p\r\n"), item);

	//ASSERT(item->magic == MENUITEM_MAGIC);

	RECT rc; GetClientRect(&rc);

	CPoint point2(rc.right, rc.top); ClientToScreen(&point2);
	CPoint point3(point); ClientToScreen(&point3);

	//P2N(_T("Cliked on submenu %d %s\r\n"), item->submenu, item->str);

	subopen = item->submenu;

	item->submenu->mparent = this;
	item->submenu->sendto = sendto;

	item->submenu->ontop 		=	ontop;
	item->submenu->highcol 		=	highcol;
	item->submenu->lowlowcol 	=	lowlowcol; 
	item->submenu->lowcol 		=	lowcol; 
	item->submenu->dlgbackcol 	=	dlgbackcol;
	item->submenu->checkcol 	=	checkcol;
	item->submenu->disabcol 	=	disabcol;
	item->submenu->grayedcol	=	grayedcol;
	item->submenu->textcolor 	=	textcolor;

	item->submenu->Show(point2.x, point3.y - topgap, 1);

	return 0;
}

void CXrayM::OnLButtonUp(UINT nFlags, CPoint point) 

{
	CGrayDlg::OnLButtonUp(nFlags, point);
	xScanForAction(point, false); 
}

void CXrayM::OnRButtonUp(UINT nFlags, CPoint point) 

{	
	CGrayDlg::OnRButtonUp(nFlags, point);
	//xScanForAction(point, false); 
}

// ////////////////////////////////////////////////////////////////////////
/// Set Item's Enabled/Disabled status by idx
/// Return old status.

int		CXrayM::SetItemStatus(int idx, int status)

{
	//ASSERT(magic == MENU_MAGIC);
	
	int ret  = -1;

	if(idx < arr.GetSize() && idx >= 0)
		{
		CXrayMitem *item = (CXrayMitem*)arr[idx];

		//ASSERT(item->magic == MENUITEM_MAGIC);

		ret = item->status;
		item->status = status;
		}
	return ret;
}

//////////////////////////////////////////////////////////////////////////

void CXrayM::xAutoPos(int *pxx, int *pyy, int ww, int hh)

{
	if(!limit_mon)
		return;

	CPoint here(*pxx, *pyy);
	CRect	rc3; misc.CurrentMonRect(here, &rc3);

	//P2N(_T("GetMonitorInfo left=%d right=%d top=%d bottom=%d\r\n"),  
	//					rc3.left, rc3.right, rc3.top, rc3.bottom);

	// Needs a turn around to show to the left
	if(here.x + ww > rc3.right)
		{
		//P2N(_T("Needs a turn around to show to the left\r\n"));		
		*pxx = rc3.right - ww - 10;
		}

	// Needs a turn around to show to the upper side
	if(here.y + hh > rc3.bottom)
		{
		//P2N(_T("Needs a turn around to show to the upper side\r\n"));
		*pyy =  rc3.bottom -  hh - 10;
		}
}

BOOL CXrayM::OnEraseBkgnd(CDC* pDC)
{
	CRect rec; GetClientRect(rec);
	
	pDC->FillSolidRect(rec, dlgbackcol);

	return true;

	//return CGrayDlg::OnEraseBkgnd(pDC);
}
