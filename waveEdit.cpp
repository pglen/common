
/* =====[ waveEdit.cpp ]========================================== 
                                                                             
   Description:     The daco project, implementation of the waveEdit.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  11/16/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */


#include "stdafx.h"

#include <math.h>

#include "waveEdit.h"
#include "dsp.h"
#include "misc.h"
#include "support.h"
#include "mxpad.h"

#include "FileDialogST.h"

#pragma warning(disable : 4302)

#ifdef _DEBUG
#undef THIS_FILE
//static TCHAR THIS_FILE[] = _T(__FILE__);
static const char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define IDR_MENU1                       145

#ifndef IDC_HAND
#define IDC_HAND  MAKEINTRESOURCE(32649)
#endif
	
#define MAX_MRU 8

/////////////////////////////////////////////////////////////////////////////
// CwaveEdit

CwaveEdit::CwaveEdit()

{
	magnify = mag = 1;

	msgflag = true; 
	msgval = WM_USER + 2;

	old_offset = old_marker	= in_play = 0;
	old_startsel = old_endsel = startsel = endsel = 0;

	changed = fired = 0;
	
	currpos	=  marker = 0;

	markstart = markend = markmove = moveoffset = 0;

	//xxmenu = NULL;

	startlen = endlen = 0;
	
	//mouspressed = FALSE;
	
	pplaysnd = NULL;

	in_mark     = 0;
	startpos	= 0;
	soundbuff	= NULL;
	soundlen	= 0;

	first = 0;
	right = 0;			// set this for right channel
	skip =  1;
	ipwleft = wleft = 0;

	soundbuff2 = NULL; soundlen2 = NULL;

	cformat = RegisterClipboardFormat(_T("MonoWav44100"));

	selcolor = RGB( 32, 64, 32);
	bgcolor = RGB( 0, 0, 0);
	ipcolor = RGB(128, 128, 128);
	gridcol = RGB(0,0,30);

	undolim = 10;

	oldpath = ".";

	section = "wavehist";

	//in_append = false;
	
	//AP2N(_T("CwaveEdit::RegisterClipboardFormat %d\r\n"), cformat);

	//::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT),sizeof(m_lf),&m_lf);
	//m_font.CreateFontIndirect(&m_lf);
}

CwaveEdit::~CwaveEdit()

{
	//AP2N(_T("CwaveEdit::~CwaveEdit()\r\n"));

	if(soundbuff) free(soundbuff);

	for(int loop = 0; loop < undoptr.GetSize(); loop++)
		{
		free( (TCHAR *) undoptr.GetAt(loop));
		}

	//CXrayM::FreeLast();

	//m_font.DeleteObject();
	//AP2N(_T("CwaveEdit::~CwaveEdit()\r\n"));
}

//////////////////////////////////////////////////////////////////////////
// Copy this to resource file .rc and resource.h

#define IDR_WAVED_MENU                  139

#define ID_RCLICLK_PLAY                 32771
#define ID_RCLICLK_SELECTALL            32773
#define ID_RCLICLK_CROPSELECTED         32775
#define ID_RCLICLK_CUTSELECTED          32776
#define ID_RCLICLK_COPYSELECTED         32777
#define ID_RCLICLK_SAVE                 32778
#define ID_RCLICLK_ANALIZE              32779
#define ID_RCLICLK_SAVEASFILE           32780
#define ID_RCLICLK_OPENFILE             32781
#define ID_RCLICLK_FRAGEDIT             32782
#define ID_RCLICLK_RECOG                32783
#define ID_RCLICLK_ADJUSTTOZEROCROSSING 32784
#define ID_RCLICLK_PASTE                32785
#define ID_RCLICLK_UNDO                 32786
#define ID_RCLICLK_PROCESS_FADEIN       32787
#define ID_RCLICLK_PROCESS_FADEOUT      32788
#define ID_RCLICLK_PROCESS_NORMALIZE    32792
#define ID_RCLICLK_HELLO                32793
#define ID_RCLICLK_CLEARBUFFER          32816
#define	ID_RCLICLK_SAVEASFILE2			32817	
#define ID_RCLICLK_NEW_PASTE            32818


#define ID_RCLICLK_MRU					32900


BEGIN_MESSAGE_MAP(CwaveEdit, CStatic)
	//{{AFX_MSG_MAP(CwaveEdit)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()	
	ON_WM_SETCURSOR()
	ON_WM_MOUSEACTIVATE()
	ON_COMMAND(ID_RCLICLK_PLAY, OnRcliclkPlay)
	ON_COMMAND(ID_RCLICLK_SAVEASFILE2, OnRcliclkSaveFile2)
	ON_COMMAND(ID_RCLICLK_SAVEASFILE, OnRcliclkSaveFile)
	ON_COMMAND(ID_RCLICLK_OPENFILE, OnRcliclkOpenFile)	
	
	ON_COMMAND(ID_RCLICLK_SELECTALL, OnRcliclkSelectall)
	ON_COMMAND(ID_RCLICLK_CROPSELECTED, OnRcliclkCropselected)
	ON_COMMAND(ID_RCLICLK_SAVE, OnRcliclkSave)
	ON_COMMAND(ID_RCLICLK_CUTSELECTED, OnRcliclkCutselected)
	ON_COMMAND(ID_RCLICLK_ADJUSTTOZEROCROSSING, OnRclickZcross)
	ON_COMMAND(ID_RCLICLK_COPYSELECTED, OnRclickCopy)
	ON_COMMAND(ID_RCLICLK_PASTE, OnRclickPaste)
	ON_COMMAND(ID_RCLICLK_NEW_PASTE, OnRclickNewPaste)
	ON_COMMAND(ID_RCLICLK_UNDO, OnRclickUndo)
	ON_COMMAND(ID_RCLICLK_PROCESS_NORMALIZE, OnRclickNorm)
	ON_COMMAND(ID_RCLICLK_CLEARBUFFER, OnRclickDel)

	ON_COMMAND_RANGE(ID_RCLICLK_MRU, ID_RCLICLK_MRU + MAX_MRU, OnRclickMRU)

	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CwaveEdit message handlers

void CwaveEdit::OnPaint() 

