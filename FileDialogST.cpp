
/* =====[ FileDialogST.cpp ]========================================================== 
                                                                             
   Description:     The wed project, implementation of the FileDialogST.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  4/26/2010  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include "FileDialogST.h"
#include "mxpad.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static int showthumbs = 0;

UINT CALLBACK CFileDialogST::_OFNHookProc(
  HWND hdlg,      // handle to child dialog window
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
)

{
	//P2N("Called hook %s procedure win=%x\r\n", num2msg(uiMsg), hdlg);

	// The window needed serious repositioning (MICROSOFT?)

#if 1
	if(uiMsg == WM_INITDIALOG)
		{
		WINDOWPLACEMENT wp;
		::GetWindowPlacement(::GetParent(hdlg), &wp);

		int hh  = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		int ww  = wp.rcNormalPosition.right - wp.rcNormalPosition.left;

		RECT rect;
		AfxGetApp()->m_pMainWnd->GetWindowRect(&rect);

		int hh2, ww2;

		hh2  = rect.bottom - rect.top;
		ww2  = rect.right - rect.left;

		POINT pt;
		pt.x = rect.left + ww2/2 -  ww/2;
		pt.y = rect.top + hh2/2  - hh/2;

		::SetWindowPos(::GetParent(hdlg), NULL, pt.x, pt.y, 0, 0,
							SWP_NOSIZE |  SWP_NOZORDER );
		}
#endif

#if 1
	if(showthumbs)
		{
		// This is a hook to display the thumbnail view first
		if(uiMsg == 78)
			{
			HWND hWndParent = ::GetParent(hdlg) ;

			//char str[128];
			//::GetWindowText(hWndParent, str, 128);
			//P2N("Showindow on '%s'\r\n", str);

			//HWND hwndLv = FindWindowEx(hWndParent, NULL, "SHELLDLL_DefView", NULL) ;

			HWND hwndLv = FindWindowEx(hWndParent, NULL, NULL, NULL) ;

			// Hack-o-matic send thumbnail command to evert child
			while(true)
				{
				if(!hwndLv)
					break;

				HWND hwnd2 = FindWindowEx(hwndLv, NULL, NULL, NULL) ;

				SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0) ;

				// char str2[128];
				//::GetWindowText(hwndLv, str2, 128);
				//P2N("Found window:'%s'\r\n", str2);

				hwndLv = FindWindowEx(hWndParent, hwndLv, NULL, NULL) ;
				}
			}
		}
#endif

	return 0;
}

// Constructs a CFileDialogST object.
// Most frequently used parameters can be passed on the argument list.
//
// Parameters:
//		[IN]	bOpenFileDialog
//				Set to TRUE to construct a File Open dialog box or
//				FALSE to construct a File Save As dialog box.
//		[IN]	lpszDefExt
//				The default filename extension.
//				If the user does not include an extension in the Filename edit box,
//				the extension specified by lpszDefExt is automatically appended
//				to the filename.
//				If this parameter is NULL, no file extension is appended.
//		[IN]	lpszFileName
//				The initial filename that appears in the filename edit box.
//				If NULL, no filename initially appears
//		[IN]	dwFlags
//				A combination of one or more flags that allow you to customize the dialog box.
//		[IN]	lpszFilter
//				A series of string pairs that specify filters you can apply to the file.
//				If you specify file filters, only selected files will appear in the
//				Files list box.
//		[IN]	pParentWnd
//				Pointer to the owner window for the dialog box. Can be NULL.
//

CFileDialogST::CFileDialogST(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, 
							 DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd)
{
	//P2N("CFileDialogST::CFileDialogST 6 arg\r\n");

	xInitVars();

	// Store parameters
	m_bOpenFileDialog = bOpenFileDialog;

	if(lpszDefExt)
		m_ofn.lpstrDefExt = lpszDefExt;

	if (lpszFileName != NULL)
		m_ofn.lpstrFile = (LPTSTR)lpszFileName;
	else
		{
		m_ofn.lpstrFile = m_szFile;
		m_ofn.nMaxFile = MAX_PATH;
		}	
	
	m_ofn.lpstrFileTitle = m_szFileTitle;
	m_ofn.nMaxFileTitle = MAX_PATH;

	m_ofn.Flags |= dwFlags;

	if(lpszFilter)
		m_ofn.lpstrFilter = lpszFilter;
		
	if (pParentWnd != NULL)
		m_ofn.hwndOwner = pParentWnd->m_hWnd;
}

#if 0

CFileDialogST::CFileDialogST(BOOL bOpenFileDialog, CWnd* pParentWnd)

{
	//P2N("CFileDialogST::CFileDialogST 3 arg\r\n");

	xInitVars();
	
	// Store parameters
	m_bOpenFileDialog = bOpenFileDialog;

	if(pParentWnd)
		m_ofn.hwndOwner = pParentWnd->m_hWnd;
	else
		m_ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
}
	
CFileDialogST::CFileDialogST()

{
	//P2N("CFileDialogST::CFileDialogST 0 arg\r\n");

	xInitVars();

	m_bOpenFileDialog = TRUE;
}

#endif

void	CFileDialogST::xInitVars()

{
	::ZeroMemory(&m_ofn, sizeof(m_ofn));
	::ZeroMemory(&m_szFile, sizeof(m_szFile));
	::ZeroMemory(&m_szFileTitle, sizeof(m_szFileTitle));
	::ZeroMemory(m_szSelectedFolder, sizeof(m_szSelectedFolder));

	m_ofn.Flags |= OFN_EXPLORER | OFN_ENABLESIZING;

	// Initial positioning and thumb view
	m_ofn.Flags		|= OFN_ENABLEHOOK;
	m_ofn.lpfnHook	=  _OFNHookProc;

	m_thumbs = false;
}

// Constructs a CFileDialogST object.
// All required parameters must be initialized by hand accessing
// the m_ofn and m_bOpenFileDialog public members.

CFileDialogST::~CFileDialogST()
{
	//P2N("CFileDialogST::~CFileDialogST()\r\n");
}

// This function displays the file selection dialog box and allows the user to make a selection.
// All required fields of the m_ofn public structure must be filled. This can be done
// using the class constructor or accessing directly the structure. Also the public
// variable m_bOpenFileDialog must be set to TRUE to get an open dialog box or to FALSE to
// get a save dialog box.
//
// Return value:
//		IDOK
//			The user has selected a filename.
//		IDCANCEL
//			The user has closed the dialog without selecting any filename.
//

int CFileDialogST::DoModal()
{
	BOOL	bRetValue;
	DWORD	dwWinMajor;

	dwWinMajor = (DWORD)(LOBYTE(LOWORD(::GetVersion())));

	// Hack thru a global static variable
	showthumbs = m_thumbs;

	if (dwWinMajor >= 5)
		m_ofn.lStructSize = sizeof(m_ofn);
	else
		m_ofn.lStructSize = sizeof(OPENFILENAME);

	// Execute dialog
	if (m_bOpenFileDialog)
		bRetValue = ::GetOpenFileName(&m_ofn);
	else
		bRetValue = ::GetSaveFileName(&m_ofn);

	return bRetValue;

	//return (bRetValue ? IDOK : IDCANCEL);
} // End of DoModal


BOOL CFileDialogST::OnInitDialog( )

{
	//AfxMessageBox("INIT DIALOG");

	//P2N("Called init filedialog");

	//CFileDialog::OnInitDialog();
	return true;
}


// This function returns the full path of the selected file.
//
// Return value:
//			A CString object containing the full path of the file.
//

CString CFileDialogST::GetPathName() const
{
	return m_ofn.lpstrFile;
} // End of GetPathName

// This function returns the filename of the selected file.
//
// Return value:
//			A CString object containing the name of the file.
//

CString CFileDialogST::GetFileName() const
{
	return m_ofn.lpstrFileTitle;
} // End of GetFileName

// This function returns the title of the selected file.
//
// Return value:
//			A CString object containing the title of the file.
//

CString CFileDialogST::GetFileTitle() const
{
	TCHAR szTitle[MAX_PATH];

	// Split path into components
	_tsplitpath(m_ofn.lpstrFile, NULL, NULL, szTitle, NULL);

	return szTitle;
} // End of GeFileTitle

// This function returns the extension of the selected file.
//
// Return value:
//			A CString object containing the extension of the file.
//
CString CFileDialogST::GetFileExt() const
{
	TCHAR szExt[MAX_PATH];

	// Split path into components
	_tsplitpath(m_ofn.lpstrFile, NULL, NULL, NULL, szExt);

	return szExt;
} // End of GeFileExt

// This function returns the directory (without drive) of the selected file.
//
// Return value:
//			A CString object containing the directory (without drive) of the file.
//
CString CFileDialogST::GetFileDir() const
{
	TCHAR szDrive[MAX_PATH];
	TCHAR szDir[MAX_PATH];

	// Split path into components
	_tsplitpath(m_ofn.lpstrFile, szDrive, szDir, NULL, NULL);
	::lstrcat(szDrive, szDir);

	return szDrive;
} // End of GeFileDir

// This function returns the drive of the selected file.
//
// Return value:
//			A CString object containing the drive of the file.
//
CString CFileDialogST::GetFileDrive() const
{
	TCHAR szDrive[MAX_PATH];

	// Split path into components
	_tsplitpath(m_ofn.lpstrFile, szDrive, NULL, NULL, NULL);

	return szDrive;
} // End of GeFileDrive

// This function returns the position of the first element of the filename list.
//
// Return value:
//			A POSITION value that can be used for iteration.
//			NULL if the list is empty.
//
POSITION CFileDialogST::GetStartPosition() const
{
	return (POSITION)m_ofn.lpstrFile;
} // End of GetStartPosition

// This function returns the full path of the next selected file.
//
// Parameters:
//		[IN]	pos
//				A reference to a POSITION value returned by a previous GetNextPathName
//				or GetStartPosition function call.
//				NULL if the end of the list has been reached.
//
// Return value:
//			A CString object containing the full path of the file.
//
// Note:	this function has been copied exactly from the MFC
//			implementation of the CFileDialog class.
//
CString CFileDialogST::GetNextPathName(POSITION& pos) const
{
	BOOL bExplorer = m_ofn.Flags & OFN_EXPLORER;
	TCHAR chDelimiter;
	if (bExplorer)
		chDelimiter = '\0';
	else
		chDelimiter = ' ';

	LPTSTR lpsz = (LPTSTR)pos;
	if (lpsz == m_ofn.lpstrFile) // first time
	{
		if ((m_ofn.Flags & OFN_ALLOWMULTISELECT) == 0)
		{
			pos = NULL;
			return m_ofn.lpstrFile;
		}

		// find char pos after first Delimiter
		while(*lpsz != chDelimiter && *lpsz != '\0')
			lpsz = _tcsinc(lpsz);
		lpsz = _tcsinc(lpsz);

		// if single selection then return only selection
		if (*lpsz == 0)
		{
			pos = NULL;
			return m_ofn.lpstrFile;
		}
	}

	CString strPath = m_ofn.lpstrFile;
	if (!bExplorer)
	{
		LPTSTR lpszPath = m_ofn.lpstrFile;
		while(*lpszPath != chDelimiter)
			lpszPath = _tcsinc(lpszPath);
		strPath = strPath.Left(lpszPath - m_ofn.lpstrFile);
	}

	LPTSTR lpszFileName = lpsz;
	CString strFileName = lpsz;

	// find char pos at next Delimiter
	while(*lpsz != chDelimiter && *lpsz != '\0')
		lpsz = _tcsinc(lpsz);

	if (!bExplorer && *lpsz == '\0')
		pos = NULL;
	else
	{
		if (!bExplorer)
			strFileName = strFileName.Left(lpsz - lpszFileName);

		lpsz = _tcsinc(lpsz);
		if (*lpsz == '\0') // if double terminated then done
			pos = NULL;
		else
			pos = (POSITION)lpsz;
	}

	// only add '\\' if it is needed
	if (!strPath.IsEmpty())
	{
		// check for last back-slash or forward slash (handles DBCS)
		LPCTSTR lpsz = _tcsrchr(strPath, '\\');
		if (lpsz == NULL)
			lpsz = _tcsrchr(strPath, '/');
		// if it is also the last character, then we don't need an extra
		if (lpsz != NULL &&
			(lpsz - (LPCTSTR)strPath) == strPath.GetLength()-1)
		{
			ASSERT(*lpsz == '\\' || *lpsz == '/');
			return strPath + strFileName;
		}
	}
	return strPath + '\\' + strFileName;
} // End of GetNextPathName

int __stdcall CFileDialogST::_BrowseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED && lpData != NULL)
	{
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	else // uMsg == BFFM_SELCHANGED
	{
	}

	return 0;
} // End of BrowseCtrlCallback

// This function lets the user to selec a folder.
//
// Parameters:
//		[IN]	lpszTitle
//				Address of a null-terminated string that is displayed above the
//				tree view control in the dialog box. This string can be used to
//				specify instructions to the user. Can be NULL.
//		[IN]	lpszStartPath
//				Address of a null-terminated string containing the initial folder
//				to open. Can be NULL.
//		[IN]	ulFlags
//				Flags specifying the options for the dialog box.
//		[IN]	pParentWnd
//				Pointer to the owner window for the dialog box. Can be NULL.
//
// Return value:
//		IDOK
//			The user has selected a folder and pressed OK. A call
//			to GetSelectedFolder() will return the selected folder.
//		IDCANCEL
//			The user has closed the dialog without selecting any folder.
//
int CFileDialogST::SelectFolder(LPCTSTR lpszTitle, LPCTSTR lpszStartPath, UINT ulFlags, CWnd* pParentWnd)
{
	LPMALLOC		pMalloc;
	BROWSEINFO		bi;
	LPITEMIDLIST	pidl;
	int				nRetValue = IDCANCEL;

	::ZeroMemory(&bi, sizeof(bi));

	// Gets the Shell's default allocator
	if (::SHGetMalloc(&pMalloc) == NOERROR)
		{
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		
		if (pParentWnd != NULL)
			bi.hwndOwner = pParentWnd->m_hWnd;

		bi.pidlRoot = NULL;
		bi.pszDisplayName = m_szSelectedFolder;
		bi.lpszTitle = lpszTitle;
		bi.ulFlags = ulFlags;
		bi.lpfn = _BrowseCtrlCallback;
		bi.lParam = (LPARAM)lpszStartPath;
		
		// This next call issues the dialog box.
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, m_szSelectedFolder))
			{
				// At this point pszBuffer contains the selected path
				nRetValue = IDOK;
			} // if
			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pidl);
		} // if
		// Release the shell's allocator.
		pMalloc->Release();
	} // if

	return nRetValue;
} // End of SelectFolder

// This function returns the folder selected by the user with a call to SelectFolder.
//
// Return value:
//			A CString object containing the selected folder.
//			Without a previous call to SelectFolder this string can be empty or
//			reflect the last selected folder.
//
CString CFileDialogST::GetSelectedFolder() const
{
	return m_szSelectedFolder;
} // End of GetSelectedFolder


