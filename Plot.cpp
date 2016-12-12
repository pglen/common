
/* =====[ Plot.cpp ]========================================== 
                                                                             
   Description:     The daco project, implementation of the Plot.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  9/8/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

// Plot.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>

#include "Plot.h"
#include "GetText.h"
#include "mxpad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define		ID_PLOT_RCLICLK_COPY	(WM_USER + 100)
#define		ID_PLOT_RCLICLK_PASTE	(WM_USER + 101)
#define		ID_PLOT_RCLICLK_PASTE2	(WM_USER + 102)
#define		ID_PLOT_RCLICLK_CLEAR	(WM_USER + 103)
#define		ID_PLOT_RCLICLK_AUTO	(WM_USER + 104)
#define		ID_PLOT_RCLICLK_SMOOTH	(WM_USER + 105)
#define		ID_PLOT_RCLICLK_SMOOTH2	(WM_USER + 106)
#define		ID_PLOT_RCLICLK_SMOOTH3	(WM_USER + 107)
#define		ID_PLOT_RCLICLK_LABEL	(WM_USER + 108)

#define SEPARATOR ""

/////////////////////////////////////////////////////////////////////////////
// CPlot

CPlot::CPlot()

{
	//TRACE("CPlot::CPlot %p\r\n", this);
	magic = PLOT_MAGIC;
	
	xold_max = ytimer = xscroll = xstep = 0, lastmove = 0;
	xhor_gap = 5;				// Gap size at the sides
	xver_gap = 5;				// Gap size at the top/bottom
	xwasdouble = 0;

	xmag = 0;
	
	xinited = xchanged = xtimed = 0;
	xcformat = RegisterClipboardFormat("PlotData");

	lastpoint.x = lastpoint.y = 0;

	xlabel = "No Label";
	
	lastclock = clock();
}


CPlot::~CPlot()
{
	//P2N("CPlot::CPlot %p\r\n", this);
}


BEGIN_MESSAGE_MAP(CPlot, CStatic)
	//{{AFX_MSG_MAP(CPlot)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlot message handlers

void CPlot::OnPaint() 

{	
	static in = 0;

	if(in)
		return;

	in = true;

	xscroll = max(xscroll, 0);

	CPaintDC dc(this); // device context for painting
	
	int offset = 0, sdc = dc.SaveDC();

	double	loop;
	
	RECT rc; GetClientRect(&rc);
	int midline = rc.bottom / 2;

	if(!xinited)
		{
		xinited = true;

		dc.FillSolidRect(&rc, RGB(220,220,220));
		dc.Draw3dRect(&rc, RGB(200,200,200), RGB(100,100,100));

		SetTimer(1, 200, NULL);
		}

	//try
	//{
	// Estabilish range
	int at2 = 0, ss = xint_arr.GetSize();	
	for(loop = 0; loop < ss; loop++)
		{
		int xx = xint_arr.GetAt((int)loop);
		at2 = max(at2, abs(xx));
		}

	if(at2 == 0) 
		at2 = 1;  // contain it

	xold_max = at2;
	
	//P2N("max at2=%d\r\n", at2);
	
	dc.SetBkColor(RGB(220, 220, 220));
	dc.SetTextColor(RGB(120, 120, 120));

	if(ss==0)
		{
		CString str; str.Format(
			"Left drag to scroll, Right drag to magnify.");
		CString str2; str2.Format(
			"Left dblclick to Reset, "
			"Right dblclick to Fit.");

		dc.TextOut(2, 2, str);
		
		int lx = 2, ly = 20;

		if(rc.right - rc.left > 600)
			lx = 280, ly = 2;

		dc.TextOut(lx, ly, str2);
		}
	else
		{
		int val = 0;
		if(xscroll < xint_arr.GetSize() && xscroll >= 0)
			val = xint_arr.GetAt((int)xscroll);

		CString num; num.Format("%d", at2);	  xAddCommas(num);
		CString num2; num2.Format("%d", val); xAddCommas(num2);
		
		CString str2; str2.Format(
			"Mag=%-4.2f Scroll=%-4d Currval=%s Maxval=%s Items=%-6d", 
					xmag, xscroll, num2, num, ss);
		
		CSize ss = dc.GetTextExtent(str2);
		
		dc.TextOut(rc.right - ss.cx - xhor_gap, rc.bottom - (ss.cy + 2), str2);
		}

	//dc.TextOut(2, 22, str2);
	//str.Format("min=%06d", at3);
	//dc.TextOut(lgap, rc.bottom - 18, str);

	// Draw middle line
	CPen pen(PS_SOLID, 1, RGB(180, 180, 180));
	CPen *oldpen = (CPen*)dc.SelectObject(pen);

	dc.MoveTo(rc.left + xhor_gap, midline);
	dc.LineTo(rc.right - xhor_gap, midline);

	// Draw ticks
	int cnt = 0, tick = 10 ; //(rc.right - rc.left) / 100;
	for (int loopm = xhor_gap; loopm < rc.right - xhor_gap; loopm += tick)
		{
		int ticksize = 8; if(cnt++ % 10 == 0) ticksize = 16;

		//dc.MoveTo(rc.left + xhor_gap + loopm, rc.bottom / 2 - ticksize);
		dc.MoveTo(rc.left + xhor_gap + loopm, midline);
		dc.LineTo(rc.left + xhor_gap + loopm, midline + ticksize);
		}

	//////////////////////////////////////////////////////////////////////
	
	double fact =  ((rc.bottom - rc.top - 2 * xver_gap) / 2);
	double mag2 = 1 + xmag/100;

	// Draw Markers
	CPen pen2(PS_SOLID, 1, RGB(128, 128, 228));

	dc.SelectObject(pen2);
	int alen = xmark_arr.GetSize();
	for(int loopk = 0; loopk < alen; loopk++)
		{
		int posi = xmark_arr.GetAt(loopk);

		posi -= xscroll;	

		if(mag2 > 0)
			posi = int(double(posi) / mag2);
		
		// Clip to window
		if(posi > xhor_gap && posi < rc.right - xhor_gap)
			{
			dc.MoveTo(rc.left + xhor_gap + posi, midline - 20);
			dc.LineTo(rc.left + xhor_gap + posi, midline + 20);
			}	
		}

	dc.MoveTo(rc.left + xhor_gap, midline);

	dc.SelectStockObject(BLACK_PEN);

	// Preset 
	if(ss)
		{
		int atp, zzp = 0;
		if(xscroll < xint_arr.GetSize() && xscroll >= 0) 
			zzp = xint_arr.GetAt((int)xscroll);

		atp = (int)((fact * zzp)  / at2);
		dc.MoveTo(rc.left + offset + xhor_gap, midline - atp);
		}

	//////////////////////////////////////////////////////////////////////
	// Put out coordinate idles and peaks

	if(ss)
		{
		int edge, eval, found = false;;
		int hhh = rc.bottom / 2 - rc.top / 2;
		
		// Calc real pos
		int val = 0, xxx = lastpoint.x - xhor_gap;
		xxx *= (int)mag2;

		xxx += xscroll; xxx = MAX(xxx, 0);
		if(xxx > xint_arr.GetSize() - 1)
			xxx = 0; 			
		else
			val = xint_arr.GetAt(xxx);	
			
		for(int loop2 = 0; loop2 < xarr.GetSize(); loop2++)
			{
			edge = xarr.GetAt(loop2);
			eval = varr.GetAt(loop2);
			
			//AP2N("%d\r\n", edge);

			if(ABS(xxx - edge) < 6)
				{
				//AP2N("found edge %d\r\n", edge);
				found = true;
				break;
				}
			}
			
		CString str3; 
		if(found)
			str3.Format(" x=%-3d  y=%-3d edge=%d (%d)", xxx, val, edge, eval);
		else
			str3.Format(" x=%-3d  y=%-3d", xxx, val);

		CSize ss2 = dc.GetTextExtent(str3);
		dc.TextOut(xhor_gap, rc.bottom - (ss2.cy + 2), str3);
		}
	
	try
	{
	for(loop = xscroll; loop < ss; loop += mag2)
		{
		int	zz = 0;

		// Sum up mltiple entries

		if(mag2)
			{
			if(loop + mag2 < ss)
				{
				for(int loop2 = (int)loop; loop2 < loop + mag2; loop2++)
					zz =+ xint_arr.GetAt(loop2);

				//if(mag2 > 1)
				//	zz = (int) (((double)zz) / mag2);
				}
			}
		else
			{
			zz = xint_arr.GetAt((int)loop);	
			}

		int at = (int)((fact * zz)  / at2);
		int lpos = rc.left + offset + xhor_gap;
		
		// Draw line, clip it
		if(lpos < rc.right - xhor_gap)
			{
			//dc.MoveTo(rc.left + offset + lgap, rc.bottom / 2);
			dc.LineTo(lpos, midline - at);
			}
		else
			{
			// End of window
			break;
			}

		offset++;
		}
	}
	catch(...)
		{
		AP2N("****** Exception in plot ****** \r\n");
		//throw;
		}

	//arrloc.Unlock();

	CString str3; str3.Format("%s", xlabel);

	CSize tx = dc.GetTextExtent(str3);
	dc.TextOut(rc.right - (tx.cx + 5), 2, str3);

	dc.RestoreDC(sdc);

	in = false;
}

//////////////////////////////////////////////////////////////////////////

void CPlot::AddLine(double val)

{
	int cvt;

	if(val < 0)
		cvt = (int)ceil(val);
	else
		cvt = (int)floor(val);

	AddLine(cvt);
}

//////////////////////////////////////////////////////////////////////////

void CPlot::AddIntArr(CIntArr *parr)


{
	int alen = parr->GetSize();
	for(int loop = 0; loop < alen; loop++)
		{
		AddLine(parr->GetAt(loop));
 		}
}

//////////////////////////////////////////////////////////////////////////

void CPlot::AddLine(int hight)

{
	RECT rc; GetClientRect(&rc);
	
	//P2N("line %d\r\n", hight);

	//if(hight > rc.bottom - 15)
	//	{
	//	hight = rc.bottom - 15;
	//	}

	//if(xint_arr.GetSize() > (rc.right - 9))
	//	{
	//	xint_arr.RemoveAt(0, 20);
	//	inited = false;
	//	}

	if(!xchanged && !xtimed)
		{
		SetTimer(1, 200, NULL); xtimed = true;
		}

	xchanged = true;

	arrloc.Lock();

	xint_arr.Add(hight);
	
	// Throw away some if too big
	if(xint_arr.GetSize() > 10000)
		{
		xint_arr.RemoveAt(0, 1000);

		// Correct markers
		int alen = xmark_arr.GetSize();
		for(int loop = 0; loop < alen; loop++)
			{
			xmark_arr[loop] -= 1000;
 			}
		}

	arrloc.Unlock();

	//int asize = xint_arr.GetSize();
	//if(asize > 10000)
	//	mag = 10;

	//WINDOWPLACEMENT wp;
	//GetWindowPlacement(&wp);
	
	//int ww = rc.right - rc.left;

	//if(asize > ww + 40 + scroll)
	//	{
	//	//P2N("Added scroll %d\r\n", scroll);
	//	scroll = asize - ww;
	//	Invalidate();
	//	support.YieldToWinEx();
	//	}

	// Self invalidate after some work
	//if(asize % 2000 == 0)
	//	{
	//	Invalidate();
	//	support.YieldToWin();
	//	}
}


BOOL CPlot::OnEraseBkgnd(CDC* pDC) 

{
	//P2N("Erase background\r\n");

	RECT rc; GetClientRect(&rc);

	pDC->FillSolidRect(&rc, RGB(220,220,220));
	pDC->Draw3dRect(&rc, RGB(200,200,200), RGB(100,100,100));

	//pDC->TextOut(lgap, rc.bottom - 18, "aaaa");
	//inited = true;
	//return CStatic::OnEraseBkgnd(pDC);

	return(true);
}

void CPlot::AddMarker()

{
	//AddLine(0);

	//AddLine(old_max);
	//AddLine(-old_max);
	
	//AddLine(0);

	xmark_arr.Add(xint_arr.GetSize());
}

//////////////////////////////////////////////////////////////////////////

void CPlot::ClearAll()

{
	xinited = false;

	xint_arr.RemoveAll(); 	xmark_arr.RemoveAll();

	SetTimer(1, 200, NULL);	xtimed = true;

	if(IsWindow(m_hWnd))
		Invalidate(false);
	
	//support.YieldToWinEx();
}

void CPlot::OnMouseMove(UINT nFlags, CPoint point) 

{
	//P2N("Scroll by: %d old=%d\r\n", point.x, point.y);


	if(MK_RBUTTON & nFlags )
		{
		//P2N("Scroll by: %d old=%d  diff=%d  ", xold.x, point.x, point.x - xold.x);

		if(MK_SHIFT  & nFlags )	
			{
			xmag -= (point.x - xoldpoint.x) * 20;
			}
		else
			{
			xmag -= (point.x - xoldpoint.x) * 2;
			}
		
		xmag = max(xmag, -100);

		//P2N("Mag=%f\r\n", 1 + mag/100);
			
		Invalidate();
		}

	if(MK_LBUTTON & nFlags )
		{
		if(MK_SHIFT  & nFlags )	
			{	
			xscroll -= (point.x - xoldpoint.x) * 20;
			}
		else
			{
			xscroll -= (point.x - xoldpoint.x) * 2;
			}

		xscroll = max(xscroll, 0);

		if(xscroll > xint_arr.GetSize() - 1)
			xscroll = xint_arr.GetSize() - 1;

		//P2N("Scroll=%d\r\n", scroll);
		Invalidate();
		}

	xoldpoint.x = point.x; 	xoldpoint.y = point.y;

	lastmove = clock();
	
	if(!ytimer)
		{
		lastpoint = point;
		ytimer = true;
		SetTimer(2, 100, NULL);
		}

	CStatic::OnMouseMove(nFlags, point);
}

void CPlot::OnLButtonDblClk(UINT nFlags, CPoint point) 

{
	xmag = 0;	xscroll = 0;

	Invalidate();

	CStatic::OnLButtonDblClk(nFlags, point);
}

void CPlot::OnRButtonDown(UINT nFlags, CPoint point) 

{
	xwasdouble = 0;
	downpoint = point;
	CStatic::OnRButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

void CPlot::OnRButtonDblClk(UINT nFlags, CPoint point) 

{
	// Stop meu from popup
	xwasdouble = true;
	KillTimer(3);

	RECT rc; GetClientRect(&rc);
	unsigned int ss = xint_arr.GetSize();

	xmag = ss / (rc.right - rc.left) * 100;
	//P2N("ss=%d  rcwidth=%d mag=%f\r\n", ss, rc.right - rc.left, 1 + mag/100);
	xscroll = 0;

	Invalidate();
	
	CStatic::OnRButtonDblClk(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

void CPlot::OnTimer(UINT nIDEvent) 

{		
	//AP2N("CPlot::OnTimer %d\r\n", nIDEvent);

	switch(nIDEvent)
		{
		case 1:
			{
			KillTimer(nIDEvent);
			unsigned int newclock = clock();

			xarr.Clear(); varr.Clear();
			xint_arr.TurnArr(&xarr, &varr, true);
			//xarr.DumpAll();

			//if(newclock > lastclock + CLOCKS_PER_SEC / 3)
				{
				//lastclock = newclock;
				if(xchanged)
					{
					RECT rc; GetClientRect(&rc);

					int asize = xint_arr.GetSize();
					xchanged = false;
				
					//if(mag == 1)
						xmag = asize / (rc.right - rc.left) * 100;

					//P2N("ss=%d  rcwidth=%d mag=%f\r\n", asize, rc.right - rc.left, 1 + mag/100);
					xscroll = 0;
					Invalidate();

					xtimed = 0;
					}
				}
			}
			break;

		//////////////////////////////////////////////////////////////////
		
	case 2:
		{
		KillTimer(nIDEvent);
		ytimer = false;
	
		RECT rc; GetClientRect(&rc);
	
		rc.top = rc.bottom - 30;
		rc.right = rc.left + 300;

		InvalidateRect(&rc, true);
		}
		break;

		//////////////////////////////////////////////////////////////////

	case 3:
		{
		KillTimer(nIDEvent);
		
		if(!xwasdouble)
			{
			CXrayM *xxmenu = new CXrayM(this);
			
			xCreateMenu(xxmenu);
			xxmenu->sendto = this; //GetParent();; 
		
			POINT scr(downpoint);  ClientToScreen(&scr);
			xxmenu->Show(scr.x, scr.y);
			}
		}
		break;
	
	}

	CStatic::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////

void	CPlot::xAddCommas(CString &str) 

{
	CString tmp;

	//P2N("addcomas() in='%s'  ", str);

	int len = str.GetLength();

	// Reverse string, add commas
	for(int loop = 0; loop < len ; loop++)
		{
		if(loop % 3 == 0 && loop != 0 ) 
			tmp+= ',';
		
		tmp += str[len - 1 - loop];
		}

	// Get ready to reconstruct
	str = ""; len = tmp.GetLength();

	// Reverse back to original
	for(int loop2 = 0; loop2 < len ; loop2++)
		{
		str += tmp[len - 1 - loop2];
		}

	//P2N("out='%s'\r\n", str);
}

//////////////////////////////////////////////////////////////////////////

void CPlot::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 

{

	//P2N("CPlot::OnKeyDown -- %d\r\n", nChar);

	//switch(nChar)
	//	{
		//case VK_LEFT:
		//	P2N("CPlot::OnKeyDown -- Left\r\n");
		//	break;
	//	}

	CStatic::OnKeyDown(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////////////

BOOL CPlot::PreTranslateMessage(MSG* pMsg) 

{
	//P2N("CPlot::PreTranslateMessage %s\r\n", mxpad.num2msg(pMsg->message));

	int ch = 0;

	switch(pMsg->message)
		{
		case WM_COMMAND:
				{
				switch(pMsg->wParam)
					{
					case ID_PLOT_RCLICLK_COPY:
						//AP2N("PreTranslateMessage ID_PLOT_RCLICLK_COPY\r\n");
						xCopy();
						break;

					case ID_PLOT_RCLICLK_PASTE:
						//AP2N("PreTranslateMessage ID_PLOT_RCLICLK_PASTE\r\n");
						xPaste();
						break;

					case ID_PLOT_RCLICLK_PASTE2:
						//AP2N("PreTranslateMessage ID_PLOT_RCLICLK_PASTE2\r\n");
						xPaste(true);
						break;

					case ID_PLOT_RCLICLK_CLEAR:
						//AP2N("PreTranslateMessage ID_PLOT_RCLICLK_CLEAR\r\n");
						ClearAll();
						break;		
						
					case ID_PLOT_RCLICLK_AUTO:
						//AP2N("PreTranslateMessage ID_PLOT_RCLICLK_AUTO\r\n");
						xAuto();
						break;		

					case ID_PLOT_RCLICLK_SMOOTH:
						//AP2N("PreTranslateMessage ID_PLOT_RCLICLK_SMOOTH\r\n");
						xSmooth();
						break;		

					case ID_PLOT_RCLICLK_SMOOTH2:
						//AP2N("PreTranslateMessage ID_PLOT_RCLICLK_SMOOTH\r\n");
						xSmooth(4);
						break;		

					case ID_PLOT_RCLICLK_SMOOTH3:
						//AP2N("PreTranslateMessage ID_PLOT_RCLICLK_SMOOTH\r\n");
						xSmooth(12);
						break;		

					case ID_PLOT_RCLICLK_LABEL:
						{
						CGetText gt; 
						gt.m_title = "Enter Label for Plot"; gt.m_edit = xlabel;
						gt.DoModal();
						xlabel = gt.m_edit;
						Invalidate();
						break;
						}
					}
				}

		case WM_KEYDOWN:
			//P2N("PreTranslateMessage WM_KEYDOWN %p %d\r\n", this, pMsg->wParam);
			switch(pMsg->wParam)
				{
				case VK_LEFT:
					//P2N("CPlot::OnKeyDown -- Left\r\n");
					xscroll++; ch++;
					break;

				case VK_RIGHT:
					//P2N("CPlot::OnKeyDown -- Right\r\n");
					xscroll--; ch++;
					break;		

				case VK_HOME:
				case VK_ESCAPE:
					xscroll = 0; ch++;
					break;
				}

			if(ch)
				{
				xscroll = max(xscroll, 0);

				if(xscroll > xint_arr.GetSize() - 1)
					xscroll = xint_arr.GetSize() - 1;

				Invalidate();
				}
			return true;
			break;
		}
	
	return CStatic::PreTranslateMessage(pMsg);
}

void CPlot::OnLButtonDown(UINT nFlags, CPoint point) 

{
	SetFocus();	
	CStatic::OnLButtonDown(nFlags, point);
}

void CPlot::PreSubclassWindow() 

{

	ModifyStyle(0, WS_CLIPCHILDREN | SS_NOTIFY);
		
	CStatic::PreSubclassWindow();
}

double CPlot::GetMag()

{
	return xmag;
}

void CPlot::SetMag(double mm)

{
	xmag = mm;
	xchanged = false;
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

void CPlot::OnRButtonUp(UINT nFlags, CPoint point) 

{
	//AP2N("CPlot::OnRButtonUp this=%p\r\n", this);
	
#if 1
	if(ABS(downpoint.x - point.x) < 3)
		{
		//AP2N("CPlot::OnRButtonUp nomove\r\n", this);
		
		SetTimer(3, 100, NULL);
		}
#endif

	CStatic::OnRButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////


void CPlot::xCreateMenu(CXrayM *pmenu)

{
	// Construct sub menu
	pmenu->AppendMenuItem("Copy",						ID_PLOT_RCLICLK_COPY);
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem("Paste as New",               ID_PLOT_RCLICLK_PASTE);
	pmenu->AppendMenuItem("Paste Append",               ID_PLOT_RCLICLK_PASTE2);
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem("Clear",                      ID_PLOT_RCLICLK_CLEAR);

	//pmenu->AppendMenuItem( SEPARATOR );
	//pmenu->AppendMenuItem("Auto",                       ID_PLOT_RCLICLK_AUTO);
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem("Smooth",						ID_PLOT_RCLICLK_SMOOTH);
	pmenu->AppendMenuItem("Smooth Hard",				ID_PLOT_RCLICLK_SMOOTH2);
	pmenu->AppendMenuItem("Smooth Extreme",				ID_PLOT_RCLICLK_SMOOTH3);
	pmenu->AppendMenuItem("Label Plot",					ID_PLOT_RCLICLK_LABEL);

}

//////////////////////////////////////////////////////////////////////////

void CPlot::xCopy()

{
	VERIFY(OpenClipboard()); EmptyClipboard();
 
	int *iptr = xint_arr.GetData(); 
	int nsize = xint_arr.GetSize() * sizeof(unsigned long);

	//AP2N("CPlot::Copy() %d bytes\r\n", nsize);

	HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nsize);
	void *ptr = GlobalLock(hMem); ASSERT(hMem);
    memcpy(ptr, iptr, nsize);
    ::GlobalUnlock(hMem);

	HANDLE hh = SetClipboardData(xcformat, hMem);
	VERIFY(CloseClipboard());

}

//////////////////////////////////////////////////////////////////////////

void CPlot::xPaste(int noclear)

{
	VERIFY(OpenClipboard()); HANDLE hh = GetClipboardData(xcformat);

	if(!hh)
		return;

	int len = GlobalSize(hh) / sizeof(int);

	if(!len)
		return;

	int *ptr = (int*)GlobalLock(hh); ASSERT(hh);

	//AP2N("CPlot::Paste() ptr=%p  len=%d ints\r\n", ptr, len);

	if(!noclear)
		xint_arr.RemoveAll();	

	for(int loop = 0; loop < len; loop++)
		{
		xint_arr.Add(ptr[loop]);
		}

	GlobalUnlock(hh);
	
	VERIFY(CloseClipboard());

	if(!xchanged && !xtimed)
		{
		SetTimer(1, 200, NULL);	xtimed = true;
		}

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

void CPlot::xSmooth(int fact)

{
	int nlen = xint_arr.GetSize();

	for(int loop3 = 0; loop3 < fact; loop3++)
		{
		CIntArr ccc; ccc.SetSize(nlen);
		
		for(int loop = 1; loop < nlen - 1; loop++)
			{
			double val = 0;
			
			// Catch low boundary
			val += (double)xint_arr.GetAt(loop - 1);
			val += (double)xint_arr.GetAt(loop);
			val += (double)xint_arr.GetAt(loop + 1);
						
			ccc.SetAt(loop, (int)(val / 3));
			}
		xint_arr.Copy(ccc);
		}
	
	SetTimer(1, 200, NULL);	xtimed = true;

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

void CPlot::xAuto()

{
	int nlen = xint_arr.GetSize();

	RECT rc; GetClientRect(&rc);

	int ew = (rc.right - rc.left) - 2 * xhor_gap;

	AP2N("CPlot::xAuto() nlen=%d ew=%d xmag=%f\r\n", nlen, ew, xmag);

	xmag = -100 * ( (double)ew / nlen - 1);

	AP2N("CPlot::xAuto() xmag=%f\r\n", xmag);

	Invalidate();
}