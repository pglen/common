// xRect.cpp: implementation of the CxRect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xRect.h"

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[]=_T(__FILE__);
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxRect::CxRect()
	: CRect(0,0,0,0)
{
	
}

CxRect::~CxRect()
{

}
	

//////////////////////////////////////////////////////////////////////////
//
// Function name:    clear_rect
// Description:      <empty description>
// Return type:      
// Argument:         RECT *rc
//
//////////////////////////////////////////////////////////////////////////

CxRect&	CxRect::Clear()

{
	left = 0;
	top =  0;
	right = 0;
	bottom = 0;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
//
// Function name:    norm_rect
// Description:      Normalize rect (Swap top/buttom left rigth if needed)
// Return type:      
// Argument:         RECT *rc
//
//////////////////////////////////////////////////////////////////////////

CxRect&	CxRect::Normalize()

{
	if(left > right)
		{
		int tmp = left;
		left = right;	
		right = tmp;
		}

	if(top >  bottom)
		{
		int tmp = top;
		top = bottom;	
		bottom = tmp;
		}

	return *this;
}

//////////////////////////////////////////////////////////////////////////
//
// Function name:    pad_rect
// Description:      <empty description>
// Return type:      
// Argument:         RECT *rc
// Argument:         int pad
//
//////////////////////////////////////////////////////////////////////////

CxRect&	CxRect::Pad(int pad)

{
	Normalize();

	left -= pad;
	top -= pad;
	right += pad;
	bottom += pad;

	return *this;
}

