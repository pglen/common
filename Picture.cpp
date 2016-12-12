// Picture.cpp: implementation of the CPicture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Picture.h"
#include "..\..\common\work\mxpad.h"


#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[]=_T(__FILE__);
#define new DEBUG_NEW
#endif

#define DEBUG_NEW new

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPicture::CPicture()
{
	m_pPicture	= NULL;
}

CPicture::~CPicture()
{
	xUnLoad();
}

bool CPicture::Load(CString sResourceType, CString sResource)
{
	bool bResult = false;

	if (m_pPicture != NULL)
		xUnLoad();

	if (m_pPicture == NULL)
	{
		TCHAR*	pBuff = NULL;
		int		nSize = 0;
		if (xGetResource(sResource.GetBuffer(0), 
						sResourceType.GetBuffer(0), pBuff, nSize))
		{
			if (nSize > 0)
			{
				pBuff = new TCHAR[nSize];

				if (xGetResource(sResource.GetBuffer(0), 
								sResourceType.GetBuffer(0), pBuff, nSize))
				{
					if (xLoadFromBuffer((BYTE*)pBuff, nSize))
						bResult = true;
				}

				delete [] pBuff;
			}
		}
	}
	return bResult;
}

bool CPicture::Load(CString sFileName)
{
	bool bResult = false;

	//P2N(_T("CPicture::Load %s\r\n"), sFileName);

	if (m_pPicture != NULL)
		xUnLoad();

	if (m_pPicture == NULL)
		{
		CFile cFile; CFileException	ex;

		if (cFile.Open(sFileName, CFile::modeRead | CFile::typeBinary, &ex))
			{
			int flen = (int)cFile.GetLength();

			BYTE* pBuff = new BYTE[flen];

			if (cFile.Read(pBuff, flen) > 0)
				{
				if (xLoadFromBuffer(pBuff, flen))
					bResult = true;
				}

			delete [] pBuff;
			current = sFileName;
			}
		else
			{
			//P2N(_T("Cannot load image %s\r\n"), sFileName);
			}
		}
	return bResult;
}

void CPicture::xUnLoad()
{
	if (m_pPicture != NULL)
	{
		m_pPicture->Release();
		m_pPicture = NULL;
	}
}

bool CPicture::Draw(CDC* pDC)
{
	if (m_pPicture != NULL)
	{
		long hmWidth;
		long hmHeight;
		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);

		int nWidth	= MulDiv(hmWidth,	pDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
		int nHeight	= MulDiv(hmHeight,	pDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);
		
		return Draw(pDC, 0, 0, nWidth, nHeight);
	}
	return false;
}

bool CPicture::Draw(CDC* pDC, CPoint Pos)
{
	if (m_pPicture != NULL)
	{
		long hmWidth;
		long hmHeight;
		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);

		int nWidth	= MulDiv(hmWidth,	pDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
		int nHeight	= MulDiv(hmHeight,	pDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);
		
		return Draw(pDC, Pos.x, Pos.y, nWidth, nHeight);
	}
	return false;
}

bool CPicture::Draw(CDC* pDC, CPoint Pos, CSize Size)
{
	if (m_pPicture != NULL)
	{
		long hmWidth;
		long hmHeight;
		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);

		int nWidth	= Size.cx;
		int nHeight	= Size.cy;
		
		return Draw(pDC, Pos.x, Pos.y, nWidth, nHeight);
	}
	return false;
}

bool CPicture::Draw(CDC* pDC, double nSizeRatio)
{
	if (m_pPicture != NULL)
	{
		long hmWidth;
		long hmHeight;
		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);

		int nWidth	= int(MulDiv(hmWidth,	pDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH) * nSizeRatio);
		int nHeight	= int(MulDiv(hmHeight,	pDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH) * nSizeRatio);
		
		return Draw(pDC, 0, 0, nWidth, nHeight);
	}
	return false;
}

bool CPicture::Draw(CDC* pDC, CPoint Pos, double nSizeRatio)
{
	if (m_pPicture != NULL)
	{
		long hmWidth;
		long hmHeight;
		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);

		int nWidth	= int(MulDiv(hmWidth,	pDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH) * nSizeRatio);
		int nHeight	= int(MulDiv(hmHeight,	pDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH) * nSizeRatio);
		
		return Draw(pDC, Pos.x, Pos.y, nWidth, nHeight);
	}
	return false;
}

bool CPicture::Draw(CDC* pDC, int x, int y, int cx, int cy)