{
	CPaintDC dc(this); 

	// Silently ignore if nothing to paint
	if(!soundbuff)
		return;

	if(!soundlen)
		return;
	
	//if(in_append)
	//	return;
	
	mx.Lock();

    HPEN  hPen2, hpenOld;            // green pen for drawing columns
    
	DWORD dwMaxStart;                // maximum value for starting point
    
	int   iBase;                     // vertical position of baseline
    int   iYScale;					// scaling factors
	double iXScale;
    int   iCol;                      // horizontal position of a column
    int   iColHeight;                // height of a column

	RECT  rClient;	GetClientRect( &rClient );
    
    iBase = RECTHEIGHT(rClient) / 2;
	
	//AP2N(_T("WaveEdit Paint data: startsel=%d endsel=%d\r\n"), startsel, endsel);
	//AP2N(_T("WaveEdit Paint rcwidth=%d soundlen=%d\r\n"), RECTHEIGHT(rClient), soundlen);
		
	short *data = (short*)soundbuff, *data2 = (short*)soundbuff;
	short *dend = (short*)(soundbuff + soundlen);

	/* Graph waveform data */
    hPen2 = CreatePen( PS_SOLID, 1, 0x00ff00 );     // color display
	hpenOld = (HPEN)dc.SelectObject(hPen2 );

    dc.MoveTo( 0, iBase);

    /* Current sample position in the waveform data */
    dwMaxStart = soundlen - RECTWIDTH(rClient) - 1;
    //lp = data; // + min( dwCurrentSample, dwMaxStart );

    /* Determine the WIDTH scaling factor */
	iXScale = (double)soundlen / (RECTWIDTH(rClient));
	iXScale /= sizeof(short);

	iXScale /= max(1, (int)mag);

	//AP2N(_T("iXScale=%.2f\r\n"), iXScale);
	
	// Catch it at minimum value
	//if(iXScale < 1)
	//	{
	//	iXScale = 1;
	//	}

    /* determine the height scaling factor */
    iYScale = ((MAXVALUE-MINVALUE) + 1) /		// amplitude
                  (RECTHEIGHT(rClient) - 4);    // control's height

	// Make sure we jump on 4 bytes boundary
	//int offset = (((startpos * soundlen) / 100) / iXScale) * iXScale;
	//int offset = (((startpos * soundlen) / 1000) / iXScale) * iXScale;
	//data += offset;

	// Ahead of beginning
	if(data < data2)
		data = data2;

	//AP2N(_T("Scales: x:%d y:%d Start: %d\r\n"),
	//					iXScale, iYScale, offset);

    // Subtracting 4 from the height ensures a small
    // margin above and below the biggest waves
    //data += dwMaxStart;

	int prog = 0;
	short *data3 = (short*)soundbuff;
	try
		{
		/* paint samples from the waveform data */
		for (iCol = (int)rClient.left; iCol <= (int)rClient.right; iCol++)
		   {
			data3 = data + (int)(prog * floor(iXScale));
			//data += skip * (iXScale / sizeof(short));
			
			// End of memory
			if(data3 >= dend - (int)iXScale)
				{
				AP2N(_T("Paint ended at buffer %x\r\n"), data3); 
				break;
				}

			short val = *data3;

			//for(int loop = 1; loop < iXScale / (int)sizeof(short); loop++)
			//	{
			//	//val += *(data + loop); val /= 2;
			//	}

			iColHeight =  val / iYScale;

			//AP2N(Paint dot %d -- %d\r\n", iCol, iColHeight);
			
			dc.LineTo(iCol, iBase - iColHeight );

			prog++;
			}
		} 

	catch(...)
		{
		xShowInfo(_T("Exception in Paint"), 300);
		AP2N(_T("Exception in wavedit paint data=%x data3=%x dend=%x iXScale=%f soundlen=%d\r\n"), 
			data, data3, dend, iXScale, soundlen);
		}

	//AP2N(_T("Wavedit paint data3=%x dend=%x iXScale=%f\r\n"), 
	//		data3, dend, iXScale);
	
	dc.SelectObject( hpenOld );

	mx.Unlock();

//endd:
    // Free resources
    DeleteObject( hPen2 );
}

//////////////////////////////////////////////////////////////////////////

BOOL CwaveEdit::OnEraseBkgnd(CDC* pDC) 

{
    HPEN  hPen2, hPen3 = NULL, hPen4 = NULL, hPen5 = NULL, hPenOld;
    RECT  rClient; GetClientRect( &rClient );
	
    int   iBase;			// vertical position of baseline

	iBase = RECTHEIGHT(rClient)/2;

	//AP2N(_T("WaveEdit Paint background x=%d y=%d\r\n"), 
	//		RECTWIDTH(rClient), RECTHEIGHT(rClient));
 		    
	pDC->FillSolidRect(&rClient, bgcolor);
	
	int startsel2 = min(startsel, endsel);
	int endsel2   = max(startsel, endsel);

	CRect rc(rClient);
	CLIP(startsel2, 0, rc.Width());	CLIP(endsel2, 0, rc.Width());

	// Paint selection rect
	CRect selrc(rClient);	selrc.left = startsel2; selrc.right = endsel2;
	pDC->FillSolidRect(selrc, selcolor);
		
	if(GetFocus() == this)
		gridcol = RGB(0,0,60);
	else
		gridcol = RGB(0,0,30);

	// Paint fancy grid
   	hPen3 = CreatePen( PS_SOLID, 1, gridcol);
    if (hPen3)
		{
		hPenOld	= (HPEN)pDC->SelectObject( hPen3 );
		// Vertical
		for (int bb=10; bb < RECTHEIGHT(rClient)/2; bb+= 10)
			{
			// Top
			pDC->MoveTo(0 , iBase + bb);
			pDC->LineTo((int)rClient.right, iBase + bb );
			if(!(bb % 50))
				{
				pDC->MoveTo(0 , iBase + bb+1);
				pDC->LineTo((int)rClient.right, iBase+ bb+1 );
				}
			// Buttom
			pDC->MoveTo(0 , iBase - bb);
			pDC->LineTo((int)rClient.right, iBase - bb );
			if(!(bb % 50))
				{
				pDC->MoveTo(0 , iBase - bb+1);
				pDC->LineTo((int)rClient.right, iBase - bb+1 );
				}
			}
		// Horizontal
		for (int aa = 0; aa < RECTWIDTH(rClient); aa += 15)
			{
			pDC->MoveTo(aa, 0);
			pDC->LineTo(aa, RECTHEIGHT(rClient));
			if(!(aa % 75))
				{
				pDC->MoveTo(aa+1, 0);
				pDC->LineTo(aa+1, RECTHEIGHT(rClient));
				}
			}
		}

    // Draw the waveform baseline 
	hPen2 = CreatePen( PS_SOLID, 1, 0x000050);     
    if (hPen2)
		{
		pDC->SelectObject(hPen2 );
        iBase = RECTHEIGHT(rClient) / 2;
        pDC->MoveTo( 0, iBase);
        pDC->LineTo((int)rClient.right, iBase);
		}

	// Paint insertion point
	hPen5 = CreatePen( PS_SOLID, 1, ipcolor);
	pDC->SelectObject(hPen5);
	pDC->MoveTo(wleft , 0);	pDC->LineTo(wleft, rc.Height());

	if(hPenOld)
		pDC->SelectObject(hPenOld);

    if (hPen2) DeleteObject(hPen2);
    if (hPen3) DeleteObject(hPen3);
    if (hPen4) DeleteObject(hPen4);
	if (hPen5) DeleteObject(hPen5);

	return FALSE;
}

//////////////////////////////////////////////////////////

void CwaveEdit::OnLButtonUp(UINT nFlags, CPoint point) 

{
	//AP2N(_T("Mouse in WaveEdit\r\n"));

	moveoffset = 0;
	markmove = markstart = markend = false;

	CStatic::OnLButtonUp(nFlags, point);
}

///////////////////////////////////////////////////////////////
//
// Draw marker.
// 
//	pos = byte offset in bitmap
//

void CwaveEdit::xMarker(int pos)

{
    HPEN	hPen2, hPenOld;  
    RECT	rClient;      
	int		offset, iXScale;
	int		in_mark = 0; 

	if(!soundbuff)
		return;

	if(!soundlen)
		return;

	if(in_mark)
		return;

	in_mark = TRUE;
	
	int len = soundlen; 

    GetClientRect( &rClient );
	iXScale = len / RECTWIDTH(rClient);

	// Make sure it is divisable
	if(iXScale < 1)
		return;

	offset = 2 * pos / iXScale;

	CLIP(offset, 0, RECTWIDTH(rClient));

	CDC		*dc = GetDC( ), memdc;

	if(!first)
		{
		bm2.CreateCompatibleBitmap(dc, 1 , RECTHEIGHT(rClient));
		}
	memdc.CreateCompatibleDC(dc); memdc.SelectObject( bm2 );

	// The first item, presave
	if(!first)
		{
		memdc.BitBlt(0, 0, 1, RECTHEIGHT(rClient), 
			dc, old_offset, 0, SRCCOPY );
		first++;
		}

	// Restore old contents
	dc->BitBlt(old_offset, 0, 1, RECTHEIGHT(rClient), &memdc, 0,0, SRCCOPY );

	// Save new
	memdc.BitBlt(0, 0, 1, RECTHEIGHT(rClient), 
							dc, offset, 0, SRCCOPY );
    // Draw marker
	hPen2 = CreatePen( PS_SOLID, 1, RGB(255, 20, 20) );     // color display
    if (hPen2)
		{
		hPenOld = (HPEN)dc->SelectObject( hPen2 );
		dc->MoveTo( offset, 0);
        dc->LineTo( offset, RECTHEIGHT(rClient));
		}
	old_offset = offset;

	// Reentry prevention
	in_mark = FALSE;

	// Free resources ...
	dc->SelectObject( hPenOld );

    if (hPen2) 
		DeleteObject(hPen2);

	ReleaseDC(dc); ReleaseDC(&memdc);
	memdc.DeleteDC();
}

		
////////////////////////////////////////////////////////////

