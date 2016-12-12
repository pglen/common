// Picture.h: interface for the CPicture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PICTURE_H__6098A4C3_D6D5_4711_BC7B_1595F459B480__INCLUDED_)
#define AFX_PICTURE_H__6098A4C3_D6D5_4711_BC7B_1595F459B480__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPicture  
{
	static	bool	xGetResource(TCHAR *lpName, TCHAR *lpType, void* pResource, int& nBufSize);

	enum
	{
		HIMETRIC_INCH	= 2540
	};

public:
	CPicture();
	virtual ~CPicture();

	bool		IsValid();
	bool		Load(CString sResourceType, CString sResource);
	bool		Load(CString sFileName);
	bool		Draw(CDC* pDC);
	bool		Draw(CDC* pDC, CPoint Pos);
	bool		Draw(CDC* pDC, CPoint Pos, CSize Size);
	bool		Draw(CDC* pDC, double nSizeRatio);
	bool		Draw(CDC* pDC, CPoint Pos, double nSizeRatio);

	bool		Draw(CDC* pDC, int x, int y, int cx, int cy);
	bool		Draw(CDC* pDC, int alpha, int x, int y, int cx, int cy);

	CSize		GetSize(CDC* pDC);

	long		GetHeight();
	long		GetWidth();

	CString		current;

private:

	void			xUnLoad();
	bool			xLoadFromBuffer(BYTE* pBuff, int nSize);
	IPicture*		m_pPicture;
	
};

#endif // !defined(AFX_PICTURE_H__6098A4C3_D6D5_4711_BC7B_1595F459B480__INCLUDED_)
