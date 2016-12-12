
/* =====[ PersistDlg.cpp ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the PersistDlg.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  11/7/2007  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

// PersistDlg.cpp : implementation file

#include "stdafx.h"
#include "PersistDlg.h"
#include "mxpad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static TCHAR THIS_FILE[] = _T(__FILE__);
#endif

// Static members for the Persist class

DLGTEMPLATE PersistDlg::tmpl; 
DWORD		PersistDlg::padarr[3];

CString		PersistDlg::section(_T("Positions"));
CxPtrArr	PersistDlg::modals;
int			PersistDlg::resetpositions = 0;
int			PersistDlg::inmodal = 0;

/////////////////////////////////////////////////////////////////////////////
// PersistDlg dialog

PersistDlg::PersistDlg(CWnd* pParent /*=NULL*/)
	: CDialog(PersistDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(PersistDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	parent = pParent;
	xidd = PersistDlg::IDD;
	InitVars();
}

PersistDlg::PersistDlg(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)

{
	//P2N(_T("PersistDlg::PersistDlg this=%p parent=%p name='%s'\r\n"), 
	//									this, pParentWnd, windowname);

	parent = pParentWnd;
	xidd	= 	nIDTemplate;
	InitVars();
}

void PersistDlg::InitVars()

{
	//P2N(_T("PersistDlg::InitVars() this=%p name=%s\r\n"), 
	//									this, windowname);

	bAlpha = 255;
	
	magic = PERSIST_MAGIC;
	g_pSetLayeredWindowAttributesx = NULL;

	HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
	g_pSetLayeredWindowAttributesx = (lpfnSetLayeredWindowAttributes)
                      GetProcAddress(hUser32, "SetLayeredWindowAttributes");

	pre_want_x = pre_want_y = want_x  = want_y = 0;
	
	want_w = want_h = -1;
	
	oldAlpha = rdown = destroyed = shown = 0;
	
	showstate = pre_showstate = false;
	posload = modal = m_centerdlg = false;

	xdrag = GetSystemMetrics(SM_CXDRAG);
	ydrag = GetSystemMetrics(SM_CYDRAG);

	noremember = false;

	memset(&tmpl, 0, sizeof(tmpl));
	memset(&padarr, 0, sizeof(padarr));

	//tmpl.style = WS_POPUP | WS_THICKFRAME;
	tmpl.style = WS_POPUP ;

}

void PersistDlg::DoDataExchange(CDataExchange* pDX)

{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PersistDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PersistDlg, CDialog)
	//{{AFX_MSG_MAP(PersistDlg)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PersistDlg message handlers

BOOL PersistDlg::OnInitDialog() 

{
	CDialog::OnInitDialog();

	//P2N(_T("PersistDlg::OnInitDialog() IDD=%d\r\n"), xidd);
	
	SetAlpha(bAlpha);

	// Restore old placement, if any
	WINDOWPLACEMENT wp2;
	
	//CDialog *dlg = ((CDialog* )AfxGetApp()->m_pMainWnd);

	CString str2;

	if(resetpositions)
		{
		if(m_centerdlg)
			{
			CenterWindow();

			GetWindowPlacement(&wp2);

			pre_want_x = wp2.rcNormalPosition.left;
			pre_want_y = wp2.rcNormalPosition.top;
			pre_want_w = wp2.rcNormalPosition.right  - wp2.rcNormalPosition.left;
			pre_want_h = wp2.rcNormalPosition.bottom - wp2.rcNormalPosition.top;
			}
		else
			{
			// Reset, do nothing
			}
		}
	else
		{
		CString entity; entity.Format(_T("DlgPos%d"), xidd);

		str2 = AfxGetApp()->GetProfileString(section, entity, _T(""));
		
		if(str2 != "")
			{
			// Position acq from registry
			posload = true;
			
			//P2N(_T("PersistDlg::LoadPos IDD=%d pos='%s' name='%s'\r\n"), xidd, str2, windowname);

			//sscanf(str2, "%d %d %d %d",
			//	&wp2.rcNormalPosition.top, &wp2.rcNormalPosition.left,
			//		&wp2.rcNormalPosition.bottom,  &wp2.rcNormalPosition.right);

			_stscanf_s(str2, _T("%d %d %d %d"),
				&wp2.rcNormalPosition.top, &wp2.rcNormalPosition.left,
					&wp2.rcNormalPosition.bottom,  &wp2.rcNormalPosition.right);

			// Save old positions as pre_wants
			pre_want_x = wp2.rcNormalPosition.left;
			pre_want_y = wp2.rcNormalPosition.top;
			pre_want_w = wp2.rcNormalPosition.right  - wp2.rcNormalPosition.left;
			pre_want_h = wp2.rcNormalPosition.bottom - wp2.rcNormalPosition.top;
			}
		else 
			{				
			if(m_centerdlg)
				{
				GetWindowPlacement(&wp2);

				want_x = wp2.rcNormalPosition.left;
				want_y = wp2.rcNormalPosition.top;
				want_w = wp2.rcNormalPosition.right  - wp2.rcNormalPosition.left;
				want_h = wp2.rcNormalPosition.bottom - wp2.rcNormalPosition.top;
				}
			}

		CString entity2; entity2.Format(_T("DlgShow%d"), xidd);
		CString tmp2 = AfxGetApp()->GetProfileString(section, entity2, _T("1"));

		//P2N(_T("Load Visibility flag: %s\r\n"), tmp2);

		pre_showstate = _tstoi(tmp2);
		}

	// Execute the calculated dimentsions

	if(posload)
		{
		want_x = pre_want_x;
		want_y = pre_want_y;

		want_w = pre_want_w;
		want_h = pre_want_h;
		}

	// This did not work on multi screens
	//SetWindowPos(NULL, want_x, want_y, want_w, want_h,  flag);  
	
	MoveToPos();
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
// Save old positions and visibility

void PersistDlg::OnDestroy() 

{
	//P2N(_T("PersistDlg::OnDestroy() %p  IDD=%d name: '%s'\r\n"), this, xidd, windowname);

	destroyed = true;

	WINDOWPLACEMENT wp2; GetWindowPlacement(&wp2);

	int hh2  = wp2.rcNormalPosition.bottom - wp2.rcNormalPosition.top;	
	int ww2  = wp2.rcNormalPosition.right - wp2.rcNormalPosition.left;

	CString tmp; tmp.Format(_T("%d %d %d %d"),
		wp2.rcNormalPosition.top, wp2.rcNormalPosition.left,
				wp2.rcNormalPosition.bottom,  wp2.rcNormalPosition.right);	

	CString entity; entity.Format(_T("DlgPos%d"), xidd);
	
	//P2N(_T("PersistDlg::SavePos  %s IDD=%d Pos= '%s'\r\n"), windowname, xidd, tmp);
	
	AfxGetApp()->WriteProfileString(section, entity, tmp);

	CString tmp2; tmp2.Format(_T("%d"), showstate);

	//P2N(_T("PersistDlg::OnDestroy Showstate %s\r\n"), tmp2);

	CString entity2; entity2.Format(_T("DlgShow%d"), xidd);
	AfxGetApp()->WriteProfileString(section, entity2, tmp2);
	
	CDialog::OnDestroy();	
}


BOOL PersistDlg::PreTranslateMessage(MSG* pMsg) 

{
	//P2N(_T("PersistDlg::PreTranslateMessage %p %s\r\n"), this, mxpad.num2msg(pMsg->message));

#if 0

	// ReRoute messages to main window
	if(pMsg->message == WM_CHAR)
		{
		CDialog *dlg = ((CDialog* )AfxGetApp()->m_pMainWnd);
		dlg->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);

		//P2N(_T("PersistDlg pretrans WM_TCHAR %c\r\n"), pMsg->wParam);
		return(0);
		}

	if(pMsg->message == WM_KEYDOWN)
		{
		CDialog *dlg = ((CDialog* )AfxGetApp()->m_pMainWnd);
		dlg->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);

		//P2N(_T("PersistDlg::PreTrans WM_KEYUP %c (%d) %x\r\n"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
		return(0);
		}

	if(pMsg->message == WM_KEYUP)
		{
		CDialog *dlg = ((CDialog* )AfxGetApp()->m_pMainWnd);
		dlg->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);

		//P2N(_T("PersistDlg::PreTrans WM_KEYUP %c (%d) %x\r\n"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
		return(0);
		}

	if(pMsg->message == WM_SYSCHAR)
		{
		CDialog *dlg = ((CDialog* )AfxGetApp()->m_pMainWnd);
		dlg->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);

		//P2N(_T("PersistDlg::PreTrans WM_SYSTCHAR %c (%d) %x\r\n"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
		return(0);
		}

	if(pMsg->message == WM_SYSKEYDOWN)
		{
		CDialog *dlg = ((CDialog* )AfxGetApp()->m_pMainWnd);
		dlg->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);

		//P2N(_T("PersistDlg::PreTrans WM_SYSKEYDOWN %c (%d) %x\r\n"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
		return(0);
		}

	if(pMsg->message == WM_SYSKEYUP)
		{
		CDialog *dlg = ((CDialog* )AfxGetApp()->m_pMainWnd);
		dlg->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);

		//P2N(_T("PersistDlg::PreTrans WM_SYSKEYUP %c (%d) %x\r\n"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
		return(0);
		}

