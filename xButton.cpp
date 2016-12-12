
/* =====[ xButton.cpp ]========================================================== 
                                                                             
   Description:     The wavfrag project, implementation of the xButton.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  10/5/2010  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include "xButton.h"
#include "mxpad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CxButton

CxButton::CxButton()

{
	//P2N("CxButton::CxButton()\r\n");
	xInitTip();
}

CxButton::~CxButton()
{

}

BEGIN_MESSAGE_MAP(CxButton, CButton)
	//{{AFX_MSG_MAP(CxButton)
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CxButton message handlers

void CxButton::OnMouseMove(UINT nFlags, CPoint point) 

{
	mouse = point;

	if(!capped)
		{
		capped = true;

		if(!fired)
			{
			TRACKMOUSEEVENT te;

			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_HOVER;
			te.hwndTrack = this->m_hWnd;
			te.dwHoverTime = tiptime;		
			::_TrackMouseEvent(&te);

			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = this->m_hWnd;		
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
				//AP2N("CxButton::moved mouse\r\n");
				xHideTip();
				}
			}
		}
	CButton::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

BOOL CxButton::PreTranslateMessage(MSG* pMsg) 

{
	if(pMsg->message == WM_MOUSELEAVE)
		{
		//AP2N("CxButton::PreTranslateMessage WM_MOUSELEAVE\r\n");
			
		TRACKMOUSEEVENT te;

		te.cbSize = sizeof(TRACKMOUSEEVENT);
		te.dwFlags = TME_HOVER | TME_CANCEL ;
		te.hwndTrack = this->m_hWnd;		
		::_TrackMouseEvent(&te);	

		te.cbSize = sizeof(TRACKMOUSEEVENT);
		te.dwFlags = TME_LEAVE | TME_CANCEL ;
		te.hwndTrack = this->m_hWnd;		
		::_TrackMouseEvent(&te);	

		xHideTip();
		tipcnt = 0;
		fired = capped = false;
		return 0;
		}

	if(pMsg->message == WM_MOUSEHOVER )
		{   
		//AP2N("CxButton::PreTranslateMessage WM_MOUSEHOVER\r\n");
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
	return CButton::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////

void CxButton::xShowTip()

{
	//AP2N("CxButton::ShowTip %d\r\n", tipcnt);
	
	CPoint screen;	GetCursorPos(&screen);
	//ClientToScreen(&screen); 

	int gap = GetSystemMetrics(SM_CXCURSOR) / 2;

	screen.x += gap / 2; screen.y += gap;

	if(tiptext != "")
		{		
		if(tipcnt < maxtipcnt)
			{
			TRACKMOUSEEVENT te;

			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = this->m_hWnd;
			::_TrackMouseEvent(&te);

			tip.Show(tiptext, screen);
			tipcnt++;
			}
		}
}

//////////////////////////////////////////////////////////////////////////

void CxButton::xHideTip()

{
	//AP2N("CxButton::xHideTip %d\r\n", tipcnt);

	tip.Hide();

	capped = false;
	fired = false;
}

//////////////////////////////////////////////////////////////////////////

CxButton& CxButton::SetTooltip(const CString& strText)

{
	tiptext = strText;
	return *this;
}


CxButton& CxButton::SetTooltip(int id)

{
	tiptext.LoadString(id);
	//AP2N("CxButton::SetTooltip %s\r\n", tiptext);
	return *this;
}
	
//////////////////////////////////////////////////////////////////////////

CxButton& CxButton::SetTooltip(const char *strText)

{
	tiptext = strText;
	return *this;
}

void	CxButton::xInitTip()

{
	capped = fired = false;
	tipcnt = 0;
	maxtipcnt = 1;
	tiptime = 1000;
}

void CxButton::Some()
{

}

BOOL	CxButton::OnEraseBkgnd(CDC* pDC)

{
	//P2N("CxButton::OnEraseBkgnd\r\n");
	return true;
}

void	CxButton::OnPaint()

{
	//P2N("CxButton::OnPaint\r\n");
	CButton::OnPaint();
}

