// xRect.h: interface for the CxRect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XRECT_H__398EDAF2_137D_4FF9_9EB0_6AEA53E3F3FA__INCLUDED_)
#define AFX_XRECT_H__398EDAF2_137D_4FF9_9EB0_6AEA53E3F3FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CxRect : public CRect  
{
public:
	CxRect();
	virtual ~CxRect();

	CxRect&	CxRect::Clear();
	CxRect&	CxRect::Normalize();
	CxRect&	CxRect::Pad(int pad);

};

#endif // !defined(AFX_XRECT_H__398EDAF2_137D_4FF9_9EB0_6AEA53E3F3FA__INCLUDED_)