void CwaveEdit::OnMouseMove(UINT nFlags, CPoint point) 

{
	if(nFlags & MK_RBUTTON)
		{
		if(misc.IsShiftDown())
			{
			AP2N(_T("Mouse MK_RBUTTON\r\n"));
			
			if(point.x > oldx)
				magnify *= 1.3;
			else if (magnify > 0.5)
				magnify /= 1.3;

			Magnify(magnify);
			oldx = point.x;
			}
		else
			{		
			}
		}

	if(nFlags & MK_LBUTTON)
		{
		//AP2N(_T("Mouse MK_LBUTTON\r\n"));
		if(misc.IsShiftDown())
			{
			if(point.x > oldrx)
				SetStartPos(currpos--);
			else
				SetStartPos(currpos++);

			//oldrx = point.x;
			}
		else
			{			
			// Obey selection flags
			if(markstart)
				startsel = point.x;
			else if(markend)
				endsel = point.x;
			else if(markmove)
				{
				int mlen = endsel - startsel;
				startsel = point.x + moveoffset;
				endsel = startsel + mlen;
				}
	
			//AP2N(_T("CwaveEdit::OnMouseMove  drag %d startsel=%d endsel=%d\r\n"), 
			//			point.x, startsel, endsel);

			xDelayedInval();				
			}
		}
	
	oldrx = oldx = point.x;

	xPostSelChange();

	CStatic::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::OnLButtonDown(UINT nFlags, CPoint point) 

{
	wleft = point.x;

	//SetFocus();

	if(startsel == endsel) 	// Are starting out new?
		{
		AP2N(_T("starting new\r\n"));

		startsel = endsel = point.x;
		markstart = markmove = false;
		markend = true; 
		
		Invalidate();
		xPostSelChange();
		}
	// Override inside sel as a priority
	else if(point.x > startsel && point.x < endsel) // Inside sel
		{
		//AP2N(_T("Between sel\r\n"));
		markstart = markend = false; markmove = true;
		moveoffset = startsel - point.x;
		}
	else if(xIsNear(point.x, startsel)) // Are we close to terminals?
		{
		//AP2N(_T("Close to start\r\n"));
		markstart = true;
		markend = markmove = false;
		}
	else if(xIsNear(point.x, endsel))
		{
		//AP2N(_T("Close to end\r\n"));
		markstart = markmove = false;
		markend = true;
		}
	else
		{
		// Erase selection, start new
		startsel = endsel = point.x; 
		markstart = markmove = false;
		markend = true;
		Invalidate();
		xPostSelChange();
		}	

	AP2N(_T("CwaveEdit::OnLButtonDown startsel=%d endsel=%d point.x=%d\r\n"), 
							startsel, endsel, point.x);

	CStatic::OnLButtonDown(nFlags, point);
}


//////////////////////////////////////////////////////////////////////////

void CwaveEdit::xMarkerOff()

{
    RECT	rClient;      
	CDC		*dc = GetDC( );
	CDC		memdc;

	//return;

    GetClientRect( &rClient );

	memdc.CreateCompatibleDC(dc);		
	memdc.SelectObject( bm2 );

	// Restore old contents
	dc->BitBlt(old_offset, 0, 1, 
			RECTHEIGHT(rClient), &memdc, 0,0, SRCCOPY );

	// Reset old position
	old_offset = 0;

	// Presave for new
	memdc.BitBlt(0, 0, 1, 
			RECTHEIGHT(rClient), dc, 0, 0, SRCCOPY );

	//first = 0;
	// Free reources
	ReleaseDC(dc); ReleaseDC(&memdc);
	memdc.DeleteDC();
}

////////////////////////////////////////////////////////////

void CwaveEdit::OnTimer(UINT nIDEvent) 

{
	//AP2N(_T("CwaveEdit::OnTimer nIDEvent = %d\r\n"), nIDEvent);

	if(nIDEvent == 0)
		{
		KillTimer(nIDEvent);

		//AP2N(_T("Message erased\r\n"), lastinfo);
		lastinfo = "";

		if(IsWindow(m_hWnd))
			Invalidate();
		}

	if(nIDEvent == 1)
		{
		//CRect rc; GetClientRect(rc);

		int curr = pplaysnd->GetCurrPos() / 2 + playoffset;
		
		xMarker(curr / 2);  // Adjust for stereo
		
		//AP2N(_T("Timer for progress start=%d curr=%d slen=%d last=%d playing=%d\r\n"), 
		//	startlen, curr, soundlen, lastpos, pplaysnd->IsPlaying());
		
		// Done?
		int stopped = curr == lastpos;
		if(curr >= soundlen || stopped)
			{
			KillTimer(1); xMarkerOff();
			in_play = false;
			}
		if(postto)
			{	
			//P2N(_T(" CwaveEdit::OnTimer Posting %d %d\r\n"), in_play, curr);
			::PostMessage(postto, WM_USER + 4, in_play,	curr);
			}					
		lastpos = curr;	
		}
	
	if(nIDEvent == 2)
		{
		KillTimer(nIDEvent);

		//AP2N(_T("Delayed Invalidate\r\n"));
		
		if(IsWindow(m_hWnd))
			Invalidate();
		}

	if(nIDEvent == 3)
		{
		KillTimer(nIDEvent);

		//AP2N(_T("Message: '%s'\r\n"), lastinfo);
		
		CClientDC  dc(this);
		RECT  rClient;	GetClientRect( &rClient );
    
		dc.SetBkMode(TRANSPARENT);	
		dc.SetTextColor(RGB(255, 255, 255));
		//dc.SelectObject(&m_font);
			
		dc.TextOut(4, 4, lastinfo); 
		SetTimer(0, lasttimeout, NULL);
		
		//dc.SetTextColor(RGB(155, 155, 155));

		CSize ss = dc.GetTextExtent(wavefname);
		dc.TextOut(rClient.right - rClient.left - ss.cx - 8, 4, wavefname); 
		}

	if(nIDEvent == 4)
		{
		KillTimer(nIDEvent);

		OpenWave(commstr);
		}
	
	CStatic::OnTimer(nIDEvent);
}

void CwaveEdit::Magnify(double factor)

{
   mag = factor;
   RedrawWindow();
}

///////////////////////////////////////////////////////////////
// startposition expresses in mil

void CwaveEdit::SetStartPos(int pos)

{
	//AP2N(_T("Set startpos %d\r\n"), pos);
	startpos = (pos * 4 ) / 4;

    RedrawWindow();
}

///////////////////////////////////////////////////////////////
//

void CwaveEdit::OnClose() 

{
	//AP2N(_T("Close WaveEdit\r\n"));
	
	//playsound.StopPlay();
	
	CStatic::OnClose();
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::OnLButtonDblClk(UINT nFlags, CPoint point) 

{
	AP2N(_T("CwaveEdit::OnLButtonDblClk nFlags=%d point.x=%d point.y=%d\r\n"),
		nFlags, point.x, point.y);

	currpos = 0; SetStartPos(currpos);
	mag = magnify = 1;	
	wleft = startsel = endsel = 0;
	
	xFrameSel(); Invalidate();
	
	CStatic::OnLButtonDblClk(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::xMySysKeyDown(UINT wParam, UINT lParam)

{
	if(wParam == VK_F10)
		{
		// Show menu in the middle
		CXrayM *xxmenu = new CXrayM;
		
		//if(!xxmenu)
		//	xxmenu = new CXrayM();

		xxmenu->sendto = this;
		//xxmenu->LoadMenu(IDR_WAVED_MENU, 1);
		
		xCreateMenu(xxmenu);
	
		CRect rc; GetClientRect(&rc);

		POINT scr; 
		scr.x = rc.left + rc.Width() / 2;
		scr.y = rc.top + rc.Height() / 2;  
		
		ClientToScreen(&scr);
		xxmenu->Show(scr.x, scr.y);
		}
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::xMyKeyDown(UINT wParam, UINT lParam)

{
	int processed = false;

	CRect rc; GetClientRect(rc);

	//AP2N(_T("CwaveEdit::OnKeyDown %d %c\r\n"), wParam, wParam);

	if(misc.IsCtrlDown())
		{
		// Move Start
		switch(wParam)
			{
			case VK_LEFT:
				if(startsel)
					{
					startsel--; Invalidate();
					}
				break;

			case VK_RIGHT:
				if(endsel)
					{
					startsel++; Invalidate();
					}
				break;
			}
		}
	else if(misc.IsShiftDown())
		{
		// Move end
		switch(wParam)
			{
			case VK_LEFT:
				if(endsel)
					{
					endsel--; Invalidate();
					}
				break;

			case VK_RIGHT:
				if(endsel)
					{
					endsel++; Invalidate();
					}
				break;
			}
		}
	else
		{
		// Move Selection
		switch(wParam)
			{
			case VK_LEFT:
				if(startsel)
					{
					startsel--; endsel--; Invalidate();
					}
				break;

			case VK_RIGHT:
				if(endsel)
					{
					startsel++; endsel++; Invalidate();
					}
				break;
			}

		if(wParam == VK_ESCAPE)
			{
			wleft = startsel = endsel = 0;
			Invalidate();
			}
		
		if(wParam == VK_UP)
			{
			int dist = endsel - startsel;
			startsel = 0; endsel = dist;
			Invalidate();
			}
		if(wParam == VK_DOWN)
			{
			int dist = endsel - startsel;
			endsel = rc.Width(); 
			startsel = endsel - dist; Invalidate();
			}
		if(wParam == VK_HOME)
			{
			int dist = endsel - startsel;

			wleft = startsel = 0; 
			//endsel = dist;
			Invalidate();
			}
		if(wParam == VK_END)
			{
			endsel = rc.Width(); 
			//int dist = endsel - startsel;
			//startsel = endsel - dist; 
			Invalidate();
			}
		if(wParam == VK_NEXT)
			{
			startsel-= 10; endsel-= 10; Invalidate();
			}
		if(wParam == VK_PRIOR)
			{
			startsel+= 10; endsel+= 10; Invalidate();
			}

		if(wParam == VK_DELETE)
			{
			Del();
			}

		// Defunct in favour of F10
		//if(wParam == VK_F5)
		//	{
		//	AP2N(_T("F5\r\n"));
		//	//VK_F10 f10
		//	CXrayM *xxmenu = new CXrayM;
		//	xxmenu->sendto = this;
		//	xxmenu->LoadMenu(IDR_WAVED_MENU, 1);
			
		//	CRect rc; GetClientRect(&rc);
		//	POINT scr; 
		//	scr.x = rc.left + rc.Width() / 2;
		//	scr.y = rc.top + rc.Height() / 2;  
			
		//	ClientToScreen(&scr);
		//	xxmenu->Show(scr.x, scr.y);
		//	}
		
		if(wParam >= '0' && wParam <= '9')
			{
			endsel = startsel + 20 * (wParam - '0');
			xDelayedInval();
			}
		}

	if(misc.IsCtrlDown())
		switch(wParam)
			{
			case 'A':	// Ctrl-a
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-A (sel)\r\n"));
				SelAll();
				processed = true;
			break;

			case 'C':	// Ctrl-c
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-C (copy)\r\n"));
				processed = true;
				Copy();
				//startsel = endsel = 0;
				//Invalidate();
			break;

			case 'G':	// Ctrl-g
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-G (zero)\r\n"));
				processed = true;
				OnRclickZcross();
				//startsel = endsel = 0;
				//Invalidate();
			break;

			case 'Q':	// Ctrl-q
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-G (zero)\r\n"));
				processed = true;
				Save();
				//startsel = endsel = 0;
				//Invalidate();
			break;

			case 'V':	// Ctrl-v
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-V (paste)\r\n"));
				processed = true;
				OnRclickPaste();				
			break;

			case 'N':	// Ctrl-n
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-N (new paste)\r\n"));
				processed = true;
				OnRclickNewPaste();				
			break;

			case 'O':	// Ctrl-o
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-O (open)\r\n"));
				processed = true;
				OpenWave(_T(""));
			break;
			
			case 'R':	// Ctrl-r
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-R (crop) \r\n"));
				processed = true;
				Crop();
			break;
			
			case 'S':	// Ctrl-s
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-S (save)\r\n"));
				processed = true;
				SaveWave(wavename);
			break;
						
			case 'X':	// Ctrl-x
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-X (cut)\r\n"));
				Copy();
				Cut();
				processed = true;
			break;

			case 'Z':	// Ctrl-z
				//AP2N(_T("CwaveEdit::OnKeyDown CTRL-Z (undo)\r\n"));
				Undo();
				processed = true;
			break;		
			}

		else if(misc.IsShiftDown())
			{
			switch(wParam)
				{
				// Play selected
				case ' ':
				//AP2N(_T("CwaveEdit::OnKeyDown Shift SPACE (sel play) \r\n"));
				PlayBuff();		
				processed = true;
				break;
				}
				
			if(wParam >= '0' && wParam <= '9')
				{
				// Calc buff to pixel width
				CRect rc; GetClientRect(rc);

				int desired =  (int)ldexp(256.f, wParam - '0');				

				//AP2N(_T("desired %d\r\n"), desired);

				endsel = startsel + desired * rc.Width() / soundlen;
				
				//CLIP(startsel, 0, rc.Width());	CLIP(endsel, 0, rc.Width());

				xDelayedInval();
				}
			}
		else
			{
			switch(wParam)
				{
				case ' ':
					//AP2N(_T("CwaveEdit::OnKeyDown SPACE (play) \r\n"));
					PlayBuff();				
					processed = true;
				break;

				case 'Z':
					//AP2N(_T("CwaveEdit::OnKeyDown Letter Z (zero cross) \r\n"));
					ZeroCross();					
				break;				
				}
			}

	xPostSelChange();
}

void CwaveEdit::OnKillFocus(CWnd* pNewWnd) 

{
	CStatic::OnKillFocus(pNewWnd);
	Invalidate();	
}

void CwaveEdit::OnSetFocus(CWnd* pOldWnd) 

{
	CStatic::OnSetFocus(pOldWnd);	
	Invalidate();	
}

void CwaveEdit::OnRButtonDown(UINT nFlags, CPoint point) 

{
	//AP2N(_T("CwaveEdit::OnRButtonDown\r\n"));

	CStatic::OnRButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::OnRButtonUp(UINT nFlags, CPoint point) 

{
	//AP2N(_T("CwaveEdit::OnRButtonUp this=%p\r\n"), this);
	
	// CMenu
	CXrayM *xxmenu = new CXrayM(this);
	
	xCreateMenu(xxmenu);
	xxmenu->sendto = this; //GetParent();; 
	
	// Remove save buffer
	if(!soundbuff2)
		{	
		int item = xxmenu->FindMenuItem(_T("Save Buffer"));
		xxmenu->DelMenuItem(item);
		xxmenu->DelMenuItem(item);   // delete separator after it
		}

	POINT scr(point);  ClientToScreen(&scr);
	xxmenu->Show(scr.x, scr.y);

	CStatic::OnRButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

int CwaveEdit::xIsNear(int x1, int x2, int dist)

{
	return abs(x1 - x2) < dist;
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::WaitForEnd()

{
	if(!pplaysnd->IsOpened())
		{
		//AP2N(_T("Player needs open.\r\n"));		
		xShowInfo(_T("Sound player not open"), 600); 		
		return ;
		}

	pplaysnd->WaitForEnd();
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::PlayBuff()

{
	if(!xIsOKToGo() || !xIsPlaySnd())
		return;

	if(!pplaysnd->IsOpened())
		{
		//AP2N(_T("Player needs open.\r\n"));		
		xShowInfo(_T("Sound player not open"), 600); 		
		return ;
		}

	// Stop play if it is going
	if(in_play)
		{
		pplaysnd->StopPlay();
		return;
		}

	in_play = true;

	xFrameSel();	
	ZeroCross();

	char *buff2 = soundbuff; int len2 = soundlen;

	// None selected, play whole buffer
	if(startsel == 0 && endsel == 0)
		{
		buff2 += ipwleft;
		len2 -= ipwleft;
		playoffset = ipwleft;
		}
	else
		{
		// Partial buffer
		buff2 += startlen;
		len2 = nsize;
		playoffset = startlen;
		}

	//AP2N(_T("CwaveEdit::PlayBuff (%p %d) %p len2=%d ipwleft=%d\r\n"), 
	//				soundbuff,	soundlen, buff2, len2, ipwleft);

	lastpos = -1;
	pplaysnd->WaveReset();
	SetTimer(1, 30, NULL);

	pplaysnd->PlayMonoWave(buff2, len2);		

	//pplaysnd->Play(buff2, len2);		
}

BOOL CwaveEdit::PreTranslateMessage(MSG* pMsg) 

{
	if(pMsg->message == WM_SETCURSOR)
		{
		//AP2N(_T("CwaveEdit::PreTranslateMessage setcur\r\n"));
		return true;  // we took care of it ...
		}

	if(pMsg->message == WM_KEYDOWN)
		{
		// Interpret standard dialog kewstrokes
		// Removed esc for easier editing

		//|| pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)

		if(pMsg->wParam == VK_TAB)			
			return CStatic::PreTranslateMessage(pMsg);

		//AP2N(_T("CArrPick::PreTranslateMessage(MSG* pMsg) %d\r\n"), pMsg->wParam);
	
		xMyKeyDown(pMsg->wParam, pMsg->lParam);

		return true;
		}

	if(pMsg->message == WM_SYSKEYDOWN)
		{
		// Just the F10 menu key
		if(pMsg->wParam != VK_F10) 
			return CStatic::PreTranslateMessage(pMsg);

			//AP2N(_T("CArrPick::PreTranslateMessage(MSG* pMsg) %d\r\n"), pMsg->wParam);
		
		xMySysKeyDown(pMsg->wParam, pMsg->lParam);
		return true;
		}


	return CStatic::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////

BOOL CwaveEdit::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 

{
	POINT point;	GetCursorPos(&point); ScreenToClient(&point);

	if(point.x > startsel && point.x < endsel) // Inside sel
		{
		//AP2N(_T("CwaveEdit::OnSetCursor Between sel\r\n"));
		::SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
		//::SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
		return true;
		}
	else if(xIsNear(point.x, startsel)) // Are we close to terminals?
		{
		//AP2N(_T("CwaveEdit::OnSetCursor Close to start\r\n"));
		::SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
		return true;
		}
	else if(xIsNear(point.x, endsel))
		{
		//AP2N(_T("CwaveEdit::OnSetCursor Close to end\r\n"));
		::SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
		return true;
		}
		
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

void CwaveEdit::OnRcliclkPlay() 

{
	PlayBuff();		
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::OnRcliclkSaveFile()

{
	//AP2N(_T("OnRcliclkSaveFile\r\n"));
	SaveWave(wavename);
}

void CwaveEdit::OnRcliclkSaveFile2()

{
	//AP2N(_T("OnRcliclkSaveFile\r\n"));
	SaveWave(_T(""));
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::OnRcliclkSelectall() 

{
	SelAll();	
}	


//////////////////////////////////////////////////////////////////////////

void CwaveEdit::OnRcliclkOpenFile() 

{
	//P2N(_T("CwaveEdit::OnRcliclkOpenFile()\r\n"));

	OpenWave(_T(""));
}	

void CwaveEdit::SelAll()

{
	CRect rc; GetClientRect(rc);
				
	if(startsel == endsel)
		startsel = 0, endsel = rc.Width();
	else
		startsel = endsel = 0;

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::Crop()

{
	if(!soundbuff)
		return;

	CRect rc; GetClientRect(rc);
	//AP2N(_T("Crop %d %d\r\n"), startsel, endsel);

	mx.Lock();

	xConvCoord();
	int	nsize = endlen - startlen;

	//AP2N(_T("Crop %d %d nsize=%d\r\n"), startlen, endlen, nsize);

	char *start = (char *)soundbuff;
	void *tmp   = malloc(nsize);

	// Swap buffers
	memcpy(tmp, start + startlen, nsize);
	//free(soundbuff);

	AddUndo();
		
	// Shuffle it
	soundbuff = (char*)tmp; 
	soundlen = nsize;
	startsel = endsel = 0;

	mx.Unlock();

	changed++;

	Invalidate();
	
}

void CwaveEdit::OnRcliclkCropselected() 

{
	Crop();	
}

void CwaveEdit::OnRcliclkSave() 
{
	Save();
}


//////////////////////////////////////////////////////////////////////////

int		CwaveEdit::SaveWave(const TCHAR *fname)
	
{
	if(!xIsOKToGo() || !xIsPlaySnd())
		return 0;

	CString strx(fname);

	if(strx == "")
		{
		CFileDialogST  cdf(false, NULL, NULL, 
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
		
		//CFileDialog  cdf(TRUE);

		TCHAR filter[] =
			_T("Wave Files (*.wav)\0*.wav\0All Files\0*.*\0\0") ;

		cdf.m_ofn.lpstrInitialDir = _T(".");		
		cdf.m_ofn.lpstrFilter	= filter;
		cdf.m_ofn.nFilterIndex	= 1;
		cdf.m_ofn.Flags |=  OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		if(cdf.DoModal() != IDOK)
			 return 0;
		
		strx = cdf.GetPathName();
		if(cdf.GetFileExt() == _T(""))
			{
			strx += _T(".wav");
			}
		}

	//AP2N(_T("Save Wave '%s'\r\n"), strx );
	
	xFrameSel(); ZeroCross();

	wavename = strx;
	int ret = pplaysnd->SaveWave(strx, soundbuff + startlen, nsize);

	if(!ret)
		{
		AP2N(_T("Could not save wave\r\n"));
		xShowInfo(_T("Could not save wave"), 1200);
		}

	CString tmp; tmp.Format(_T("Saved Wave to '%s'"), strx);
	xShowInfo(tmp, 2000);

	return ret;
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::Save()

{
	//AP2N(_T("CwaveEdit::Save \r\n"));	

	if(!xIsOKToGo())
		return;

	if(!soundbuff2 || !soundlen2)
		{
		xShowInfo(_T("Read Only Sound Buffer, Cannot Save"), 1200);
		return;
		}

	char *buff2 = (char*)malloc(soundlen); //ASSERT(buff2);
	memcpy(buff2, soundbuff, soundlen);
		
	free(*soundbuff2);

	*soundbuff2 = buff2;
	*soundlen2 = soundlen;

	xShowInfo(_T("Saved "), 600);
}

void CwaveEdit::ClearBuff()

{
	if(soundbuff) free(soundbuff);
	soundbuff = NULL; soundlen = 0;
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

char *	CwaveEdit::GetBuff()

{
	return (soundbuff);
}

//////////////////////////////////////////////////////////////////////////

int		CwaveEdit::GetBuffLen()

{
	return (soundlen);
}

//////////////////////////////////////////////////////////////////////////

int		CwaveEdit::GetStartLen()

{
	xFrameSel();
	return ipwleft;
}
 
const TCHAR * CwaveEdit::GetFileName()

{
	return wavefname;
}

const TCHAR * CwaveEdit::GetFullName()

{
	return wavename;
}

//////////////////////////////////////////////////////////////////////////
// Allocate new buffer for editing/showing

void CwaveEdit::SetBuff(void *buff, int len)

{
	AddUndo();
	
	soundbuff = (char*)malloc(len); //ASSERT(soundbuff);
	memcpy(soundbuff, buff, len);

	soundlen  = len;

	wleft = startlen = currpos = startpos = startsel = endsel = 0;
	xFrameSel();

	wavename = ""; wavefname = "";

	if(IsWindow(m_hWnd))
		xDelayedInval();
}

//////////////////////////////////////////////////////////////////////////
// Anticipate more data to come in

void CwaveEdit::AppendBuff(char *buff, int len, int undo)

{
	if(!soundbuff)
		{
		SetBuff(buff, len);
		return;
		}

//	in_append = true;

	mx.Lock();

	char *tmp = (char*)malloc(len + soundlen); //ASSERT(tmp);

	memcpy(tmp, soundbuff, soundlen);
	memcpy(tmp + soundlen, buff, len);

	soundlen += len;
	startsel = endsel = 0;

	// For real time rec ... too much data
	if(undo)
		AddUndo();	
	else
		free(soundbuff);

	soundbuff = tmp;
	changed++;

	//in_append = false;
	mx.Unlock();

	xDelayedInval();
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::Cut()

{
	if(!soundbuff)
		return;

	mx.Lock();

	AddUndo();

	xConvCoord();

	int	nsize = soundlen - (endlen - startlen);

	//AP2N(_T("Cut %d %d nsize=%d\r\n"), startlen, endlen, nsize);

	char *start = (char *)soundbuff;
	char *tmp   = (char *)malloc(nsize);

	// Assemble new buffer
	memcpy(tmp, soundbuff, startlen);
	memcpy(tmp + startlen, soundbuff + endlen, soundlen - endlen);
	
	// Shuffle it
	soundbuff = tmp; 
	soundlen -= (endlen - startlen);
	startsel = endsel = 0;

	changed++;

	mx.Unlock();

	Invalidate();
}

void CwaveEdit::xConvCoord()

{
	CRect rc; GetClientRect(rc);

	int startsel2 = min(startsel, endsel);
	int endsel2   = max(startsel, endsel);

	CLIP(startsel2, 0, rc.Width());
	CLIP(endsel2, 0, rc.Width());

	startlen = startsel2 * soundlen / rc.Width();
	endlen  = endsel2 * soundlen / rc.Width();

	startlen = startlen  + startlen % sizeof(short);
	endlen = endlen  + endlen % sizeof(short);
}

void CwaveEdit::OnRclickCopy() 

{
	Copy();
}

void CwaveEdit::OnRclickPaste()

{
	Paste();
}

void CwaveEdit::OnRclickNewPaste()

{
	ClearBuff();
	Paste();
}

void CwaveEdit::OnRclickUndo()

{
	Undo();
}

void CwaveEdit::OnRclickNorm()

{
	//AP2N(_T("Normalize\r\n"));
	Norm();
}

void CwaveEdit::OnRclickDel()

{
	Del();
}

void CwaveEdit::OnRclickMRU(UINT cmd)

{
	int mi = lastmenu2->FindMenuItem(cmd);
	lastmenu2->GetMenuItemText(mi, commstr);

	//AP2N(_T("CwaveEdit::OnRclickMRU %d %s\r\n"), cmd, str);

	SetTimer(4, 100, NULL);
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::OnRclickZcross() 

{
	//AP2N(_T("OnRclickZcross\r\n"));

	if(!xIsOKToGo() || !xIsPlaySnd())
		return;

	if(startsel == endsel)
		return;

	xFrameSel();	ZeroCross();

	CRect rc; GetClientRect(rc);

	// Get it back from buffer dimentions
	startsel  =  startlen *  rc.Width() / soundlen  ;
	endsel    =  endlen * rc.Width()   / soundlen  ;
	wleft	  =  ipwleft *  rc.Width() / soundlen  ;			
	
	xPostSelChange();
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

//DEL void CwaveEdit::OnRclickRecog() 
//DEL 
//DEL {
//DEL 	//AP2N(_T("OnRclickRecog\r\n"));
//DEL 
//DEL 	if(!IsOKToGo() || !IsPlaySnd())
//DEL 		return;
//DEL 
//DEL 	FrameSel();
//DEL 
//DEL 	TCHAR *buff = soundbuff;
//DEL 	int len = soundlen;
//DEL 	int part = RECLEN / 2;
//DEL 
//DEL 	for(int loop = 0; loop < len; loop += part)
//DEL 		{
//DEL 		TCHAR *buff3 = (char*)malloc(part); //ASSERT(buff3);
//DEL 	
//DEL 		int len2 = min(len - loop, part);
//DEL 		if(len2 < part)
//DEL 			memset(buff3, 0, part);
//DEL 
//DEL 		memcpy(buff3, buff + loop, len2);
//DEL 		recog.Recognize(buff3, part);
//DEL 		}
//DEL }

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::OnRcliclkCutselected() 

{
	Cut();	
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::xDelayedInval()

{
	DWORD  tick = GetTickCount();

	// Timer has been retriggerd for too long, force it
	if(tick - fired > 20)
		{
		if(IsWindow(m_hWnd))
			Invalidate();
		}
	else
		{
		SetTimer(2, 10, NULL);				
		}
	fired = tick;
}


void CwaveEdit::PreSubclassWindow() 

{
	//AP2N(_T("CwaveEdit::PreSubclassWindow() \r\n"));

	ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, 0);
	ModifyStyle(0, WS_CLIPCHILDREN | SS_NOTIFY);
		
	CStatic::PreSubclassWindow();
}

void CwaveEdit::SetSel(int beg, int end)

{
	startsel = beg; endsel = end;

	if(IsWindow(m_hWnd))
		{
		xDelayedInval();
		}
}

//////////////////////////////////////////////////////////////////////////
// Post message about change

void CwaveEdit::xPostSelChange()

{
	if(old_startsel != startsel || old_endsel != endsel)
		{
		if(msgflag)
			PostMessage(msgval, (int)m_hWnd, MAKELONG(startsel, endsel));

#if 1
		if(xIsOKToGo())
			{
			xFrameSel();
			CString str; str.Format(
				_T("Selection start=%d end=%d len=%d"), 
							startlen, endlen, endlen - startlen);
			xShowInfo(str, 2000);
			}
		old_startsel = startsel; old_endsel = endsel;
#endif

		}
}

//////////////////////////////////////////////////////////////////////////

int CwaveEdit::xIsOKToGo()

{
	if(!soundbuff)
		{
		//AP2N(_T("No sound buffer loaded.\r\n"));
		xShowInfo(_T("No Sound Buffer loaded."), 600); 
		return 0;
		}
	return true;
}

//////////////////////////////////////////////////////////////////////////

int CwaveEdit::xIsPlaySnd()

{
	if(!pplaysnd)
		{
		//AP2N(_T("No player started.\r\n"));		
		xShowInfo(_T("No Sound Player Present"), 600); 		
		return 0;
		}

	return true;
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::ZeroCross()

{
	if(!xIsOKToGo() || !xIsPlaySnd())
		return;

	if(startsel == endsel)
		return;

	//AP2N(_T("CwaveEdit::ZeroCross() startlen=%d endlen=%d nsize=%d\r\n"), 
	//								startlen, endlen, nsize);

	CRect rc; GetClientRect(rc);

	char  *wstart = soundbuff + startlen;
	char  *wend   = soundbuff + endlen;
	char  *elim   = soundbuff + soundlen - sizeof(short);
	char  *blim   = soundbuff + sizeof(short);

	short *curr;

	//AP2N(_T("org %p start %p\r\n"), wstart, soundbuff);

	//////////////////////////////////////////////////////////////////////
	// Do the beginning
	while(true)
		{
		// Frame boundaries
		if(wstart <= blim)
			break;
		
		curr = (short*)wstart;

		// Test for zcross
		//if( *curr < 0 && *(curr - 1) >= 0)
		//	break;

		if( *curr > 0 && *(curr - 1) <= 0)
			break;

		wstart -= sizeof(short);
		}

	//AP2N(_T("walking %p %d %d\r\n"), curr, *curr, *(curr-1));

	startlen = wstart - soundbuff;
	//startsel = (startlen * rc.Width()) / soundlen;

	//AP2N(_T("org2 %p end %p\r\n"), wend, wlim);
	//AP2N(_T("*curr=%d *(curr-1)=%d\r\n"), *curr, *(curr-1));

	//////////////////////////////////////////////////////////////////////
	// Do the end
 	while(true)
		{
		// Frame boundaries
		if(wend >= elim)
			break;
		
		curr = (short*)wend;
		
		// Test for zcross
		if( *curr < 0 && *(curr + 1) >= 0)
			break;

		//if( *curr > 0 && *(curr + 1) <= 0)
		//	break;

		wend += sizeof(short);
		}

	//AP2N(_T("walking2 %p %d %d\r\n"), curr, *curr, *(curr+1));
	//
	//AP2N(_T("*curr=%d *(curr+1)=%d\r\n"), *curr, *(curr+1));

	endlen = wend - soundbuff;
	//endsel = (endlen * rc.Width()) / soundlen;

	nsize = endlen - startlen;

	//AP2N(_T("CwaveEdit::ZeroCross()2 startlen=%d endlen=%d nsize=%d\r\n"), 
	//								startlen, endlen, nsize);

	//DelayedInval();
}

//////////////////////////////////////////////////////////////////////////

void	CwaveEdit::xFrameSel()

{
	if(!IsWindow(m_hWnd))
		return;

	CRect rc; GetClientRect(rc);

	int startsel2 = min(startsel, endsel); 
	int endsel2   = max(startsel, endsel);

	CLIP(startsel2, 0, rc.Width());	CLIP(endsel2, 0, rc.Width());

	startlen = startsel2 * soundlen / rc.Width();
	endlen  = endsel2 * soundlen / rc.Width();
	ipwleft = wleft * soundlen / rc.Width();

	//AP2N(_T("CwaveEdit::FrameSel() startlen=%d endlen=%d nsize=%d\r\n"), 
	//					startlen, endlen, nsize);

	// Make sure they are on word boundaries
	startlen = startlen  + startlen % sizeof(short);
	endlen = endlen  + endlen % sizeof(short);
	ipwleft = ipwleft  + ipwleft % sizeof(short);

	nsize = endlen - startlen;

	if(nsize == 0)
		{
		startlen = 0; nsize = soundlen;
		}

	//AP2N(_T("CwaveEdit::FrameSel() startlen=%d endlen=%d nsize=%d\r\n"), 
	//					startlen, endlen, nsize);
}

//////////////////////////////////////////////////////////////////////////
// Pass empty string for dialog
// 

int	CwaveEdit::OpenWave(const TCHAR *fname, int mruflag)

{
	if(!xIsPlaySnd())
		return 0;

	CString strx(fname);
	
	//AP2N(_T("OpenWave '%s'\r\n"), strx);

	if(strx == "")
		{		
		CFileDialogST  cdf(true, NULL, NULL, 
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
		
		//CFileDialogST  cdf(TRUE);

		//AP2N(_T("OpenWave2 '%s'\r\n"), strx);

		TCHAR filter[] =
			_T("Wave Files (*.wav)\0*.wav\0All Files\0*.*\0\0") ;

		cdf.m_ofn.lpstrInitialDir = oldpath;
		cdf.m_ofn.lpstrFilter	= filter;
		cdf.m_ofn.nFilterIndex	= 1;
		cdf.m_ofn.Flags |=  OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		//AP2N(_T("OpenWave2 %s\r\n"), strx);

		if(cdf.DoModal() != IDOK)
			 return 0;
		
		strx = cdf.GetPathName();

		//wavefname = cdf.GetFileName ();
		oldpath = cdf.GetFileDir();
		}
	
	char *buff; int len;
	int ret = pplaysnd->LoadWave(strx, &buff, &len);
	
	if(!ret)
		{
		xShowInfo(_T("Cannot Open Wave File"), 1000); 
		return 0;
		}

	//////////////////////////////////////////////////////////////////////
	// Manage MRU

	if(mruflag)
		{
		CString xxx; int cnt = 0, add = true;
		while(true)
			{
			xxx.Format(_T("File%d"), cnt);
			CString tmpstr = AfxGetApp()->GetProfileString(section, xxx);

			//AP2N(_T("GetProfileString %s\r\n"), tmpstr);

			if(tmpstr == "")
				break;

			if(tmpstr == strx)
				{
				add = false;
				break;
				}
			cnt++;
			}

		if(add)
			{
			int cnt_save = cnt;
			// Too many, shift it down
			if(cnt >= MAX_MRU)
				{
				cnt = MAX_MRU - 1;
				int cnt2 = 1;
				
				while(true)
					{
					xxx.Format(_T("File%d"), cnt2);
					CString tmpstr2 = AfxGetApp()->GetProfileString(section, xxx);

					xxx.Format(_T("File%d"), cnt2 - 1);
					//AP2N(_T("WriteProfileString shift %s - %s\r\n"), xxx, tmpstr2);
					AfxGetApp()->WriteProfileString(section, xxx, tmpstr2);

					cnt2++;
					
					if(cnt2 > MAX_MRU) 
						{
						break;
						}
					}
				}

			xxx.Format(_T("File%d"), cnt);
			//AP2N(_T("WriteProfileString %s - %s\r\n"), xxx, strx);
			AfxGetApp()->WriteProfileString(section, xxx, strx);
			}
		}

	// Done MRU
	//////////////////////////////////////////////////////////////////////

	wavefname = support.GetFileNameExt(strx); wavename = strx;

	int len2 = len / 2;

	short *lptr = (short*)malloc(len2); //ASSERT(lptr);
	short *rptr = (short*)malloc(len2); //ASSERT(rptr);

	dsp.SplitWave((short*)buff, len, lptr, rptr, len2);

	ClearUndo();

	if(soundbuff)
		free(soundbuff);

	soundbuff = (char*)lptr;
	soundlen  = len2;

	// Fresh start for this buffer
	startsel = endsel = currpos = startpos = wleft = moveoffset = 0;
	
	markstart = markend = markmove = false;
		
	xFrameSel();

	if(IsWindow(m_hWnd))
		Invalidate();

	changed++;
	free(buff); free(rptr);
	
	//AP2N(_T("CwaveEdit::OpenWave -- Opened %s (%s)\r\n"), wavename, wavefname);

	CString sss; sss.Format(_T("Opened '%s'  (%d Bytes)"), wavefname, soundlen);
	xShowInfo(sss, 1200);

	PostMessage(WM_USER+3, (int)m_hWnd, (int)(const TCHAR *)wavename);

	return true;
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::GetSel(int *ssel, int *esel)

{
	*ssel = startsel;
	*esel = endsel;
}

//////////////////////////////////////////////////////////////////////////
//

void CwaveEdit::GetSelBuff(TCHAR **pbuff, int *plen, int zcross)

{	
	xFrameSel();

	if(zcross)
		ZeroCross();

	*pbuff = (TCHAR *)soundbuff + startlen;	
	*plen = nsize;
}

void CwaveEdit::xShowInfo(const TCHAR *str, int timeout)

{
	CString str2(str);
	xShowInfo(str2, timeout);
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::xShowInfo(CString &str, int timeout)

{
	lasttimeout = timeout;
	if(lastinfo != "")
		{
		lastinfo = "";
		if(IsWindow(m_hWnd))
			Invalidate();
		}
	lastinfo = str;
	SetTimer(3, 100, NULL);
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::Copy()

{
	(OpenClipboard());
	
	EmptyClipboard();
 
	xFrameSel();

	//AP2N(_T("CwaveEdit::Copy() %d bytes\r\n"), nsize);

	char *sptr = soundbuff + startlen;
	HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nsize);
	void *ptr = GlobalLock(hMem); //ASSERT(hMem);
    memcpy(ptr, sptr, nsize);
    ::GlobalUnlock(hMem);
	HANDLE hh = SetClipboardData(cformat, hMem);
	(CloseClipboard());

	xShowInfo(_T("Copied to clipboard."), 600);		
}

void CwaveEdit::Paste()

{
	//AP2N(_T("CwaveEdit::Paste() %d bytes\r\n"), nsize);

	(OpenClipboard());

	HANDLE hh = GetClipboardData(cformat);

	if(!hh)
		return;

	int len = GlobalSize(hh);

	if(!len)
		return;

	void *ptr = GlobalLock(hh); //ASSERT(hh);

	//AP2N(_T("CwaveEdit::Paste() %d bytes\r\n"), len);
	
	mx.Lock();

	xFrameSel();

	char *newmem = (char *)malloc(len + soundlen); //ASSERT(newmem);

	if(!soundbuff)
		{
		memcpy(newmem, ptr, len);
		}
	else
		{
		// Re-assemble 
		memcpy(newmem, soundbuff, ipwleft);
		memcpy(newmem + ipwleft, ptr, len);
		memcpy(newmem + ipwleft + len, soundbuff + ipwleft, soundlen - ipwleft);
		//free(soundbuff);

		AddUndo();
		}

	GlobalUnlock(hh);

	soundbuff = newmem;
	soundlen = soundlen + len;

	(CloseClipboard());

	CRect rc; GetClientRect(rc);

	// Get it back from buffer dimentions
	wleft	  =  ipwleft *  rc.Width() / soundlen  ;			
	startsel  =  startlen *  rc.Width() / soundlen  ;
	endsel    =  endlen * rc.Width()   / soundlen  ;

	changed++;

	mx.Lock();

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::Undo()

{
	//if(!IsOKToGo())
	//	return;

	if(undoptr.GetSize() == 0)
		{
		xShowInfo(_T("Nothing to undo."), 600);
		return;
		}

	mx.Lock();

	char *ptr = (char*)undoptr.GetAt(undoptr.GetSize() - 1);

	int len =	undolen.GetAt(undolen.GetSize() - 1);

	if(soundbuff)
		free(soundbuff);

	soundbuff = ptr;
	soundlen = len;

	undoptr.RemoveAt(undoptr.GetSize() - 1);
	undolen.RemoveAt(undolen.GetSize() - 1);

	mx.Unlock();

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
// Make sure you do not free the buffer that is in the undo stack

void CwaveEdit::AddUndo()

{
	changed++;

	if(!soundbuff) 
		return;

	// Limit to N
	if(undoptr.GetSize() > undolim)
		{
		for(int loop = undoptr.GetSize() - 1; loop >= undolim; loop--)
			{
			//AP2N(_T("Rermoving undo %d\r\n"), loop);
			free( (TCHAR *) undoptr.GetAt(loop));
			undoptr.RemoveAt(loop); undolen.RemoveAt(loop);
			}
		}
	undoptr.Add(soundbuff); undolen.Add(soundlen);
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::Norm(int maxfact)

{
	if(!xIsOKToGo())
		return;
	
	char *outptr = (char*)malloc(soundlen); //ASSERT(outptr);
	short *pptr = (short*)soundbuff;
	
	AddUndo();

	mx.Lock();

	int	loop, maxx = 0;

	int len2 = soundlen / sizeof(short);	
	// Calc maximum value
	for( loop = 0; loop < len2; loop++)
		{
		short val = abs(*pptr++);

		if(maxx < val)
			maxx = val;
		}

	// Calc scaling factor
	double factor = (double)SHRT_MAX / maxx;

	//AP2N(_T("CwaveEdit::Normalize factor=%.2f\r\n"), factor);

	CString tmp; tmp.Format(_T("Normalize factor=%.f "), factor);
	xShowInfo(tmp, 2000);

	short *pptr2 = (short*)soundbuff;
	short *optr2 = (short*)outptr;

	// Limit scaling factor
	factor = min (factor, maxfact);
		
	//if(factor < maxfact)
	//	{
		// Multiply
		for(loop = 0; loop < len2; loop++)
			{
			short val = *pptr2++; 
			double dval = factor * val;		
			*optr2++ = short(dval);
			}
	//	}
	//else
	//	{
	//	// Copy straight
	//	for(loop = 0; loop < len2; loop++)
	//		{
	//		*optr2++ = *pptr2++;
	//		}
	//	}	

	//free(soundbuff);

	soundbuff = outptr;

	mx.Unlock();

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::ClearUndo()

{
	for(int loop = 0; loop < undoptr.GetSize(); loop++)
		{
		free( (TCHAR *) undoptr.GetAt(loop));
		}

	undoptr.RemoveAll();
	undolen.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::Del()

{
	AddUndo();

	mx.Lock();
	soundbuff = NULL;	soundlen = NULL;
	mx.Unlock();

	Invalidate();
}

#define SEPARATOR _T("")

//////////////////////////////////////////////////////////////////////////

void CwaveEdit::xCreateMenu(CXrayM *pmenu)

{
	CXrayM *sub = new CXrayM, *sub2 = new CXrayM;

	lastmenu2 = sub2;

	// Construct sub menu
	sub->AppendMenuItem( _T("Normalize"), ID_RCLICLK_PROCESS_NORMALIZE);
	sub->AppendMenuItem( SEPARATOR );
	sub->AppendMenuItem( _T("Fade In"),   ID_RCLICLK_PROCESS_FADEIN);
	sub->AppendMenuItem( _T("Fade Out"),  ID_RCLICLK_PROCESS_FADEOUT);

	// Construct sub menu 2

	CString xxx; int cnt = 0, nums = 0;
	while(true)
		{
		// Get Last one
		xxx.Format(_T("File%d"), cnt);
		CString sss = AfxGetApp()->GetProfileString(section, xxx);
		if(sss == "")
			break;

		cnt++;
		}

	while(true)
		{
		cnt--;

		if(cnt < 0)
			break;
	
		// Work backwards
		xxx.Format(_T("File%d"), cnt);
		CString sss = AfxGetApp()->GetProfileString(section, xxx);
		if(sss == "")
			break;

		if(nums >= MAX_MRU)
			break;

		nums++;
		sub2->AppendMenuItem(sss, ID_RCLICLK_MRU + cnt);		
		}
	
	// Fill it in
 	pmenu->AppendMenuItem( _T("Play\tSpace"),                 ID_RCLICLK_PLAY);
   	pmenu->AppendMenuItem( SEPARATOR );
   	pmenu->AppendMenuItem( _T("Select All (toggle)\tCtrl-A"), ID_RCLICLK_SELECTALL);
	pmenu->AppendMenuItem( SEPARATOR );	

	// Sub menu
   	pmenu->AppendSubMenu(_T("Process Sound"), sub);
	
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem(_T("Crop Selected\tCtrl-R"),       ID_RCLICLK_CROPSELECTED);
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem( _T("Cut Selected\tCtrl-X"),        ID_RCLICLK_CUTSELECTED);
	pmenu->AppendMenuItem( _T("Copy Selected\tCtrl-C"),       ID_RCLICLK_COPYSELECTED);
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem( _T("Paste\tCtrl-V"),               ID_RCLICLK_PASTE);
	pmenu->AppendMenuItem( _T("Paste as New\tCtrl-N"),        ID_RCLICLK_NEW_PASTE);
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem( _T("Save Buffer\tCtrl-Q"),         ID_RCLICLK_SAVE);
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem( _T("Save as &File\tCtrl-S"),       ID_RCLICLK_SAVEASFILE);
	pmenu->AppendMenuItem( _T("Save as new file\t"),		  ID_RCLICLK_SAVEASFILE2);
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem( _T("Open File\tCtrl-O"),           ID_RCLICLK_OPENFILE );
	
   	// Sub menu
   	pmenu->AppendSubMenu(_T("Recent Files"), sub2);
	
	pmenu->AppendMenuItem( SEPARATOR );
	pmenu->AppendMenuItem(_T(" Zero Crossing\tCtrl-G"),      ID_RCLICLK_ADJUSTTOZEROCROSSING );
	pmenu->AppendMenuItem( _T("Undo\tCtrl-Z"),                ID_RCLICLK_UNDO);
}

//////////////////////////////////////////////////////////////////////////

int CwaveEdit::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 

{
	AP2N(_T("CwaveEdit::OnMouseActivate %p\r\n"), this);

	if(GetFocus() != this)
		{
		SetFocus();
		return MA_ACTIVATE;
		}
	else
		return MA_NOACTIVATE;
}