{
	long hmWidth, hmHeight;

	m_pPicture->get_Width(&hmWidth);
	m_pPicture->get_Height(&hmHeight);

	if (m_pPicture->Render(pDC->m_hDC, x, y, cx, cy, 0, hmHeight, hmWidth, -hmHeight, NULL) == S_OK)
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Draw with alpha blending

bool CPicture::Draw(CDC* pDC, int alpha, int x, int y, int cx, int cy)

{
	//P2N(_T("CPicture::Draw alpha=%d\r\n"), alpha);

	//XntMask xmask;

	// Alpha blend
    BLENDFUNCTION m_bf; 
	
	m_bf.BlendOp	= AC_SRC_OVER;					
	m_bf.BlendFlags = 0;							
	m_bf.SourceConstantAlpha = alpha;
	m_bf.AlphaFormat = 0;							
				
    CDC  dcMem;	dcMem.CreateCompatibleDC(pDC);

	// CreateCompatibleBitmap does not work on color/memory device!

	HBITMAP bm =::CreateCompatibleBitmap(pDC->GetSafeHdc(), abs(cx), abs(cy));
	HBITMAP oldbm = (HBITMAP)::SelectObject(dcMem, bm);
	
	// Fill it in
	CRect rc(0, 0, abs(cx), abs(cy));
	dcMem.FillSolidRect(rc, RGB(255,255,255));

	// Draw
	Draw(&dcMem, 0, 0,  cx, cy);

	//HBITMAP hBmpMask = xmask.MakeMaskBmpDC(dcMem, cx, cy, RGB(255, 255, 255));

	// Put it out
	AlphaBlend(pDC->GetSafeHdc(), x, y, cx, cy, 
						dcMem.GetSafeHdc(), 0, 0, cx, cy, m_bf); 	

	//xmask.MaskBlt(pDC->GetSafeHdc(), x, y,	cx, cy, 
	//			dcMem, 0, 0, hBmpMask, 0, 0,
	//					MAKEROP4(SRCCOPY, DSTCOPY), alpha);

	::SelectObject(dcMem, oldbm);  ::DeleteObject(bm);	

	return true;
}


bool CPicture::xLoadFromBuffer(BYTE* pBuff, int nSize)
{
	bool bResult = false;

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, nSize);
	void* pData = GlobalLock(hGlobal);
	memcpy(pData, pBuff, nSize);
	GlobalUnlock(hGlobal);

	IStream* pStream = NULL;

	if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK)
	{
		HRESULT hr;
		if ((hr = OleLoadPicture(pStream, nSize, FALSE, IID_IPicture, (LPVOID *)&m_pPicture)) == S_OK)
			bResult = true;
	
		pStream->Release();
	}

	return bResult;
}

bool CPicture::xGetResource(TCHAR *lpName, TCHAR* lpType, void* pResource, int& nBufSize)
{ 
	HRSRC		hResInfo;
	HANDLE		hRes;
	HMODULE		hInst	= NULL; 
	LPSTR		lpRes	= NULL; 
	int			nLen	= 0;
	bool		bResult	= FALSE;

	// Find the resource
	hResInfo = FindResource(hInst, lpName, lpType);

	if (hResInfo == NULL) 
		return false;

	// Load the resource
	hRes = LoadResource(hInst, hResInfo);

	if (hRes == NULL) 
		return false;

	// Lock the resource
	lpRes = (char*)LockResource(hRes);

	if (lpRes != NULL)
	{ 
		if (pResource == NULL)
		{
			nBufSize = SizeofResource(hInst, hResInfo);
			bResult = true;
		}
		else
		{
			if (nBufSize >= (int)SizeofResource(hInst, hResInfo))
			{
				memcpy(pResource, lpRes, nBufSize);
				bResult = true;
			}
		} 

		UnlockResource(hRes);  
	}

	// Free the resource
	FreeResource(hRes);

	return bResult;
}

CSize CPicture::GetSize(CDC* pDC)
{
	CSize rcResult = CSize(0,0);
	
	if (m_pPicture != NULL)
	{
		long hmWidth;
		long hmHeight;
		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);

		rcResult.cx	= MulDiv(hmWidth,	pDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
		rcResult.cy	= MulDiv(hmHeight,	pDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);
	}

	return rcResult;
}

//////////////////////////////////////////////////////////////////////////

long CPicture::GetWidth()

{
	long hmWidth = 0;

	if (m_pPicture == NULL)
		return 0;

	CClientDC dc(AfxGetMainWnd());
	m_pPicture->get_Width(&hmWidth);

	hmWidth	= MulDiv(hmWidth, dc.GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);

	//P2N(_T("CPicture::GetWidth %d\r\n"), hmWidth);

	return hmWidth;
}

//////////////////////////////////////////////////////////////////////////

long CPicture::GetHeight()

{
	long hmHeight = 0;

	if (m_pPicture == NULL)
		return 0;

	CClientDC dc(AfxGetMainWnd());

	m_pPicture->get_Height(&hmHeight);

	hmHeight = MulDiv(hmHeight,dc.GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);

	//P2N(_T("CPicture::GetHeight %d\r\n"), hmHeight);

	return hmHeight;
}

bool CPicture::IsValid()
{
	return  (m_pPicture != NULL);
}