#endif

	return CDialog::PreTranslateMessage(pMsg);
}

void PersistDlg::OnShowWindow(BOOL bShow, UINT nStatus) 

{
	CDialog::OnShowWindow(bShow, nStatus);
	//P2N(_T("PersistDlg::OnShowWindow %d %d\r\n"), bShow, nStatus);
	showstate = bShow;	
}

void PersistDlg::OnPaint() 

{
	//CPaintDC dc(this); // device context for painting
	// Do not call CDialog::OnPaint() for painting messages
	// BLAH BLAH ...

	CDialog::OnPaint();
}


void PersistDlg::Show(int center)

{
	//P2N(_T("PersistDlg::Show wnd=%p wname='%s' xidd=%d\r\n"), m_hWnd, windowname, xidd);

	//ASSERT(magic == PERSIST_MAGIC);
	
	xCreate();
	
	if(IsWindowVisible())
		{
		SetActiveWindow();
		return;
		}
	if(center)
		{
		CenterWindow();
		}

	// Finally, ready to show
	ShowWindow(SW_SHOW);
	SetActiveWindow();

	shown = true;
}

void PersistDlg::Hide(int ret)

{
	//ASSERT(magic == PERSIST_MAGIC);
	
	//P2N(_T("PersistDlg::Hide() %s\r\n"), windowname);

	// If we never used this window, ignore request
	if(!m_hWnd || !::IsWindow(m_hWnd))
		{
		return;
		}

	// Terminate with the appropriate function:
	if(modal)
		{	
		//P2N(_T("PersistDlg::Hide() MODAL\r\n"));
		//modal = false;
		//((CShadesApp*)AfxGetApp())->inmodal--;

		// Will call 
		//EndDialog(ret);
		
		if(ret == IDOK)
			OnOK();
		else
			OnCancel();
		}
	else
		{
		ShowWindow(SW_HIDE);
		}
}

