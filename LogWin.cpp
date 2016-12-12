
/* =====[ LogWin.cpp ]========================================== 
                                                                             
   Description:     The timedim project, implementation of the LogWin.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  8/30/2007  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

// LogWin.cpp : implementation file
//

#include "stdafx.h"
#include "mxpad.h"
#include "LogWin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LRESULT CALLBACK CLogWin::MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{ 
	//AP2N("MainWndProc %s\r\n", mxpad.num2msg(uMsg));

	return ::DefWindowProc(
		hwnd,      // handle to window
		uMsg,       // message identifier
		wParam,  // first message parameter
		lParam   // second message parameter
		);
}

/////////////////////////////////////////////////////////////////////////////
// CLogWin CWnd

CLogWin::CLogWin()

{
	//TRACE("CLogWin::CLogWin()\r\n");

	inited = false;
	created = false;

	::GetObject((HFONT)GetStockObject(OEM_FIXED_FONT),sizeof(m_lf),&m_lf);
	m_font.CreateFontIndirect(&m_lf);

	maxlines = DEFAULT_MAX_LINES;
	lines = 0;
	in_timer = 0;
	prevnl = false;
	bgcolor = RGB(245, 245, 245);
}

CLogWin::~CLogWin()

{
	//TRACE("Logwind destructor called\r\n");
}

BEGIN_MESSAGE_MAP(CLogWin, CWnd)
	//{{AFX_MSG_MAP(CLogWin)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_CHAR()
	ON_WM_VSCROLL()
	ON_WM_RBUTTONDOWN()
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogWin message handlers

void CLogWin::printf(const char *Format, ...)

{
	va_list ArgList; va_start(ArgList, Format);

	int len = _vsnprintf(OutString, sizeof(OutString) - 1, Format, ArgList);

	AddString(OutString);
}


void CLogWin::AddNewLine()

{
	AddString("\r\n");
}

void CLogWin::AddString(const char *str)

{
	CString newstr(str);
	
	int hasnewline = false;
	if(newstr.Find("\r\n") >= 0 )
		{
		hasnewline = true;
		newstr.Replace("\r\n", " ");
		}

	if(prevnl)
		{
		// Add to end
		strlist.AddTail(newstr);
		}
	else
		{
		// Append to current last
		if(!strlist.IsEmpty())
			{
			strlist.GetTail() += newstr;
			}
		else
			{
			strlist.AddTail(newstr);
			}
		}

	prevnl = hasnewline;

	//P2N("Added new str %p %s\r\n", newstr, *newstr);

	int cnt = strlist.GetCount();
	SetScrollRange(SB_VERT, 0, cnt);
	SetScrollPos(SB_VERT, cnt);
	
	int slen = newstr.GetLength();

	if(slen > 100)
		{
		//P2N("Adjusted scroll range %d\r\n", slen);
		SetScrollRange(SB_HORZ, 0, slen + 100);
		}

	// Pop first entry
	if(strlist.GetCount() > maxlines)
		{
		//CString &old = 
		strlist.RemoveHead();
		//delete	&old;
		}

	// Set refresh timer
	if(!in_timer)
		{
		in_timer = true;
		SetTimer(1, 300, NULL);
		}

	//delete newstr;
}

//////////////////////////////////////////////////////////////////////////

void CLogWin::Clear()

{
	POSITION pos = strlist.GetHeadPosition();
	while(pos)
		{
		CString &str = strlist.GetNext(pos);
		//P2N("In Clear() %s\r\n",  str);
		}

	strlist.RemoveAll();
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

void CLogWin::OnPaint() 

{
	CPaintDC dc(this); // device context for painting
	
	//P2N("CLogWin::Paint\r\n");

	if(!inited)
		{
		inited = true;
		//AP2N("CLogWin::Paint inited\r\n");
		}
	
	int smin, smax; GetScrollRange(SB_VERT, &smin, &smax);

	int scroll =   GetScrollPos(SB_HORZ);
	int vscroll =  smax - GetScrollPos(SB_VERT);

	RECT	rc; GetClientRect(&rc);
	
	//dc.SetBoundsRect(&rc, DCB_ACCUMULATE);
	//dc.SetBkMode(TRANSPARENT);

	CFont* oldFont = dc.SelectObject(&m_font);

	int xx = 0, yy = 0;
	CSize ss = dc.GetTextExtent("A");
		
	// This many lines to go back ....
	int back = max((rc.bottom - rc.top) / (ss.cy + 2), 0) ;

	lines = back;
	//P2N("bak %d\r\n", back);
	
	POSITION pos = strlist.GetTailPosition();

	back += vscroll;
	
	//back = min(back, strlist.GetCount());
	//dc.SetTextAlign(TA_UPDATECP );

	while(pos)
		{
		back--;

		if(!back)
			break;
		
		strlist.GetPrev(pos);
		}

	if(pos <= 0)
		pos = strlist.GetHeadPosition();

	while(pos)
		{
		CString &str = strlist.GetNext(pos);

		int scr = min(scroll, str.GetLength() );
		
		CString str2 = str.Mid(scr);

		dc.TextOut(xx, yy, str2);
		//dc.ExtTextOut(xx, yy, ETO_CLIPPED, &rc, *str, NULL);
		//dc.ExtTextOut(xx, yy, 0, &rc, *str, NULL);
		yy += ss.cy + 2;
		}

	dc.SelectObject(oldFont);
}

//////////////////////////////////////////////////////////////////////////

void CLogWin::OnDestroy() 

{
	CWnd::OnDestroy();
	
	//TRACE("CLogWin::Destroying\r\n");	

#if 0
	//P2N("Called destructor\r\n");

	POSITION pos = strlist.GetHeadPosition();
	while(true)
		{
		if(!pos)
			break;

		CString &str = strlist.GetNext(pos);
		//P2N("In destructor %s\r\n",  str);
		//delete str;
		}
#endif

}

void CLogWin::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 

{
	//P2N("CLogWin::Hor Scroll code=%d pos=%d  bar=%p\r\n", nSBCode, nPos, pScrollBar);	

	switch(nSBCode)
	{
	case SB_LINELEFT   :
		SetScrollPos(SB_HORZ, GetScrollPos(SB_HORZ)-1);
		Invalidate();
		break;

	case SB_LINERIGHT   :
		SetScrollPos(SB_HORZ, GetScrollPos(SB_HORZ)+1);
		Invalidate();
		break;

	case SB_PAGELEFT    :
		SetScrollPos(SB_HORZ, GetScrollPos(SB_HORZ)-10);
		Invalidate();
		break;

	case SB_PAGERIGHT:
		SetScrollPos(SB_HORZ, GetScrollPos(SB_HORZ)+10);
		Invalidate();
		break;

	case SB_THUMBPOSITION      :
		SetScrollPos(SB_HORZ, nPos);
		Invalidate();
		break;

	case SB_THUMBTRACK      :
		SetScrollPos(SB_HORZ, nPos);
		Invalidate();
		break;
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CLogWin::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 

{
	//P2N("CLogWin::OnChar() chh=%c\r\n", nChar);
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}


void CLogWin::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 

{
	int old = GetScrollPos(SB_VERT);

	//P2N("CLogWin::OnVScroll\r\n");

	switch(nSBCode)
		{
		case SB_LINEDOWN   :
			SetScrollPos(SB_VERT, old + 1);
			break;

		case SB_LINEUP   :
			SetScrollPos(SB_VERT, old - 1);
			break;

		case SB_PAGEDOWN    :
			SetScrollPos(SB_VERT, old + 10);
			break;

		case SB_PAGEUP :
			SetScrollPos(SB_VERT, old - 10);
			break;

		case SB_THUMBPOSITION      :
			SetScrollPos(SB_VERT, nPos);
			break;

		case SB_THUMBTRACK      :
			SetScrollPos(SB_VERT, nPos);		
			break;
		}

	if(old != GetScrollPos(SB_VERT))
		Invalidate();

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CLogWin::OnRButtonDown(UINT nFlags, CPoint point) 
{
	//P2N("OnRButtonDown() \r\n");
	CWnd::OnRButtonDown(nFlags, point);
} 

void CLogWin::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	//P2N("CLogWin::OnShowWindow\r\n");
	
	if(!created)
		{
		created = true;

		//SetTimer(1, 100, NULL);
		//return;
		}

	SetWindowPos(&wndTop, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
	CWnd::OnShowWindow(bShow, nStatus);
}

void CLogWin::OnTimer(UINT nIDEvent) 

{		
	if(nIDEvent == 1)
		{
		KillTimer(nIDEvent);
		in_timer = false;
		Invalidate();
		}

	//ASSERT(IsWindow(AfxGetApp()->m_pMainWnd->m_hWnd));	
	//ShowWindow(true);
	
	CWnd::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////

void CLogWin::PreSubclassWindow() 

{
 	//AP2N("CLogWin::PreSubclassWindow\r\n");

 	ModifyStyle(0, WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | SS_NOTIFY);

	SetWindowLong(m_hWnd, GWL_WNDPROC, (DWORD)MainWndProc);

	EnableScrollBarCtrl(SB_BOTH, true);

	SetScrollRange(SB_HORZ, 0, 100);
	SetScrollRange(SB_VERT, 0, 1);
	
	ShowScrollBar(SB_BOTH, true);

	CWnd::PreSubclassWindow();

#if 0
	WNDCLASS wc;
	char str[128]; GetClassName(m_hWnd, str, 128);
	//AP2N("class=%s\r\n", str);

	GetClassInfo(NULL, str, &wc);
	
    AP2N("UINT    style;        	%d\r\n", wc.style);                  
    AP2N("WNDPROC lpfnWndProc;      %p\r\n", wc.lpfnWndProc); 
    AP2N("int     cbClsExtra;       %d\r\n", wc.cbClsExtra); 
    AP2N("int     cbWndExtra;       %d\r\n", wc.cbWndExtra); 
    AP2N("HANDLE  hInstance;        %d\r\n", wc.hInstance); 
    AP2N("HICON   hIcon;            %d\r\n", wc.hIcon); 
    AP2N("HCURSOR hCursor;          %d\r\n", wc.hCursor); 
    AP2N("HBRUSH  hbrBackground;    %d\r\n", wc.hbrBackground); 
    AP2N("LPCTSTR lpszMenuName;     %s\r\n", wc.lpszMenuName); 
    AP2N("LPCTSTR lpszClassName;    %s\r\n", wc.lpszClassName); 
#endif
}

//////////////////////////////////////////////////////////////////////////
//
// Function name:    CLogWin::CreateFrom
// Description:      <empty description>
// Return type:      void 
// Argument:         CWnd *wnd		== parent window
// Argument:         int id			== control to model after
//
//////////////////////////////////////////////////////////////////////////

void CLogWin::CreateFrom(CWnd *wnd, int id)

{
	//P2N("Create from %p %d\r\n", wnd, id);

	ASSERT(wnd);

	CWnd *me = wnd->GetDlgItem(id);
	ASSERT(me);
	
	me->ShowWindow(false);

	RECT wrc;
	me->GetWindowRect(&wrc);
	wnd->ScreenToClient(&wrc);
	
	//Create(NULL, "Log Window", WS_HSCROLL | WS_VSCROLL | WS_CHILD| WS_DLGFRAME  , wrc, wnd, 0);
	CreateEx( WS_EX_CLIENTEDGE  , NULL, NULL,   WS_HSCROLL | WS_VSCROLL | 
						/*WS_TABSTOP  | */ WS_CHILD, wrc, wnd, 1);

	//WINDOWPLACEMENT wp;
	//GetWindowPlacement(&wp); 
	//wp.rcNormalPosition.right -= 1;	SetWindowPlacement(&wp);
	//wp.rcNormalPosition.right += 1; SetWindowPlacement(&wp);


	SetScrollRange(SB_HORZ, 0, 100);
	SetScrollRange(SB_VERT, 0, 1);
	SetScrollPos(SB_VERT, 0);

	EnableScrollBarCtrl(SB_HORZ, false);
	EnableScrollBarCtrl(SB_VERT, false);

	ShowScrollBar(SB_BOTH);

	//EnableScrollBarCtrl(SB_HORZ, false);
	//EnableScrollBarCtrl(SB_VERT, false);

	ShowWindow(true);
}

