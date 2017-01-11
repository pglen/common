
/* =====[ CSimpleGrid.cpp ]========================================== 
                                                                             
   Description:     The dbtest project, implementation of the CSimpleGrid.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  12/19/2007  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include "support.h"
#include "SimpleGrid.h"
#include "mxpad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
//static TCHAR THIS_FILE[] = _T(__FILE__);
const char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CSimpleGrid, CWnd)
	//{{AFX_MSG_MAP(CSimpleGrid)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_RBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int		CSimpleGrid::change_message = WM_USER + 10;
int		CSimpleGrid::check_message = WM_USER + 11;
int		CSimpleGrid::dblclick_message = WM_USER + 12;

/////////////////////////////////////////////////////////////////////////////
// CSimpleGrid

CSimpleGrid::CSimpleGrid()

{
	firstpaint = true;
	first_bold =  allowpopup = false;

	ptip = new CPopup(); ptip->Create();

	xmaxcol = xmaxrow = rows = cols = currrow = currcol = 0;
	rowhigh = reenter = init_alloc = wasbold = 0;
	startrow = startcol = xpendauto = capped = fired = 0;
	
	head_size = 5;	maxwidth = 64;

	//xbackcolor = RGB(192, 192, 192);
	xbackcolor = GetSysColor(COLOR_3DFACE);
	
	//P2N(_T("Construct CSimpleGrid::CSimpleGrid()\r\n"));
	
	init_row = 10; init_col = 30;
	
	cell_www = cell_hhh = 0;

	memset(&charrc, 0, sizeof(charrc));
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(m_lf), &m_lf);

	m_font.CreateFontIndirect(&m_lf);
	m_lf.lfWeight  = FW_BOLD;
	m_fontb.CreateFontIndirect(&m_lf);

	ReAlloc(init_row, init_col);
	//tip.Create(this);
}

//////////////////////////////////////////////////////////////////////////

CSimpleGrid::~CSimpleGrid()

{
	for(int loop = 0; loop < rows ; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		for(int loop2 = 0; loop2 < cols; loop2++)
			{
			cell *ccc = (cell *)col->GetAt(loop2);

			//ASSERT(ccc->magic == CELL_MAGIC);

			CString *str = (CString *)ccc->content;
			//P2N(_T("Destructing cell %d %d %s\r\n"), ccc->cellrow, ccc->cellcol, *str);

			delete str;
			delete ccc;
			}
		delete col;
		}

	delete ptip;
}

/////////////////////////////////////////////////////////////////////////////
// CSimpleGrid message handlers

void CSimpleGrid::OnPaint() 

{
	if(firstpaint)
		{
		firstpaint = false;
		FirstPaint();
		}

	CPaintDC dc(this); // device context for painting

	RECT rect;	GetClientRect(&rect);

	//rect.left += 10;
	//rect.top += 10;
	//int www = (rect.right - rect.left) / cols;
	//int hhh = (rect.bottom - rect.top) / rows;

	CPen dashed(PS_SOLID, 0, RGB(0x80, 0x80, 0x80));
	CPen *old =  dc.SelectObject( &dashed);

	CPen solid(PS_SOLID, 1, RGB(0x0, 0x0, 0x0));
	
	RECT rc4; rc4.left = rc4.top = rc4.bottom = rc4.right = head_size;

	int loop;

	// Draw row highlite
	if(rowhigh)
		{
		RECT rc4a; rc4a.left = rc4a.top = rc4a.bottom = rc4a.right = head_size;

		for(int loopa = startrow; loopa < rows ; loopa++)
			{
			CPtrArray *col = (CPtrArray *)xrowarr[loopa];
			cell *ccc = (cell *)col->GetAt(0);

			rc4a.bottom = rc4a.top  + ccc->hh; 

			if(loopa == currrow && currrow != 0)
				{
				CBrush rb;	rb.CreateSolidBrush(RGB(192, 192, 192));
				
				CRect rc6(rc4a); 
				rc6.left = head_size;	rc6.right = rect.right - sw;
				rc6.DeflateRect(1,1,1,1);				
				dc.FillRect(rc6, &rb);
				}		

			rc4a.top += cell_hhh;
			rc4a.left = head_size;
			}
		}

	// Draw grid and check marks
	for(loop = startrow; loop < rows ; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		cell *ccc;
		for(int loop2 = startcol; loop2 < cols; loop2++)
			{
			ccc = (cell *)col->GetAt(loop2);

			CString *str = (CString *)ccc->content;

			rc4.right  = rc4.left + ccc->ww;
			rc4.bottom = rc4.top  + ccc->hh; 

			//rc4.right  = rc4.left + cell_www;
			//rc4.bottom = rc4.top  + cell_hhh; 

			//  t -  r | b -  l |
			
			dc.MoveTo(rc4.left,  rc4.top);
			dc.LineTo(rc4.right, rc4.top);
			dc.LineTo(rc4.right, rc4.bottom);
			dc.LineTo(rc4.left, rc4.bottom);
			dc.LineTo(rc4.left, rc4.top);

			if(ccc->check)
				{
				CRect rc6(rc4);
				rc6.left += (rc4.right - rc4.left) / 2 - 5;
			
				dc.MoveTo(rc6.left+2,   rc6.top + 3);
				dc.LineTo(rc6.left+12,  rc6.top + 3);
				dc.LineTo(rc6.left+12,  rc6.bottom - 3);
				dc.LineTo(rc6.left+2,   rc6.bottom - 3);
				dc.LineTo(rc6.left+2,   rc6.top + 3);

				CPen *old2 =  dc.SelectObject( &solid);

				if(ccc->state)
					{				
					dc.MoveTo(rc6.left+4,   rc6.top + 8);
					dc.LineTo(rc6.left+6,   rc6.bottom - 4);
					dc.LineTo(rc6.left+10,  rc6.top + 4);
					//dc.LineTo(rc4.left+10, rc4.top + 4);
					}
				else
					{
					//dc.MoveTo(rc4.left+4,   rc4.top + 4);					
					//dc.LineTo(rc4.left+10,	rc4.bottom - 4);

					//dc.MoveTo(rc4.left+4,   rc4.bottom - 4);					
					//dc.LineTo(rc4.left+10,	rc4.top + 4);
					}

				dc.SelectObject(old2);
				}

			rc4.left += ccc->ww; 
			//rc4.left += cell_www; 

			// Clip after last displyed column
			if(rc4.left  >= rect.right)
				break;
			}	

		//rc4.top += ccc->hh; 
		rc4.top += cell_hhh;

		rc4.left = head_size;

		// Clip after last displyed line
		if(rc4.top + ccc->hh >= rect.bottom)
			break;
		}

	dc.SelectObject(old);

	dc.SetBkMode(TRANSPARENT);

	CFont	*oldFont, *oldFont2;
	oldFont = dc.SelectObject(&m_font);
	
	rc4.left = rc4.top = rc4.bottom = rc4.right = head_size;

	for(loop = startrow; loop < rows; loop++)
		{
		if(wasbold)
			{
			dc.SelectObject(oldFont2);
			wasbold = false;
			}
		if(loop == 0 && first_bold)
			{
			oldFont2 = dc.SelectObject(&m_fontb);
			wasbold = true;
			}
		
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		cell *ccc;

		// Draw cell highlite, row highlite and text
		for(int loop2 = startcol; loop2 < cols ; loop2++)
			{
			ccc = (cell *)col->GetAt(loop2); 
			CString *str = (CString *)ccc->content;

			rc4.bottom = rc4.top + ccc->hh; 
			rc4.right = rc4.left + ccc->ww;

			//rc4.bottom = rc4.top + cell_hhh; 
			//rc4.right = rc4.left + cell_www;

			CRect rc5(rc4); 
			rc5.top += 1; rc5.left += 1;
			rc5.DeflateRect(1, 1);

			if(loop == currrow && loop2 == currcol)
				{
				dc.Draw3dRect(rc5, RGB(128, 128, 128), RGB(128, 128, 128));
				}
			
			//CString tmp; tmp.Format(_T("r:%d c:%d ch:%d"), ccc->cellrow, ccc->cellcol, ccc->state);
			//dc.DrawText(tmp, &rc4, DT_CENTER);

			dc.DrawText(*str, &rc5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			rc4.left += ccc->ww; 
			//rc4.left += cell_www; 
		
			// Clip to last displyed column
			if(rc4.left >= rect.right)
				break;

			//P2N(_T("Drawing cell %d %d '%s'\r\n"), ccc->cellrow, ccc->cellcol, *str);
			}
		rc4.top += cell_hhh;
		//rc4.top += ccc->hh;
		rc4.left = head_size;

		// Clip to last displyed line
		if(rc4.top + ccc->hh >= rect.bottom)
			break;
		}

	dc.SelectObject(oldFont);
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::BoldCell(int qrow, int qcol, int flag)

{
	cell *ccc = RowCol2Cell(qrow, qcol);
	if(ccc)
		{
		ccc->bold = flag;
		Invalidate();
		}
}


void CSimpleGrid::GetStr(int qrow, int qcol, CString &pstr)

{
	cell *cellx = RowCol2Cell(qrow, qcol);

	if(cellx)	
		{
		pstr = *((CString *)cellx->content);
		}
}


void CSimpleGrid::SetBGcolor(int col)

{
	xbackcolor = col;
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::AddStr(int qrow, int qcol, const TCHAR *str, int inval)

{
	//P2N(_T("CSimpleGrid::AddStr %d %d %s\r\n"), qrow, qcol, str);

	ReAlloc(qrow, qcol);

	xmaxrow = MAX(xmaxrow, qrow);
	xmaxcol = MAX(xmaxcol, qcol);

	CPtrArray *colarr = (CPtrArray *)xrowarr[qrow];
	//ASSERT(colarr);

	int sss = colarr->GetSize();

	//P2N(_T("getting col %d max=%d\r\n"), qcol, colarr->GetSize());
 	cell *ccc = (cell *)colarr->GetAt(qcol);
	 
	//ASSERT(ccc->magic == CELL_MAGIC);

	*((CString *)ccc->content) = str;

	if(inval)
		{
		if(IsWindow(m_hWnd))
			{
			//InvalidateRect(&rc4);
			Invalidate();
			//support.YieldToWinEx();
			}
		}
}

//////////////////////////////////////////////////////////////////////////

BOOL CSimpleGrid::OnEraseBkgnd(CDC* pDC) 

{
	RECT rect, m_rDataBox;
	
	GetClientRect(&rect); CopyRect(&m_rDataBox, &rect);
	
	//P2N(_T("Erase background\r\n"));	

	CBrush NewBrush(xbackcolor) ; 
	pDC->SetBrushOrg(0,0) ;
	CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&NewBrush);   
	pDC->PatBlt(rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, PATCOPY);
	pDC->SelectObject(pOldBrush);

	//return CWnd::OnEraseBkgnd(pDC);
	return true;
}

//////////////////////////////////////////////////////////////////////////

int CSimpleGrid::GetCheck(int qrow, int qcol)

{
	int ret = -1;

	cell *cellx = RowCol2Cell(qrow, qcol);

	if(cellx)	
		{
		if(cellx->check)
			ret = cellx->state;
		}

	return ret;
}

void CSimpleGrid::SetCheck(int qrow, int qcol, int val)

{
	//P2N(_T("CSimpleGrid::SetCheck(qrow=%d qcol=%d val=%d)\r\n"),
	//											qrow, qcol, val);

	cell *cellx = RowCol2Cell(qrow, qcol);

	if(cellx)	
		{
		//P2N(_T("Setting check val %d\r\n"), val);

		cellx->dclick = false;
		cellx->check = true;
		cellx->state = val;
		InvalidateGrid();

		GetParent()->PostMessage(check_message, qcol | (qrow << 16), val);
		}
}

void CSimpleGrid::OnLButtonDown(UINT nFlags, CPoint point) 

{
	//P2N(_T("Left button down\r\n"));

#if 0
	TRACKMOUSEEVENT te;

	te.cbSize = sizeof(TRACKMOUSEEVENT);
	te.dwFlags = TME_LEAVE;
	te.hwndTrack = this->m_hWnd;
	te.dwHoverTime = 0;

	::_TrackMouseEvent(&te);
#endif

	SetFocus();

	int roww, coll;	CRect rc;
	Pt2Cell(point, &roww, &coll, &rc);

	//P2N(_T("Pt2Cell(row=%d, col=%d CRect top=%d left=%d bottom=%d right=%d\r\n"), 
	//			roww, coll, rc.top, rc.left, rc.bottom, rc.right);

	// Mark current cell
	SetCurr(point);

	GetParent()->PostMessage(change_message, currrow, currcol);

	// Do checkbox toggle (if check enabled)
	int old = GetCheck(roww, coll);
	if(old != -1)
		{
		SetCheck(roww, coll, !old);
		}

	CWnd::OnLButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////

BOOL CSimpleGrid::PreTranslateMessage(MSG* pMsg) 

{
	//if(!mxpad.is_junk(pMsg->message))
	//	P2N(_T("Message: %d %s\r\n"), pMsg->message, mxpad.num2msg(pMsg->message));

	//////////////////////////////////////////////////////////////////////
	// Dispatch messages ourselvs
	if(pMsg->message == WM_CHAR)
		{
		OnChar(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 

		if(pMsg->wParam != VK_TAB)
			return true;
		}
	if(pMsg->message == WM_KEYDOWN)
		{
		if(pMsg->wParam == VK_RETURN)
			{
			return CWnd::PreTranslateMessage(pMsg);
			}

		OnKeyDown(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 

		if(pMsg->wParam != VK_TAB)
			return true;
		}
	if(pMsg->message == WM_KEYUP)
		{
		OnKeyUp(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		if(pMsg->wParam != VK_TAB)
			return true;		
		}
	if(pMsg->message == WM_SYSCHAR)
		{
		OnSysChar(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		//return 0;
		}
	if(pMsg->message == WM_SYSKEYDOWN)
		{
		OnSysKeyDown(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		//return 0;
		}
	if(pMsg->message == WM_SYSKEYUP)
		{
		OnSysKeyUp(pMsg->wParam, LOWORD(pMsg->lParam), pMsg->lParam); 
		//return 0;
		}

	//////////////////////////////////////////////////////////////////////
	
	if(pMsg->message == WM_MOUSELEAVE)
		{
		if(allowpopup)
			{	
			ptip->Hide();
			//P2N(_T("Mouse leave message\r\n"));
			capped = false;
			fired = false;
			}
		return 0;
		}

	if(pMsg->message == WM_MOUSEHOVER )
		{
		//P2N(_T("Mouse hover message\r\n"));
		fired = true;

		if(capped)
			{
			capmouse.x = mouse.x;
			capmouse.y = mouse.y;
			
			TRACKMOUSEEVENT te;

			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_HOVER | TME_CANCEL ;
			te.hwndTrack = this->m_hWnd;		
			::_TrackMouseEvent(&te);	

			//tip.Move(screen);
			//tip.SetText(_T("off cell"));
			
			// Find the text for this one
	
			CString str2 (_T(""));
			CString *str = &str2;

			RECT rc4; SetRectEmpty(&rc4);

			for(int loop = startrow; loop < rows; loop++)
				{
				cell *ccc;
				CPtrArray *col = (CPtrArray *)xrowarr[loop];

				for(int loop2 = startcol; loop2 < cols; loop2++)
					{
					ccc = (cell *)col->GetAt(loop2);

					rc4.right  = rc4.left + ccc->ww;
					rc4.bottom = rc4.top  + ccc->hh; 
					
					//rc4.right  = rc4.left + cell_www;
					//rc4.bottom = rc4.top  + cell_hhh; 

					CRect rc5(rc4);
					
					if(rc5.PtInRect(mouse))
						{
						str = (CString *)ccc->content;
						str2 = *str;

						//P2N(_T("Found string hit %s\r\n"), str2);
						break;
						}
					rc4.left += ccc->ww; 
					//rc4.left += cell_www; 
					}

				rc4.top += ccc->hh;
				rc4.left = 0;
				}

			if(str2 != _T("") && allowpopup)
				{
				// Autosize and autolocate
				CPoint screen = mouse;
				ClientToScreen(&screen); screen.x += 12; screen.y += 24;
				capped = false;
				ptip->Show(str2, screen);
				}
			}
		}

	return CWnd::PreTranslateMessage(pMsg);
}

void CSimpleGrid::OnMouseMove(UINT nFlags, CPoint point) 

{
	//P2N(_T("Mouse Move xx=%d yy=%d\r\n"), point.x, point.y);

	mouse.x = point.x;
	mouse.y = point.y;

	TRACKMOUSEEVENT te;

	if(!capped)
		{
		capped = true;

		if(!fired)
			{
			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = this->m_hWnd;
			::_TrackMouseEvent(&te);
		
			te.cbSize = sizeof(TRACKMOUSEEVENT);
			te.dwFlags = TME_HOVER;
			te.hwndTrack = this->m_hWnd;
			te.dwHoverTime = 1500;
		
			::_TrackMouseEvent(&te);
			}
		}	
	else 
		{
		if(fired)
			{
			// Moved more than hover pixels?
			if	(ABS(capmouse.x - point.x) > 5 ||
					ABS(capmouse.y - point.y) > 5)
				{
				//tip.ShowWindow(false);
				ptip->Hide();
				//P2N(_T("Move in no tracking\r\n"));
				fired = false;
				capped = false;
				}
			}
		}
		
	CWnd::OnMouseMove(nFlags, point);
}

//DEL int CSimpleGrid::OnCreate(LPCREATESTRUCT lpCreateStruct) 
//DEL 
//DEL {
//DEL 	//P2N(_T("Create function called\r\n"));
//DEL 	
//DEL 	if (CWnd::OnCreate(lpCreateStruct) == -1)
//DEL 		return -1;
//DEL 		
//DEL 	//P2N(_T("CSimpleGrid::Created window\r\n"));
//DEL 
//DEL 	return 0;
//DEL }

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::Clear()

{
	lock.Lock();

	// Create a copy
	int rows2 = rows, cols2 = cols ;

	for(int loop = 0; loop < rows2; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		for(int loop2 = 0; loop2 < cols2; loop2++)
			{
			cell *ccc = (cell *)col->GetAt(loop2);

			//ASSERT(ccc->magic == CELL_MAGIC);

			CString *str = (CString *)ccc->content;
			
			delete str;
			delete ccc;
			}
		delete col;
		}

	xrowarr.RemoveAll();	
	cols = rows = 0;
	
	lock.Unlock();

	ReAlloc(init_row, init_col);

	AutoSize();

	//P2N(_T("CSimpleGrid::Clear %d %d\r\n"), rows, cols);
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::FirstPaint()
			
{
	//ModifyStyleEx(WS_EX_NOPARENTNOTIFY,  WS_EX_TOPMOST, 0);
	//ModifyStyleEx(WS_EX_STATICEDGE, 0);

	ModifyStyle(WS_DISABLED | WS_BORDER , WS_CLIPSIBLINGS /*| WS_THICKFRAME */| SS_NOTIFY , SWP_DRAWFRAME);

	sw = GetSystemMetrics(SM_CXVSCROLL); sh = GetSystemMetrics(SM_CYHSCROLL);

	//TRACE(_T("First paint\r\n"));
	//P2N(_T("First paint sw=%d fh=%d\r\n"), sw, m_lf.lfHeight);

	// Measure one char
	CClientDC  dc(this); // device context for painting
	
	RECT rc2; GetClientRect(&rc2);

	CFont	*oldFont; oldFont = dc.SelectObject(&m_font);
	dc.DrawText(_T("a"), 1, &charrc, DT_CALCRECT);
	dc.SelectObject(oldFont);

	//P2N_RECT(rc2);	P2N_RECT(charrc);

	init_col = rc2.right / (charrc.right - charrc.left) / 15;
	init_row = rc2.bottom / (charrc.bottom - charrc.top + 4 );
	
	cell_www = ((rc2.right - rc2.left) - sw) / init_col;
	cell_hhh = ((rc2.bottom - rc2.top) - sh) / init_row;

	//P2N(_T("CSimpleGrid::FirstPaint init_row=%d init_col=%d\r\n"), init_row, init_col);
	//P2N(_T("CSimpleGrid::FirstPaint cell_www=%d cell_hhh=%d\r\n"), cell_www, cell_hhh);

	ReAlloc(init_row, init_col);

	// Fill in default width/height
	for(int loop = 0; loop < rows ; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];
		for(int loop2 = 0; loop2 < cols; loop2++)
			{
			cell *ccc = (cell *)col->GetAt(loop2);
			ccc->ww = cell_www;
			ccc->hh = cell_hhh;
			}
		}

	if(first_bold)
		{
		CPtrArray *colb = (CPtrArray *)xrowarr[0];
		for(int loop2 = 0; loop2 < cols; loop2++)
			{
			cell *ccc = (cell *)colb->GetAt(loop2);
			ccc->bold = true; ccc->dclick = false;
			ccc->nosel = true;
			}
		}
	
	// Create scroll bars
	RECT rc; GetClientRect(&rc);
	rc.left = rc.right - sw;
	rc.bottom -= sw;
	vs.Create(SBS_VERT, rc, this, 1000);
	vs.EnableWindow(false);	vs.ShowWindow(true);

	RECT rc3; GetClientRect(&rc3);
	rc3.top = rc3.bottom - sh;
	rc3.right -= sh;
	hs.Create(SBS_HORZ, rc3, this, 1001);
	hs.EnableWindow(false);	hs.ShowWindow(true);

	//hrcmenu.LoadMenu(IDR_MENU1);
	//hrclickmenu = (BCMenu *)hrcmenu.GetSubMenu(0);
	//this->EnableWindow();
	//this->SetActiveWindow();

	SetFocus();
	Invalidate();

	if(xpendauto)
		AutoSize();

	//P2N(_T("Text height %d\r\n"), rc.bottom);
}