void PersistDlg::OnRButtonDown(UINT nFlags, CPoint point) 

{
	SetCapture();	rdown = true;
	CDialog::OnRButtonDown(nFlags, point);
}

void PersistDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();	rdown = false;
	CDialog::OnRButtonUp(nFlags, point);
}


void PersistDlg::OnMouseMove(UINT nFlags, CPoint point) 

{
	// Adjust transparency with right drag
	if(rdown)
		{
		if(oldpoint.y > point.y)
			bAlpha++;

		if(oldpoint.y < point.y)
			bAlpha--;

		if(oldpoint.x > point.x)
			bAlpha++;

		if(oldpoint.x < point.x)
			bAlpha--;
		
		// Limit alpha
		bAlpha = max(bAlpha, 50);
		bAlpha = min(bAlpha, 255);

		if(bAlpha != oldAlpha)
			{
			//P2N(_T("Adjusting alpha Here %d\r\n"), bAlpha);

			//  Call it with the new alpha - opacity
			if(g_pSetLayeredWindowAttributesx)
				g_pSetLayeredWindowAttributesx(m_hWnd, 0, bAlpha, LWA_ALPHA);

			Invalidate();
			}
		oldAlpha = bAlpha;
		oldpoint = point;
		}
	CDialog::OnMouseMove(nFlags, point);
}

