
/* =====[ Led.cpp ]========================================================== 
                                                                             
   Description:     The daco project, implementation of the Led.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  8/19/2010  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include "Led.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLed

CLed::CLed()

{
	state = 0;
	fired = GetTickCount();

	bgcolor = GetSysColor(COLOR_3DFACE);

	bodycol = RGB(128,128,128);

	//lightcol = RGB(255,0,0);
	//lightcol2 = RGB(200,100,0);
	//lightcol3 = RGB(255,100,100);	

	lightcol = RGB(0,255,0);
	lightcol2 = RGB(100,200, 0);
	lightcol3 = RGB(100,200, 000);	

}

CLed::~CLed()

{
}

BEGIN_MESSAGE_MAP(CLed, CStatic)
	//{{AFX_MSG_MAP(CLed)
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLed message handlers

void CLed::OnPaint() 

{
	CPaintDC dc(this); // device context for painting

	RECT rc; GetClientRect(&rc);

	//dc.FillSolidRect(&rc, bgcolor);

	COLORREF col, col2, col3;
	
	if(state)
		{
		col = lightcol;
		col2 = lightcol2;
		col3 = lightcol3;
		}
	else
		{
		col = bodycol;
		col2 = bodycol;
		col3 = bodycol;
		}

	CBrush brush; brush.CreateSolidBrush(col); 
	CPen pen; pen.CreatePen(PS_SOLID, 1, col2);
	
	dc.SelectObject(&pen); dc.SelectObject(&brush);
	dc.Ellipse(&rc);

#if 1
	if(state)
		{
		int		dd = 4;
		int		ww = rc.right - rc.left; int hh = rc.bottom - rc.top;

		RECT rc2; 
		rc2.top = hh / 2 - dd; 	rc2.left = ww / 2 + dd ; 
		rc2.right = ww/2 - dd; rc2.bottom = hh/2 + dd;

		CBrush brush2;	brush2.CreateSolidBrush(col3);
		CPen pen2; 	pen2.CreatePen(PS_SOLID, 1, col3);

		dc.SelectObject(&pen2); dc.SelectObject(&brush2);
		dc.Ellipse(&rc2);
		}
#endif

	// Do not call CStatic::OnPaint() for painting messages
}

void CLed::PreSubclassWindow() 
{
	//AP2N("CLed::PreSubclassWindow() \r\n");

	ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, WS_EX_TRANSPARENT);
	//ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, 0);
	
	//ModifyStyle(0, SS_NOTIFY | WS_BORDER);
	//ModifyStyle(0, SS_NOTIFY);
	
	CStatic::PreSubclassWindow();
}

//////////////////////////////////////////////////////////////////////////

void CLed::SetState(int ss)

{
	if(state == ss)
		return;

	state = ss;
	DelayedInval();
}

//////////////////////////////////////////////////////////////////////////

void CLed::DelayedInval()

{
	DWORD  tick = GetTickCount();

	// Timer has not been retriggerd for too long, force it
	if(tick - fired > 400)
		{
		KillTimer(2);

		if(IsWindow(m_hWnd))
			Invalidate();
		}
	else
		{
		SetTimer(2, 200, NULL);
		fired = tick;
		}
}

//////////////////////////////////////////////////////////////////////////

void CLed::OnTimer(UINT nIDEvent) 

{
	if(nIDEvent == 2)
		{
		KillTimer(nIDEvent);

		if(IsWindow(m_hWnd))
			Invalidate();

		fired = GetTickCount();
		}
	
	CStatic::OnTimer(nIDEvent);
}

