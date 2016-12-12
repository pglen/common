// Popup.cpp : implementation file
//

#include "stdafx.h"

#include "support.h"
#include "misc.h"

#include "Popup.h"
#include "mxpad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static TCHAR THIS_FILE[] = _T(__FILE__);
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopup dialog

CPopup::CPopup(int rc)

{
	xInitVars();
	rcid = rc;
}

//////////////////////////////////////////////////////////////////////////

CPopup::CPopup()
	
{
	xInitVars();
}

//////////////////////////////////////////////////////////////////////////

void	CPopup::xInitVars()

{
	//{{AFX_DATA_INIT(CPopup)
	//}}AFX_DATA_INIT

	parent	= GetDesktopWindow();
	//parent = AfxGetApp()->m_pMainWnd;

	//myww = myhh = 0;
	padding = 20;
	first =  0;
	bgcolor = RGB(0xff, 0xff, 0xee);
	
	rcid = 0;
	idletime = 18000;

	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT),sizeof(m_lf),&m_lf);
	m_font.CreateFontIndirect(&m_lf);

	m_lf.lfWeight = FW_BOLD;
	m_fontb.CreateFontIndirect(&m_lf);
}

void CPopup::DoDataExchange(CDataExchange* pDX)

{
	CEdit::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPopup)
	//DDX_Control(pDX, IDC_TEXT, m_str);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPopup, CEdit)
	//{{AFX_MSG_MAP(CPopup)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopup message handlers

BOOL CPopup::OnEraseBkgnd(CDC* pDC) 

{
	RECT rect; 
	GetClientRect(&rect);

	CBrush NewBrush;

	//NewBrush.CreateSolidBrush(RGB(0xff, 0xff, 0x80)) ;
	NewBrush.CreateSolidBrush(bgcolor) ;

	pDC->SetBrushOrg(0,0) ;
	CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&NewBrush);
	pDC->PatBlt(rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, PATCOPY);
	pDC->SelectObject(pOldBrush);
	
	//return CTransDlg::OnEraseBkgnd(pDC);
	return(false);
}

#if 0

BOOL CPopup::OnInitDialog() 
{
	CEdit::OnInitDialog();
	
	BYTE	bAlpha = 255;

	if (bAlpha < 255)
		{
        //  Set layered style for the dialog
        SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        //  Call it with 255 as alpha - opacity
		if(g_pSetLayeredWindowAttributes)
			g_pSetLayeredWindowAttributes(m_hWnd, 0, bAlpha, LWA_ALPHA);
		}

	//::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(m_lf), &m_lf);
	//m_lf.lfHeight = -26;
	//m_font.CreateFontIndirect(&m_lf);

	//m_str.SetTrans();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#endif


//////////////////////////////////////////////////////////////////////////

void CPopup::xPopAutoSize()
	
{
	PAINTSTRUCT Paint;
	
	xCreate();

	if(!IsWindow(m_hWnd))
		return;

	CDC* dc = BeginPaint(&Paint);
	CFont* oldFont = dc->SelectObject(&m_font);

	//CSize textsize = dc->GetTextExtent(str);
	
	CRect rc;
	dc->DrawText(str, &rc, DT_CALCRECT | DT_EXPANDTABS | DT_NOPREFIX );
	dc->SelectObject(oldFont);
	EndPaint(&Paint);

	size.cx = rc.Width() + padding;
	size.cy = rc.Height() + padding;

	//myhh = rc.Height() + padding;
}

//////////////////////////////////////////////////////////////////////////

void CPopup::xPopAutoPos()

{
	CRect	rc3; misc.CurrentMonRect(here, &rc3);

	//P2N(_T("GetMonitorInfo left=%d right=%d top=%d bottom=%d\r\n"),  
	//					rc3.left, rc3.right, rc3.top, rc3.bottom);

	// Needs a turn around to show to the left
	if(here.x + size.cx > rc3.right)
		{
		//P2N(_T("Needs a turn around to show to the left\r\n"));		
		here.x = rc3.right - size.cx - 20;
		}

	// Needs a turn around to show to the upper side
	if(here.y + size.cy > rc3.bottom)
		{
		//P2N(_T("Needs a turn around to show to the upper side\r\n"));
		here.y =  rc3.bottom -  size.cy - 20;
		}
}

CString CPopup::GetText()

{
	return str;
}
//////////////////////////////////////////////////////////////////////////

void CPopup::SetText(const TCHAR *xstr)

{
	//P2N(_T("Setting tooltip text: '%s'\r\n"), xstr);

	str = xstr;

#if 0
	// Autowrap to size
	if(maxwidth)
		{
		int idx = 0;
		while(true)
			{
			idx += maxwidth;

			if(idx > str.GetLength())
				break;

			// Wobble no more than half length
			for(int loop = 0; loop < maxwidth/2; loop++)
				{
				if(isspace(str.GetAt(idx - loop)) )
					{
					idx -= loop - 1;
					break;
					}
				}
			str.Insert(idx, "\r\n");
			}
		}

#endif

	xPopAutoSize();
}

void CPopup::Move(CPoint screen)

{
	//xCreate();

	here = screen;

	//SetWindowPos(&wndTopMost, screen.x, screen.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
	//SetWindowPos(NULL, screen.x, screen.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
	//SetWindowPos(NULL, here.x, screen.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER  );
	//PopAutoSize();
}

void CPopup::OnPaint() 

{
	if(!first++)
		{
		//P2N(_T("CPopup::OnPaint First paint\r\n"));
		//ModifyStyle(WS_DISABLED, WS_THICKFRAME | SS_NOTIFY | WS_VISIBLE, SWP_DRAWFRAME);
		//ModifyStyle(WS_DISABLED, WS_BORDER | /**/SS_NOTIFY | WS_VISIBLE, SWP_DRAWFRAME);
		ModifyStyle(WS_DISABLED, WS_DLGFRAME       | /**/SS_NOTIFY | WS_VISIBLE, SWP_DRAWFRAME);
		//ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_DRAWFRAME);
		SetFocus();
		}

	CPaintDC dc(this); // device context for painting

	CFont* oldFont = dc.SelectObject(&m_font);

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(0,0,0));	

	CRect rc; GetClientRect(&rc);
	rc.left += padding/2; rc.top += padding/3;

	//CString leftstr, rightstr;
	//int idx = str.Find(_T("<b>"));
	//if(idx)
	//	{
	//	dc.SelectObject(&m_fontb);
	//	leftstr = str.Left(idx);
	//	rightstr = str.Mid(idx+3);
	//	dc.DrawText(str, &rc, DT_NOPREFIX | DT_LEFT | DT_EXPANDTABS);
	//	}
	//else
	//	{
	//	dc.DrawText(str, &rc, DT_NOPREFIX | DT_LEFT | DT_EXPANDTABS);
	//	}

	dc.DrawText(str, &rc, DT_NOPREFIX | DT_LEFT | DT_EXPANDTABS);

	dc.SelectObject(oldFont);

	// Do not call CEdit::OnPaint() for painting messages
}