int PersistDlg::DoModal()

{
	shown = true;
	int ret = 0;

	//P2N(_T("PersistDlg::DoModal '%s'\r\n"), windowname);
	
	//SetAlpha(245);

	m_centerdlg = true;

	// Maintain a modal dialog count
	inmodal++;
	
	// Count pending modal dialog loops
	modals.AddUniq(this);

	modal++; // = true;
	ret = CDialog::DoModal();
	modal--; // = false;

	modals.RemoveValue(this);

	m_centerdlg = false;

	inmodal--;
	
	//P2N(_T("PersistDlg::Done Modal '%s'\r\n"), windowname);

	return ret;
}

//////////////////////////////////////////////////////////////////////////

PersistDlg::~PersistDlg()

{
	//P2N(_T("PersistDlg::~PersistDlg() '%s'\r\n"), windowname);

	// Release modal loop if not released already
	if(modal)
		{
		P2N(_T("Forced Destruction from ~PersistDlg\r\n"));

		//((CShadesApp*)AfxGetApp())->inmodal--;
		modal = false;
		CDialog::EndDialog(IDOK);
		}

	if(!destroyed && shown)
		{
		//P2N(_T("PersistDlg::~PersistDlg() '%s' Not Destroyed! %p iswnd=%d\r\n"), 
		//					windowname, this, IsWindow(m_hWnd));

		if(IsWindow(m_hWnd))
			OnDestroy();
		}
}	

//////////////////////////////////////////////////////////////////////////

void PersistDlg::SetAlpha(int alpha)

{
	//P2N(_T("PersistDlg::SetAlpha(int alpha) %d\r\n"), alpha);

	bAlpha = alpha;
	oldAlpha = bAlpha;
	
	if (bAlpha == 255)
		{
		 // UnSet layered style for the dialog
	    SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		}
	else
		{
		 // Set layered style for the dialog
	    SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        //  Call it with alpha - opacity
		if(g_pSetLayeredWindowAttributesx)
			g_pSetLayeredWindowAttributesx(m_hWnd, 0, bAlpha, LWA_ALPHA);
		}
}

	
void PersistDlg::xCreate()

{
	//virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, 
	//const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

	if(!::IsWindow(m_hWnd))
		{
		//P2N(_T("PersistDlg::xCreate() %p '%s' want_x=%d want_y=%d xidd=%d\r\n"), 
		//				this, windowname, want_x, want_y, xidd);
		
		if(xidd)
			Create(xidd);		
		else
			{
			CreateIndirect(&tmpl, parent);
			ModifyStyleEx(0, WS_EX_DLGMODALFRAME);   
			}
		}
}

void PersistDlg::SetCenterFlag(int flag)

{
	m_centerdlg = flag;
}

//////////////////////////////////////////////////////////////////////////

void PersistDlg::MoveToPos(int xx, int yy)

{
	//P2N(_T("PersistDlg::MoveToPos %s -- (xx=%d yy=%d) x=%d y=%d w=%d h=%d\r\n"), 
		// windowname, xx, yy, want_x, want_y, want_w, want_h);

	if(noremember)
		return;

	int flag =  SWP_NOZORDER;

	// Never resize
	//if(want_w <= 0 || want_h <= 0)
		flag |= SWP_NOSIZE ;
		
	if(want_x < 0 || want_y < 0)
		flag |= SWP_NOMOVE ;	

	if(modal)
		flag |= SWP_NOSIZE;	

	// This needs to find the old size
	if(want_w <= 0 || want_h <= 0)
		{
		if(IsWindow(m_hWnd))
			{
			WINDOWPLACEMENT wp3; GetWindowPlacement(&wp3);
			want_w = wp3.rcNormalPosition.right  - wp3.rcNormalPosition.left;
			want_h = wp3.rcNormalPosition.bottom - wp3.rcNormalPosition.top;
			}
		}

	::MoveWindow(m_hWnd, want_x + xx, want_y + yy, want_w, want_h,  true); 
}