BOOL CLogWin::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 

{
	int old = GetScrollPos(SB_VERT);

	if(zDelta < 0)
		SetScrollPos(SB_VERT, old + 10);
	else
		SetScrollPos(SB_VERT, old - 10);

	// If changed
	if(GetScrollPos(SB_VERT) != old)
		Invalidate();

	//P2N("OnMouseWheel() \r\n");

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

//////////////////////////////////////////////////////////////////////////

void CLogWin::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 

{
	//P2N("CLogWin::OnKeyDown() %c\r\n", nChar);

	int old = 0, proc = 0;
	switch(nChar)
		{
		case  VK_PRIOR:
			old = GetScrollPos(SB_VERT); proc = true;
			SetScrollPos(SB_VERT, old - 10);
		break;
		
		case  VK_NEXT:
			old = GetScrollPos(SB_VERT); proc = true;
			SetScrollPos(SB_VERT, old + 10);
		break;
		
		case  VK_HOME:
			old = GetScrollPos(SB_VERT); proc = true;
			SetScrollPos(SB_VERT, 0);
		break;

		case  VK_END:
			old = GetScrollPos(SB_VERT); proc = true;
			int lim, lim2;
			GetScrollRange(SB_VERT, &lim, &lim2);
			SetScrollPos(SB_VERT, lim2);
		break;		
		}

	if(proc)
		{
		if(old != GetScrollPos(SB_VERT))
			Invalidate();
		}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CLogWin::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 

{
	//AP2N("CLogWin::Activate window\r\n");

	if(nState != WA_INACTIVE)
		{
		SetFocus();
		}

	//CWnd::OnActivate(nState, pWndOther, bMinimized);
}

int CLogWin::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	//P2N("Mouse Activate\r\n");
	
	SetFocus();
	
	//Invalidate();

	return MA_ACTIVATE;

	//return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////////

BOOL CLogWin::OnEraseBkgnd(CDC* pDC) 
{
	RECT	rc;
	GetClientRect(&rc);

	//P2N("CLogWin::Erase Dialog\r\n");

	pDC->FillSolidRect(&rc, bgcolor);

	return 1;
	//return CWnd::OnEraseBkgnd(pDC);
}

//////////////////////////////////////////////////////////////////////////
// Break string into parts

void CLogWin::AddDelimStr(CString *pstr, int lenline)

{
	// Soften it by line segmentation

	int len = pstr->GetLength();
	CString res2;

	for(int loops = 0; loops < len; loops++)
		{
		char	chh = pstr->GetAt(loops);
		res2 += chh;		

		// Exceeded line len, put it
		if(res2.GetLength() > lenline && chh == ' ')
			{
			AddString(res2); AddString("\r\n");
			res2 = "";
			}
		}

	// Put remainder
	if(res2 != "")
		{
		AddString(res2); AddString("\r\n");
		}	
}

//////////////////////////////////////////////////////////////////////////

LRESULT CLogWin::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 

{
	//AP2N("CLogWin::WindowProc %s\r\n", mxpad.num2msg(message));
	return CWnd::WindowProc(message, wParam, lParam);
}



