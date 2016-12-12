
/* =====[ label.cpp ]=====================================================
                                                                             
   Description:     Implam,entation of the label control. hame on MS
					for not having such a simple control.
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  10/27/2008  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include "mxpad.h"
#include "Label.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static TCHAR THIS_FILE[] = _T(__FILE__);
#endif

IMPLEMENT_DYNAMIC(CLabel, CStatic)

LRESULT CALLBACK CLabel::MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{ 
	//AP2N(_T("MainWndProc %s\r\n"), mxpad.num2msg(uMsg));

	return ::DefWindowProc(
		hwnd,      // handle to window
		uMsg,       // message identifier
		wParam,  // first message parameter
		lParam   // second message parameter
		);
}


/////////////////////////////////////////////////////////////////////////////
// CLabel

CLabel::CLabel()

{	
	//P2N(_T("Construct CLabel %p\r\n"), this);
	
	m_inited = m_bState = m_bLink = false;
	m_center = 	m_vcenter = m_right = m_frame = false;
	m_edge = m_modframe = m_border = false;

	m_transparent = true;

	m_crText = GetSysColor(COLOR_WINDOWTEXT);
	m_hBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));

	m_bkcolor = GetSysColor(COLOR_3DFACE);
	m_crText = GetSysColor(COLOR_BTNTEXT);

	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT),sizeof(m_lf),&m_lf);
	m_font.CreateFontIndirect(&m_lf);

	m_bTimer = 0;
	m_hCursor = NULL;
	m_Type = None;

	xInitTip();

	m_text = "none";
	
	//tiptext = "test TipText";

	m_hwndBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));	
}

CLabel::~CLabel()

{
	m_font.DeleteObject();

	::DeleteObject(m_hBrush);
	::DeleteObject(m_hwndBrush);
}

//////////////////////////////////////////////////////////////////////////

CLabel& CLabel::SetText(const CString& strText, int recalc)

{
	//GetWindowRect(&orig);

	m_text = strText;

	if(!IsWindow(m_hWnd))
		return *this;

	CStatic::SetWindowText(m_text);

	if(recalc)
		xCalcSize();

	xReDrawParent();	xReDraw();
	
	//P2N(_T("CLabel::SetText '%s'\r\n"), m_text);

	return *this;
}

CLabel& CLabel::SetTextP(const TCHAR * strText, ...)

{
	//GetWindowRect(&orig);

	CString strtmp;
	
	TCHAR *buff = strtmp.GetBuffer(STRTMP);
	va_list ArgList; va_start(ArgList, strText);
    _vsntprintf_s(buff, BYTE2TXT(STRTMP), _TRUNCATE, strText, ArgList);
    //_vsnprintf(buff, STRTMP, strText, ArgList);
	strtmp.ReleaseBuffer();

	SetText(strtmp);
	//P2N(_T("CLabel::SetTextP '%s'\r\n"), m_text);

	
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// Invalidate window we are sitting on, parent too if trans

void	CLabel::xReDraw()

{
	Invalidate();
	//support.YieldToWinEx();
}

//////////////////////////////////////////////////////////////////////////

CLabel& CLabel::SetTextColor(COLORREF crText)

{
	m_crText = crText;

	xReDraw();

	return *this;
}

CLabel& CLabel::SetFontBold(BOOL bBold)
{	
	m_lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;

	xReDrawParent();
	xReconstructFont();
	
	//xCalcSize();
	xReDraw();
	return *this;
}

CLabel& CLabel::SetFontUnderline(BOOL bSet)
{	
	m_lf.lfUnderline = bSet;
	xReconstructFont();
	//xCalcSize();
	xReDraw();
	return *this;
}

CLabel& CLabel::SetFontItalic(BOOL bSet)
{
	m_lf.lfItalic = bSet;
	xReconstructFont();
	//xCalcSize();
	xReDraw();
	return *this;	
}

CLabel& CLabel::SetSunken(BOOL bSet)
{
	if (!bSet)
		ModifyStyleEx(WS_EX_STATICEDGE,0,SWP_DRAWFRAME);
	else
		ModifyStyleEx(0,WS_EX_STATICEDGE,SWP_DRAWFRAME);
		
	return *this;	
}

CLabel& CLabel::SetBorder(BOOL bSet)
{
	if (!bSet)
		ModifyStyle(WS_BORDER,0,SWP_DRAWFRAME);
	else
		ModifyStyle(0,WS_BORDER,SWP_DRAWFRAME);
		
	m_border = bSet;

	return *this;	
}

CLabel& CLabel::SetFontSize(int nSize)

{
	xReDrawParent();

	nSize*=-1;
	m_lf.lfHeight = nSize;
	xReconstructFont();
	//xCalcSize();
	xReDraw();
	return *this;
}


CLabel& CLabel::SetBkColor(COLORREF crBkgnd)
{
	if (m_hBrush)
		::DeleteObject(m_hBrush);

	m_bkcolor = crBkgnd;

	m_hBrush = ::CreateSolidBrush(crBkgnd);
	return *this;
}

CLabel& CLabel::SetFontName(const CString& strFont)
{	
	_tcscpy_s(m_lf.lfFaceName, sizeof(m_lf.lfFaceName), strFont);
	//strcpy(m_lf.lfFaceName, strFont);

	xReDrawParent();
	xReconstructFont();
	xReDraw();
	//xCalcSize();
	return *this;
}

BEGIN_MESSAGE_MAP(CLabel, CStatic)
	//{{AFX_MSG_MAP(CLabel)
	//ON_WM_CTLCOLOR_REFLECT()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLabel message handlers

HBRUSH CLabel::CtlColor(CDC* pDC, UINT nCtlColor) 

{
	if (CTLCOLOR_STATIC == nCtlColor)
		{
		pDC->SelectObject(&m_font);
		pDC->SetTextColor(m_crText);
		pDC->SetBkMode(TRANSPARENT);
		//return NULL;
		}

	if (m_Type == Background)
		{
		if (!m_bState)
			return m_hwndBrush;
		}

	pDC->SetTextColor(m_crText);
	pDC->SetBkMode(TRANSPARENT);

	if(m_transparent)
		return NULL;

	return m_hBrush;
}

void CLabel::xReconstructFont()

{
	m_font.DeleteObject();
	BOOL bCreated = m_font.CreateFontIndirect(&m_lf);

	//ASSERT(bCreated);
}


CLabel& CLabel::FlashText(BOOL bActivate)
{
	if (m_bTimer)
	{
		//SetWindowText(m_strText);
		KillTimer(1);
	}

	if (bActivate)
	{
		//GetWindowText(m_strText);
		m_bState = FALSE;
		
		m_bTimer = TRUE;
		SetTimer(1,500,NULL);
		m_Type = Text;
	}

	return *this;
}

CLabel& CLabel::FlashBk(BOOL bActivate)
{

	if (m_bTimer)
		KillTimer(1);

	if (bActivate)
	{
		m_bState = FALSE;

		m_bTimer = TRUE;
		SetTimer(1,500,NULL);

		m_Type = Background;
	}

	return *this;
}


void CLabel::OnTimer(UINT nIDEvent) 
{
	m_bState = !m_bState;

	switch (m_Type)
	{
		case Text:
			if (m_bState)
				{
				m_backup = m_text;
				m_text = "";
				}
			else
				m_text = m_backup;
		break;

		case Background:
			InvalidateRect(NULL, FALSE);
			UpdateWindow();
		break;
	}
	
	CStatic::OnTimer(nIDEvent);
}

CLabel& CLabel::SetLink(BOOL bLink)

{
	m_bLink = bLink;

	if (bLink)
		ModifyStyle(0, SS_NOTIFY);
	else
		ModifyStyle(SS_NOTIFY,0);

	return *this;
}

void CLabel::OnLButtonDown(UINT nFlags, CPoint point) 

{
	CString strLink;

	SetFocus();

	//P2N(_T("Left button down on clabel %s\r\n"), m_text);

	if(m_bLink)
		{
		GetWindowText(strLink);
		ShellExecute(NULL, _T("open"), strLink, NULL, NULL, SW_SHOWNORMAL);
		}
	else
		{
		GetParent()->PostMessage(WM_COMMAND, 
					BN_CLICKED << 16 | GetDlgCtrlID(), (int)m_hWnd);
		}

	CStatic::OnLButtonDown(nFlags, point);
}

BOOL CLabel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 

{
	if (m_hCursor)
		{
		::SetCursor(m_hCursor);
		return TRUE;
		}
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

CLabel& CLabel::SetLinkCursor(HCURSOR hCursor)

{
	m_hCursor = hCursor;
	return *this;
}

//////////////////////////////////////////////////////////////////////////

void	CLabel::OnPaint()

{
	CPaintDC dc(this); // device context for painting

	//P2N(_T("Painting top=%d left=%d bottom=%d right=%d\r\n"), 
	//						dc.m_ps.rcPaint.top,
	//							dc.m_ps.rcPaint.left,
	//								dc.m_ps.rcPaint.bottom,
	//									dc.m_ps.rcPaint.right);

	if(!m_inited)
		xInitLab();

	RECT	rc;	GetClientRect(&rc);

	if(!m_transparent)
		dc.FillSolidRect(&rc, m_bkcolor);

	//else
	//	xReDrawParent();

	dc.SetTextColor(m_crText);	dc.SetBkMode(TRANSPARENT);
	CFont* oldFont = dc.SelectObject(&m_font);

	int xx = 0, yy = 0;
	//if(m_center)
	//	{
	//	CSize ss = dc.GetTextExtent(m_text);
	//	xx = (rc.right - rc.left)/2  - ss.cx/2;
	//	}
	//else if(m_right)
	//	{
	//	CSize ss = dc.GetTextExtent(m_text);
	//	xx = rc.right - ss.cx;
	//	}

	CRect rect(rc);

	if(m_border)
		rect.DeflateRect(4, 4);

	int flag = DT_WORDBREAK;

	if(m_center)
		flag |= DT_CENTER; 
	else if(m_right)
		flag |= DT_RIGHT; 

	if(m_vcenter)
		{
		flag |= DT_SINGLELINE;
		flag |= DT_VCENTER;
		}
	

	dc.DrawText(m_text, &rect, flag);

	//dc.TextOut(xx, yy, m_text);
	dc.SelectObject(oldFont);

	//P2N(_T("CLabel Paint %s\r\n"), m_text);

	//CStatic::OnPaint();
}

//////////////////////////////////////////////////////////////////////////
 
BOOL CLabel::OnEraseBkgnd(CDC *pDC)

{
	//P2N(_T("Erase Background %p\r\n"), this);
	return true;
}

void CLabel::GetText(CString &str)

{
	str = m_text;
}

//////////////////////////////////////////////////////////////////////////

const TCHAR *CLabel::GetText()

{
	return m_text;
}

void CLabel::CenterText(int flag)

{
	m_center = flag;
	xReDrawParent();	

	xReDraw();
}

void CLabel::CenterTextV(int flag)

{
	m_vcenter = flag;
	xReDrawParent();	

	xReDraw();
}

void CLabel::SetTrans(int flag)

{
	xReDrawParent();	
	m_transparent = flag;
	xReDrawParent();	

	xReDraw();
}

//////////////////////////////////////////////////////////////////////////
// Calculate text size, resize window

void CLabel::xCalcSize()

{
	PAINTSTRUCT Paint;

	if(!m_inited)
		xInitLab();
		
	CDC* dc = BeginPaint( &Paint );

	CFont* oldFont = dc->SelectObject(&m_font);
	textsize = dc->GetTextExtent(m_text);
	dc->SelectObject(oldFont);
	EndPaint(&Paint);

	int yframe = 2, xframe = 2;

	//if(	GetWindowLong(m_hWnd, GWL_STYLE) & SS_BLACKFRAME)

	if(m_frame)
		{
		//P2N(_T("Frame around it\r\n"));
		xframe += 4;
		yframe += 4;
		}
	if(m_edge )
		{
		xframe += 12;
		yframe += 12;
		}

	 if(m_modframe)
		{
		xframe += 12;
		yframe += 12;
		}

	//P2N(_T("xCalcSize %p Orig rect left=%d right=%d top=%d bottom=%d '%s'\r\n"),
	//				this, orig.left, orig.right, orig.top, orig.bottom, m_text);

	//P2N(_T("New text size: xx=%d yy=%d\r\n"), textsize.cx, textsize.cy);

	RECT prc(orig); 

	// Only mess with it if text is bigger than the window
	if((prc.right - prc.left) < textsize.cx)
		{		
		if(m_center)
			{
			// --------------------------------------------------
			//        | left        | mid            | right

			int hcnt = prc.left + (prc.right - prc.left) / 2;
			int vcnt = prc.top  + (prc.bottom - prc.top) / 2;
			
			//P2N(_T("New text pos: xx=%d yy=%d\r\n"), hcnt - textsize.cx/2, vcnt - textsize.cy/2);

			SetWindowPos(NULL, hcnt - textsize.cx/2, prc.top, 
								textsize.cx + xframe, textsize.cy + yframe, SWP_NOOWNERZORDER | SWP_NOMOVE);
			}
		else if(m_right)
			{
			SetWindowPos(NULL, prc.right - textsize.cx, prc.top, 
								textsize.cx + xframe, textsize.cy + yframe, SWP_NOOWNERZORDER | SWP_NOMOVE);
			}
		else
			{
			SetWindowPos( NULL, 0, 0, textsize.cx + xframe, textsize.cy + yframe, SWP_NOOWNERZORDER | SWP_NOMOVE);
			}
		}
}

//////////////////////////////////////////////////////////////////////////

void	CLabel::xReDrawParent()

{
	RECT prc; GetWindowRect(&prc);

	//AP2N(_T("Redrawing parent\r\n"));

	// xReDraw parent if transparent
	if(m_transparent)
		{
		GetParent()->ScreenToClient(&prc);
		GetParent()->InvalidateRect(&prc);
		}		
}

CLabel&		CLabel::operator =(const TCHAR *str)

{
	//P2N(_T("operator = %s\r\n"), str);
	
	SetText(str);

	return *this;
}


CLabel::operator const TCHAR *()

{
	//P2N(_T("operator (const TCHAR *) %s\r\n"), m_text);
	return (const TCHAR *)m_text;
}

//////////////////////////////////////////////////////////////////////////

void CLabel::xInitLab()

{
	m_inited = true;

	GetWindowRect(&orig);
	
	// Get text from resource
	if(m_text == "none")
		{
		GetWindowText(m_text);
		//P2N(_T("CLabel::xInitLab '%s'\r\n"), m_text);
		}

	//P2N(_T("Initial paint on '%s' \r\n"), m_text);

	//DumpStyles();
	//DumpExStyles();
	//DumpSS();

	// Carry parameters over from resource cstatic
	DWORD style = ::GetWindowLong(m_hWnd, GWL_STYLE);

	if(style & SS_CENTER)		
		{
		//P2N(_T(" center "));
		m_center = true;
		}
	else if(style & SS_RIGHT)		
		{
		//P2N(_T(" right "));
		m_right = true;
		}
	else
		{
		//P2N(_T(" left "));
		}

	if(style &  WS_BORDER)		
		{
		//P2N(_T("Border on '%s'\r\n"), m_text);
		//m_frame = true;
		}
	if(style & WS_EX_CLIENTEDGE )
		{
		//P2N(_T("Model Frame on '%s'\r\n"), m_text);
		}

	// CWnd
	DWORD stylex = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

	if(stylex & WS_EX_TRANSPARENT)
		{
		m_transparent = true;
		//P2N(_T("Transparent\r\n"));
		}
	if(stylex & WS_EX_STATICEDGE)
		{
		//P2N(_T("static edge  on '%s'\r\n"), m_text);
		m_edge = true;
		}
	if(stylex & WS_EX_DLGMODALFRAME)
		{
		//P2N(_T("static edge  on '%s'\r\n"), m_text);
		m_modframe = true;
		}

	//P2N(_T("\r\n"));
	//P2N(_T("left=%d right=%d top=%d bottom=%d\r\n\r\n"), 
	//			orig.left, orig.right, orig.top, orig.bottom);

	//xCalcSize();
}

void CLabel::DumpStyles()

{
	P2N(_T("Window Styles:\r\n"));
	DWORD style = ::GetWindowLong(m_hWnd, GWL_STYLE);

	if(style & WS_BORDER   )
		P2N(_T("WS_BORDER   "));
	if(style & WS_CAPTION   )
		P2N(_T("WS_CAPTION   "));
	if(style & WS_CHILD   )
		P2N(_T("WS_CHILD   "));
	if(style & WS_CLIPCHILDREN   )
		P2N(_T("WS_CLIPCHILDREN   "));
	if(style & WS_CLIPSIBLINGS   )
		P2N(_T("WS_CLIPSIBLINGS   "));
	if(style & WS_DISABLED   )
		P2N(_T("WS_DISABLED   "));
	if(style & WS_DLGFRAME   )
		P2N(_T("WS_DLGFRAME   "));
	if(style & WS_GROUP   )
		P2N(_T("WS_GROUP   "));
	if(style & WS_HSCROLL   )
		P2N(_T("WS_HSCROLL   "));
	if(style & WS_MAXIMIZE   )
		P2N(_T("WS_MAXIMIZE   "));
	if(style & WS_MAXIMIZEBOX   )
		P2N(_T("WS_MAXIMIZEBOX   "));
	if(style & WS_MINIMIZE   )
		P2N(_T("WS_MINIMIZE   "));
	if(style & WS_MINIMIZEBOX   )
		P2N(_T("WS_MINIMIZEBOX   "));
	if(style & WS_OVERLAPPED   )
		P2N(_T("WS_OVERLAPPED   "));
	if(style & WS_OVERLAPPEDWINDOW   )
		P2N(_T("WS_OVERLAPPEDWINDOW   "));
	if(style & WS_POPUP   )
		P2N(_T("WS_POPUP   "));
	if(style & WS_POPUPWINDOW   )
		P2N(_T("WS_POPUPWINDOW   "));
	if(style & WS_SYSMENU   )
		P2N(_T("WS_SYSMENU   "));
	if(style & WS_TABSTOP   )
		P2N(_T("WS_TABSTOP   "));
	if(style & WS_THICKFRAME   )
		P2N(_T("WS_THICKFRAME   "));
	if(style & WS_VISIBLE   )
		P2N(_T("WS_VISIBLE   "));
	if(style & WS_VSCROLL   )
		P2N(_T("WS_VSCROLL   "));

	P2N(_T("\r\n"));
}


void CLabel::DumpSS()

{
	P2N(_T("Static Window Styles:\r\n"));
	DWORD style = ::GetWindowLong(m_hWnd, GWL_STYLE);

	if(style & SS_BLACKFRAME   )
		P2N(_T("  SS_BLACKFRAME   "));
	if(style & SS_BLACKRECT   )
		P2N(_T("  SS_BLACKRECT   "));
	if(style & SS_CENTER   )
		P2N(_T("  SS_CENTER   "));
	if(style & SS_GRAYFRAME   )
		P2N(_T("  SS_GRAYFRAME   "));
	if(style & SS_GRAYRECT   )
		P2N(_T("  SS_GRAYRECT   "));
	if(style & SS_ICON   )
		P2N(_T("  SS_ICON   "));
	if(style & SS_LEFT   )
		P2N(_T("  SS_LEFT   "));
	if(style & SS_LEFTNOWORDWRAP   )
		P2N(_T("  SS_LEFTNOWORDWRAP   "));
	if(style & SS_NOPREFIX   )
		P2N(_T("  SS_NOPREFIX   "));
	if(style & SS_RIGHT   )
		P2N(_T("  SS_RIGHT   "));
	if(style & SS_SIMPLE   )
		P2N(_T("  SS_SIMPLE   "));
	if(style & SS_USERITEM   )
		P2N(_T("  SS_USERITEM   "));
	if(style & SS_WHITEFRAME   )
		P2N(_T("  SS_WHITEFRAME   "));
	if(style & SS_WHITERECT   )
		P2N(_T("  SS_WHITERECT   "));

	P2N(_T("\r\n"));
}

void CLabel::DumpExStyles()

{
	P2N(_T("Extended Window Styles:\r\n"));
	DWORD style = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

	if(style & WS_EX_ACCEPTFILES   )
		P2N(_T("  WS_EX_ACCEPTFILES  "));
	if(style & WS_EX_CLIENTEDGE   )
		P2N(_T("WS_EX_CLIENTEDGE  "));
	if(style & WS_EX_CONTEXTHELP   )
		P2N(_T("WS_EX_CONTEXTHELP  "));
	if(style & WS_EX_CONTROLPARENT   )
		P2N(_T("WS_EX_CONTROLPARENT  "));
	if(style & WS_EX_DLGMODALFRAME   )
		P2N(_T("WS_EX_DLGMODALFRAME  "));
	if(style & WS_EX_LEFT   )
		P2N(_T("WS_EX_LEFT  "));
	if(style & WS_EX_LEFTSCROLLBAR   )
		P2N(_T("WS_EX_LEFTSCROLLBAR  "));
	if(style & WS_EX_LTRREADING   )
		P2N(_T("WS_EX_LTRREADING  "));
	if(style & WS_EX_MDICHILD   )
		P2N(_T("WS_EX_MDICHILD  "));
	if(style & WS_EX_NOPARENTNOTIFY   )
		P2N(_T("WS_EX_NOPARENTNOTIFY  "));
	if(style & WS_EX_OVERLAPPEDWINDOW   )
		P2N(_T("WS_EX_OVERLAPPEDWINDOW  "));
	if(style & WS_EX_PALETTEWINDOW   )
		P2N(_T("WS_EX_PALETTEWINDOW  "));
	if(style & WS_EX_RIGHT   )
		P2N(_T("WS_EX_RIGHT  "));
	if(style & WS_EX_RIGHTSCROLLBAR   )
		P2N(_T("WS_EX_RIGHTSCROLLBAR  "));
	if(style & WS_EX_RTLREADING   )
		P2N(_T("WS_EX_RTLREADING  "));
	if(style & WS_EX_STATICEDGE   )
		P2N(_T("WS_EX_STATICEDGE  "));
	if(style & WS_EX_TOOLWINDOW   )
		P2N(_T("WS_EX_TOOLWINDOW  "));
	if(style & WS_EX_TOPMOST   )
		P2N(_T("WS_EX_TOPMOST  "));
	if(style & WS_EX_TRANSPARENT   )
		P2N(_T("WS_EX_TRANSPARENT  "));
	if(style & WS_EX_WINDOWEDGE   )
		P2N(_T("WS_EX_WINDOWEDGE  "));

	P2N(_T("\r\n"));
}

//////////////////////////////////////////////////////////////////////////

COLORREF	CLabel::GetBkColor()

{
	return(m_bkcolor);
}

void CLabel::PreSubclassWindow() 

{
	//AP2N(_T("CLabel::PreSubclassWindow\r\n"));
	
	ModifyStyle(0, WS_CLIPCHILDREN | SS_NOTIFY);
	ModifyStyleEx(WS_EX_STATICEDGE, 0, SWP_DRAWFRAME);
	
	//ModifyStyle(WS_BORDER | WS_DLGFRAME | WS_THICKFRAME, 0, SWP_DRAWFRAME);
	//ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME, 0, SWP_DRAWFRAME);
	//ModifyStyle(SS_BLACKFRAME, 0); 	ModifyStyle(SS_GRAYFRAME, 0);
	
	SetWindowLong(m_hWnd, GWL_WNDPROC, (DWORD)MainWndProc);

	CStatic::PreSubclassWindow();
}

//////////////////////////////////////////////////////////////////////////

void CLabel::xShowTip()

{
	//P2N(_T("CLabel::xShowTip\r\n"));
	
	CPoint screen;	GetCursorPos(&screen);
	//ClientToScreen(&screen); 

	int gap = GetSystemMetrics(SM_CXCURSOR) / 2;

	screen.x += gap / 2; screen.y += gap;

	if(tiptext != "")
		{		
		if(tipcnt < maxtipcnt)
			{
			//tip.Show(tiptext, screen);
			tipcnt++;
			}
		}
}

//////////////////////////////////////////////////////////////////////////

void CLabel::xHideTip()

{
	//tip.Hide();

	//P2N(_T("CLabel::xHideTip\r\n"));

	capped = false;
	fired = false;
}


void CLabel::OnMouseMove(UINT nFlags, CPoint point) 

{
	mouse = point;

	//AP2N(_T("CLabel::OnMouseMove %p x=%d y=%d\r\n"), this, point.x, point.y);

	if(!capped)
		{
		capped = true;

		if(!fired)
			{
			TRACKMOUSEEVENT te;

			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = this->m_hWnd;
			::_TrackMouseEvent(&te);
		
			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_HOVER;
			te.hwndTrack = this->m_hWnd;
			te.dwHoverTime = tiptime;		
			::_TrackMouseEvent(&te);
			}
		}	
	else 
		{
		if(fired)
			{
			// Moved more than hover pixels?
			if	(abs(capmouse.x - point.x) > 5 ||
					abs(capmouse.y - point.y) > 5)
				{
				xHideTip();
				}
			}
		}
}

BOOL CLabel::PreTranslateMessage(MSG* pMsg)

{
	//AP2N(_T("CLabel::PreTranslateMessage %p\r\n"), this);

	if(pMsg->message == WM_MOUSELEAVE)
		{
		//AP2N(_T("CLabel::PreTranslateMessage WM_MOUSELEAVE\r\n"));
		
		xHideTip();

		TRACKMOUSEEVENT te;

		te.cbSize = sizeof(TRACKMOUSEEVENT);
		te.dwFlags = TME_HOVER | TME_CANCEL ;
		te.hwndTrack = this->m_hWnd;		
		::_TrackMouseEvent(&te);	

		te.cbSize = sizeof(TRACKMOUSEEVENT);
		te.dwFlags = TME_LEAVE | TME_CANCEL ;
		te.hwndTrack = this->m_hWnd;		
		::_TrackMouseEvent(&te);	

		tipcnt = 0;
		fired = capped = false;
	
		return 0;
		}

	if(pMsg->message == WM_MOUSEHOVER )
		{   
		//AP2N(_T("CLabel::PreTranslateMessage WM_MOUSEHOVER\r\n"));
		fired = true;

		if(capped)
			{			
			TRACKMOUSEEVENT te;

			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_HOVER | TME_CANCEL ;
			te.hwndTrack = this->m_hWnd;		
			::_TrackMouseEvent(&te);	

			xShowTip();
			
			capmouse = mouse;
			capped = false;		
			}
		return 0;		
		}	

	return CStatic::PreTranslateMessage(pMsg);
} 

//////////////////////////////////////////////////////////////////////////

CLabel& CLabel::SetTooltip(const CString& strText)

{
	tiptext = strText;
	return *this;
}
	
//////////////////////////////////////////////////////////////////////////

CLabel& CLabel::SetTooltip(const TCHAR *strText)

{
	tiptext = strText;
	return *this;
}


//////////////////////////////////////////////////////////////////////////

void	CLabel::xInitTip()

{
	capped = fired = false;
	tipcnt = 0;
	maxtipcnt = 1;
	tiptime = 1000;
}

void CLabel::GetWindowText(CString& rString ) 

{
	CStatic::GetWindowText(rString);
	//P2N(_T("CLabel::GetWindowText %s\r\n"),rString);	
}

//////////////////////////////////////////////////////////////////////////

void CLabel::SetWindowText(CString &str)

{
	SetText(str);
}

//////////////////////////////////////////////////////////////////////////

void CLabel::SetWindowText(const TCHAR *str)

{
	SetText(str);
}