//////////////////////////////////////////////////////////////////////////


void CSimpleGrid::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 

{
	//P2N(_T("Vscroll pos=%d\r\n"), nPos);

	switch (nSBCode)
		{
		case SB_THUMBTRACK  :
				startrow = nPos;
				startrow = min(rows - 1, startrow);
				//P2N(_T("Thumb track\r\n"));
				pScrollBar->SetScrollPos(startrow);
				InvalidateGrid();
				break;

		case SB_PAGEDOWN  :
				startrow = min(rows, startrow + 5);
		case SB_LINEDOWN  :
				startrow = min(rows, startrow + 1);;
				//P2N(_T("Vscroll down\r\n"));
				pScrollBar->SetScrollPos(startrow);
				InvalidateGrid();
				break;

		case SB_PAGEUP :
				startrow = max(startrow -5, 0);
		case SB_LINEUP  :
				startrow = max(0, startrow - 1);
				pScrollBar->SetScrollPos(nPos);
				//P2N(_T("Vscroll up\r\n"));
				pScrollBar->SetScrollPos(startrow);
				InvalidateGrid();
				break;
		}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 

{
	//P2N(_T("Hscroll pos=%d\r\n"), nPos);

	switch (nSBCode)
		{
		case SB_THUMBTRACK  :
				startcol = nPos;
				startcol = min(cols-1, startcol);
				
				//P2N(_T("Thumb track\r\n"));
				pScrollBar->SetScrollPos(startcol);
				InvalidateGrid();
				break;

		case SB_PAGEDOWN  :
				startcol = min(cols-1, startcol + 5);
		case SB_LINEDOWN  :
				startcol = min(cols-1, startcol + 1);
				P2N(_T("Hscroll down %d %d\r\n"), startcol, cols);
				pScrollBar->SetScrollPos(startcol);
				InvalidateGrid();
				break;

		case SB_PAGEUP :
				startcol = max(startcol -5, 0);
		case SB_LINEUP  :
				startcol = max(0, startcol - 1);
				pScrollBar->SetScrollPos(nPos);
				P2N(_T("Hscroll up\r\n"));
				pScrollBar->SetScrollPos(startcol);
				InvalidateGrid();
				break;
		}
	
	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::InvalidateRow(int row)

{
	row -= startrow;
	RECT rect;	GetClientRect(&rect);
	
	rect.left += head_size;
	rect.right -= sw;

	rect.top    = row * cell_hhh + head_size;
	rect.bottom = (row + 1) * cell_hhh + head_size;

	InvalidateRect(&rect, true);
}

//////////////////////////////////////////////////////////////////////////
//

void CSimpleGrid::InvalidateCell(int row, int col)

{
	
	RECT rect;	GetClientRect(&rect);
	rect.left += head_size;	rect.right -= sw;

	row -= startrow; row = MAX(row, 0);

	// Get row info
	rect.top    = row * cell_hhh + head_size;
	rect.bottom = (row + 1) * cell_hhh + head_size;

	// Walk row for column info
	row = MIN(row, xrowarr.GetSize());
	CPtrArray *pcol = (CPtrArray *)xrowarr[row];

	int alen = min(pcol->GetSize(), col);
	for(int loop = startcol; loop < alen; loop++)
		{
		cell *ccc = (cell *)pcol->GetAt(loop); 		
		rect.left += ccc->ww;
		rect.right = rect.left + ccc->ww;
 		}	

	InvalidateRect(&rect);	
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::InvalidateGrid()

{
	RECT rect;	GetClientRect(&rect);

	rect.right    -= sw;
	rect.bottom -= sh;

	InvalidateRect(&rect);	
}

//////////////////////////////////////////////////////////////////////////
//
//                            cols
//  --------------------------
// |                          |
// |       ------------------------
// |      |                   |    |
// |      |                   |    |
// |      |                   |    | 
// |      |                   |    |
// |      |                   |    |
//  ------|-------------------     |
//        |               rows     | newcol
//         ------------------------
//                                newrow   

void CSimpleGrid::ReAlloc(int newrows, int newcols)

{
	//P2N(_T("Realloc r=%d c=%d -> %d:%d\r\n"), rows, cols, newrow, newcol);

	int rowdiff = newrows - rows; int coldiff = newcols - cols;

	//P2N(_T("rDiff=%d cdiff=%d\r\n"), rowdiff, coldiff);

	// Anything to do?
	if(coldiff < 0 && rowdiff < 0)
		return;

	// Makes less frequent allocation
	newrows += 10;	newcols += 10;

	newrows = max(rows, newrows);
	newcols = max(cols, newcols);

	//P2N(_T("Realloc r=%d c=%d -> %d:%d\r\n"), rows, cols, newrows, newcols);
	
	if(newrows > init_row)
		{
		if(IsWindow(vs.m_hWnd))
			{
			vs.EnableWindow();
			vs.SetScrollRange(0, newrows);
			}
		}
	if(newcols > init_col)
		{
		if(IsWindow(hs.m_hWnd))
			{
			hs.EnableWindow();
			hs.SetScrollRange(0, newcols);
			}
		}

	lock.Lock();

	// Iterate every row
	for(int loop = 0; loop < newrows; loop++)
		{
		int new_row = false;
		CPtrArray *col;
		if(loop >= rows)
			{
			//P2N(_T("Row alloc: %d\r\n"), loop);
			col = new CPtrArray();
			xrowarr.Add(col);
			new_row = true;
			}
		else
			{
			col = (CPtrArray *)xrowarr.GetAt(loop);
			}
		// Iterate every col
		for(int loop2 = 0; loop2 < newcols; loop2++)
			{
			if((loop2 >= cols) || (new_row == (int)true))
				{
				cell *ccc = new cell; ////ASSERT(cell);

				// Initialize cell
				memset(ccc, 0, sizeof(cell));
				ccc->dclick = true;
				ccc->magic = CELL_MAGIC;
				ccc->ww = cell_www;
				ccc->hh = cell_hhh;
				ccc->cellrow = loop;
				ccc->cellcol = loop2;
				ccc->color = RGB(0,0,0);

				CString *str = new CString; 			
				ccc->content = str;
				
				col->Add(ccc);
				}			
			}
		}
	
	// Assign rows. cols
	cols = newcols;	rows = newrows;
	
	lock.Unlock();
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::AutoSize()

{
	//P2N(_T("Autosize\r\n"));

	xpendauto = true;

	CDWordArray sarr; sarr.SetSize(cols);
	int loop;

	// Determine max width per row
	for(loop = 0; loop < rows; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		cell *ccc;
		for(int loop2 = 0; loop2 < cols ; loop2++)
			{
			ccc = (cell *)col->GetAt(loop2); 
			CString *str = (CString *)ccc->content;

			sarr[loop2] = max(sarr[loop2], (DWORD)str->GetLength());
			}
		}

	// Limit rows to maximum size
	for(loop = 0; loop < cols ; loop++)
		{
		sarr[loop] = min(sarr[loop], (DWORD)maxwidth);
		//P2N(_T("c=%d-w=%d  "), loop, sarr[loop]); 
		}

	// Apply it for the whole 
	for(loop = 0; loop < rows; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		cell *ccc;
		for(int loop2 = 0; loop2 < cols ; loop2++)
			{
			ccc = (cell *)col->GetAt(loop2); 

			// Restore normal width if 0
			if(sarr[loop2] == 0)
				ccc->ww = cell_www;
			else
				ccc->ww = (sarr[loop2] + 4) * (charrc.right - charrc.left);
			}
		}
	
	//InvalidateGrid();
	//P2N(_T("\r\n"));
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::OnRButtonDown(UINT nFlags, CPoint point) 

{
	P2N(_T("Rbutton down\r\n"));

	// Mark current cell
	SetCurr(point);
	InvalidateGrid();

	RECT rc4; rc4.left = rc4.top = rc4.bottom = rc4.right = 0;

	int	found = false;
	cell *ccc = NULL;
		
	for(int loop = startrow; loop < rows; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		for(int loop2 = startcol; loop2 < cols; loop2++)
			{
			ccc = (cell *)col->GetAt(loop2);
			
			rc4.right  = rc4.left + ccc->ww;
			rc4.bottom = rc4.top  + ccc->hh; 

			CRect rc5(rc4);
			
			if(rc5.PtInRect(point))
				{
				found = true;
				goto endd;
				}
			rc4.left += ccc->ww; 
			}

		rc4.top += ccc->hh;
		rc4.left = 0;
		}

endd:
	if(found)
		{
		if(ccc)
			{
			CPoint pt(point);

			ClientToScreen(&pt);

			CString *str = (CString *)ccc->content;
			P2N(_T("Rbutton down %s\r\n"), *str);
			cccc = ccc;

			//YieldToWinEx();

			//hrclickmenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this, NULL);
			}
		}

	CWnd::OnRButtonDown(nFlags, point);
}

BOOL CSimpleGrid::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 

{
	P2N(_T("Command id=%d code=%d\r\n"), nID, nCode);
	
#if 0
	if(nCode == 0)
		{
		int cutflag = false;

		switch(nID)
			{
			case ID_RCLICK_CUT:
				cutflag = true;
				
			case ID_RCLICK_COPY:

				P2N(_T("Copy to clip '%s'\r\n"), *((CString *)cccc->content));
			    if (::OpenClipboard(AfxGetApp()->m_pMainWnd->GetSafeHwnd()))
					{
					//if(::EmptyClipboard())
					EmptyClipboard();
						{
						int len = ((CString *)cccc->content)->GetLength();
						HANDLE  txthandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, len + 1);

						TCHAR *txtptr = (TCHAR *)GlobalLock(txthandle);
						//ASSERT(txtptr);

						strncpy(txtptr, *((CString *)cccc->content), len);
						txtptr[len] = '\0';
            
						//::SetClipboardData(CF_TEXT, txthandle);
						::SetClipboardData(CF_UNICODETEXT, txthandle);
						
						}
					CloseClipboard();
					if(cutflag)
						{
						*((CString *)cccc->content) = "";
						InvalidateGrid();
						}
					}
				break;

			case ID_RCLICK_PASTE:
				{
				P2N(_T("Paste operation\r\n"));

				if (::OpenClipboard(AfxGetApp()->m_pMainWnd->GetSafeHwnd()))
					{	
					HANDLE htext = GetClipboardData(CF_UNICODETEXT);
					if (htext)
						{
						LPVOID ptr = GlobalLock(htext);
						//ASSERT(ptr);

						P2N(_T("Got clipboard text=%s\r\n"), ptr);
						*((CString *)cccc->content) = (TCHAR *)ptr;

						GlobalUnlock(htext);
						InvalidateGrid();
						}
					::CloseClipboard();
					}
				}
				break;
			}

		}
#endif

	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CSimpleGrid::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 

{
	
	P2N(_T("CSimpleGrid::OnChar %d\r\n"), nChar);
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CSimpleGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 

{
	//P2N(_T("CSimpleGrid::OnKeyDown nChar=%d nRepCnt=%d nFlags=%x\r\n"), nChar, nRepCnt, nFlags);
	int needmessage = false;

	switch(nChar)
		{
		case VK_RIGHT:
			currcol++;
			currcol = min(cols, currcol);
			needmessage = true;
			break;

		case VK_LEFT:
			currcol--;
			currcol = max(0, currcol);
			needmessage = true;
			break;

		case VK_UP:
			currrow--;
			currrow = max(0, currrow);
			needmessage = true;
			break;

		case VK_DOWN:
			currrow++;
			currrow = min(rows, currrow);
			needmessage = true;
			Invalidate();
			break;

		case VK_HOME:
			currcol = 0;
			needmessage = true;
			break;

		case VK_SPACE:
			int ch = GetCheck(currrow, currcol);
			if(ch != -1)
				{
				SetCheck(currrow, currcol, !ch);
				}
			break;
		}
		
	if(needmessage)
		{
		GetParent()->PostMessage(change_message, currrow, currcol);
		Invalidate();
		}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSimpleGrid::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 

{	
	P2N(_T("CSimpleGrid::OnKeyUp %d\r\n"), nChar);
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::SetCurr(CPoint point)

{
	RECT rect;	GetClientRect(&rect);

	int oldcurr = currrow, oldcol = currcol;

	CRect rc6;	
	RECT rc4; rc4.left = rc4.top = rc4.bottom = rc4.right = head_size;

	for(int loop = startrow; loop < rows; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		cell *ccc;

		for(int loop2 = startcol; loop2 < cols ; loop2++)
			{
			ccc = (cell *)col->GetAt(loop2); 

			//CString *str = (CString *)ccc->content;

			rc4.bottom = rc4.top + ccc->hh; 
			rc4.right = rc4.left + ccc->ww;

			CRect rc5(rc4); 

			if((currrow == loop) && (currcol == loop2))
				{
				rc6 = rc5;
				}

			if(rc5.PtInRect(point))
				{
				if(!ccc->nosel)
					{
					currrow = loop;
					currcol = loop2;
					}

				// Erase old position's select rect
				InvalidateRect(rc6);
			
				// Put new pos in
				InvalidateRect(rc5);
				//Invalidate();
				break;
				}

			rc4.left += ccc->ww; 

			// Clip to last displyed column
			if(rc4.left + ccc->ww >= rect.right)
				break;
			}
		rc4.top += ccc->hh;
		rc4.left = head_size;

		// Clip to last displyed line
		if(rc4.top + ccc->ww >= rect.bottom)
			break;
		}

	if(oldcurr != currrow || oldcol != currcol)
		{
		//GetParent()->PostMessage(change_message, currrow, currcol);
		InvalidateGrid();
		}
	
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::OnLButtonDblClk(UINT nFlags, CPoint point) 

{
	int roww, coll;	CRect rc;

	Pt2Cell(point, &roww, &coll, &rc);
	
	cell *cell = RowCol2Cell(roww, coll);

	if(cell && cell->dclick)
		{
		CString *str; str = (CString *)cell->content;
		if(str)
			{	
			//CString str2; str2.Format(_T("'%s'"), *str);
			//AfxMessageBox(str2);						
			}
		GetParent()->PostMessage(dblclick_message, currrow, currcol);
		}

	CWnd::OnLButtonDblClk(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
// Translate point to cell coordinates

void CSimpleGrid::Pt2Cell(CPoint point, int *rowp, int *colp, CRect *rc)

{
	RECT rect;	GetClientRect(&rect);

	RECT rc4; rc4.left = rc4.top = rc4.bottom = rc4.right = head_size;

	// Safety
	*rowp = *colp = -1;

	for(int loop = startrow; loop < rows; loop++)
		{
		CPtrArray *col = (CPtrArray *)xrowarr[loop];

		cell *ccc;

		for(int loop2 = startcol; loop2 < cols ; loop2++)
			{
			ccc = (cell *)col->GetAt(loop2); 
			CString *str = (CString *)ccc->content;

			//rc4.bottom = rc4.top + cell_hhh; 
			//rc4.right = rc4.left + cell_www;

			rc4.bottom = rc4.top + ccc->hh; 
			rc4.right = rc4.left + ccc->ww;

			CRect rc5(rc4); 

			if(rc5.PtInRect(point))
				{
				if(rowp)
					*rowp = loop;

				if(colp)
					*colp = loop2;

				if(rc)
					*rc = rc5;
				break;
				}

			rc4.left += ccc->ww; 

			// Clip to last displyed column
			if(rc4.left + ccc->ww >= rect.right)
				break;
			}
		rc4.top += ccc->hh;
		rc4.left = head_size;

		// Clip to last displyed line
		if(rc4.top + ccc->hh >= rect.bottom)
			break;
		}
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::Resized()

{
	RECT rc;	GetClientRect(&rc);
	RECT rc2;	vs.GetClientRect(&rc2);
	RECT rc3;	hs.GetClientRect(&rc3);

	vs.SetWindowPos(NULL, rc.right - sw, 0, rc2.right - rc2.left, rc.bottom - rc2.top - sh, SWP_NOOWNERZORDER);
	hs.SetWindowPos(NULL, 0, rc.bottom - sh, rc.right - rc.left - sw, rc3.bottom - rc3.top, SWP_NOOWNERZORDER);
}

//////////////////////////////////////////////////////////////////////////

void CSimpleGrid::OnSize(UINT nType, int cx, int cy) 

{
	CWnd::OnSize(nType, cx, cy);
	
	Resized();
}

//////////////////////////////////////////////////////////////////////////

CSimpleGrid::cell *CSimpleGrid::RowCol2Cell(int roww, int coll)

{
	cell *ccc = NULL;

	if(roww < 0 || coll < 0)
		return ccc;

	CPtrArray *col = (CPtrArray *)xrowarr[roww];
	if(col)
		{	
		ccc =  (cell *)col->GetAt(coll); 
		}		
	return ccc;
}

//////////////////////////////////////////////////////////////////////////
// Return number of active rows

int		CSimpleGrid::GetRowCount()

{
	return xmaxrow + 1;	
}

//////////////////////////////////////////////////////////////////////////

int		CSimpleGrid::GeColCount()

{
	return xmaxcol + 1;	
}

//////////////////////////////////////////////////////////////////////////

int		CSimpleGrid::GetCurrRow()

{
	return currrow;
}
	

void	CSimpleGrid::SetCurrRow(int newrow)

{
	if(currrow == newrow)
		return;

	currrow = newrow;
	//GetParent()->PostMessage(change_message, currrow, currcol);
	Invalidate();
}