void CPopup::OnTimer(UINT nIDEvent) 

{
	//P2N(_T("Timer Fired\r\n"));  EndDialog(IDOK);

	KillTimer(nIDEvent);

	Hide();

	CEdit::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////

void CPopup::Show()

{
	//P2N(_T("CPopup::Show: '%s' %d %d -- %d %d\r\n"), 
	//							str, here.x, here.y, size.cy, size.cy);

	xCreate();

	if(!IsWindow(m_hWnd))
		return;


	xPopAutoSize();	xPopAutoPos();

	SetTimer(1, idletime, NULL);

	//BringWindowToTop(); ShowWindow(SW_SHOW);	
	tipped = GetTickCount();		

	// Commit it here
	SetWindowPos(&wndTop, here.x, here.y, size.cx, size.cy,
									SWP_SHOWWINDOW | SWP_NOACTIVATE);  	
}

//////////////////////////////////////////////////////////////////////////

void CPopup::Hide()

{	 
	//AP2N(_T("CPopup::Hide() %p\r\n"), this);

	xCreate();

	if(!IsWindow(m_hWnd))
		return;

	KillTimer(1);	
	ShowWindow(SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////

void CPopup::xCreate()

{
	//AP2N(_T("CPopup::xCreate() %p\r\n"), this);

	if(!IsWindow(m_hWnd))
		{
		//AP2N(_T("CPopup::xCreate() actual %p\r\n"), this);

		RECT rc;  ::SetRect(&rc, 2, 2, 12, 12);

		//memset(&tmpl, 0, sizeof(tmpl));
		//memset(&padarr, 0, sizeof(padarr));
		//tmpl.style = WS_POPUP | WS_THICKFRAME;
		//tmpl.style = WS_POPUP ;

		//CreateIndirect(&tmpl, parent);
		//ModifyStyleEx(0, WS_EX_DLGMODALFRAME);   

		int dlgid = 1010;
			
		while (true)
			{
			int ret = CWnd::Create(NULL, NULL, WS_DISABLED, rc, parent, dlgid);
			
			// Created 
			if(ret) break;
			
			// Safety net
			if(dlgid++ > 4000)
				break;
			}

		//P2N(_T("Create ret=%d err=%s\r\n"), ret, strerror(GetLastError()));
		////ASSERT(ret != 0);
	
		//ModifyStyle(WS_DISABLED,  WS_THICKFRAME | SS_NOTIFY |WS_VISIBLE, SWP_DRAWFRAME);
		//ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_DRAWFRAME);
		//P2N(_T("CPopup::xCreate ret=%d dlgid=%d\r\n"), IsWindow(m_hWnd), dlgid);
		}
	
	////ASSERT(IsWindow(m_hWnd));	
}

//////////////////////////////////////////////////////////////////////////
// Overloaded to do everything

void	CPopup::Show(const TCHAR *str, CPoint screen)

{
	//P2N(_T("CPopup::Show('%s' screen x=%d y=%d\r\n"),
	//									str, screen.x, screen.y);

	Move(screen); SetText(str); Show();
}

