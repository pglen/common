
/* =====[ FileDialogST.h ]========================================================== 
                                                                             
   Description:     The wed project, implementation of the FileDialogST.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  4/26/2010  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#ifndef _FILEDIALOGST_H_
#define _FILEDIALOGST_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Uncomment following line if you are using this class outside the DLL
#define _FILEDIALOGST_NODLL_

#ifndef _FILEDIALOGST_NODLL_
#ifndef	_CMLHTDLL_NOLIB_
	#ifdef _DEBUG
		#ifdef _UNICODE
			#pragma comment(lib, "CmlHTud.lib")
		#else
			#pragma comment(lib, "CmlHTd.lib")
		#endif
		#else
		#ifdef _UNICODE
			#pragma comment(lib, "CmlHTu.lib")
		#else
			#pragma comment(lib, "CmlHT.lib")
		#endif
	#endif
#endif

	#ifdef	_CMLHTDLL_BUILDDLL_
		#define	FILEDIALOGST_EXPORT	__declspec(dllexport)
	#else
		#define	FILEDIALOGST_EXPORT	__declspec(dllimport)
	#endif

#else
		#define	FILEDIALOGST_EXPORT
#endif

	enum SHVIEW_ListViewModes 
{
	SHVIEW_Default	= 0 ,
	SHVIEW_ICON		= 0x7029 ,
	SHVIEW_LIST		= 0x702B ,
	SHVIEW_REPORT	= 0x702C ,
	SHVIEW_THUMBNAIL= 0x702D ,
	SHVIEW_TILE		= 0x702E
} ;

// CFileDialog OPENFILENAME GetOpenFileName

class FILEDIALOGST_EXPORT	CFileDialogST  
{

static	UINT CALLBACK _OFNHookProc(
  HWND hdlg,      // handle to child dialog window
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
);

	static int __stdcall _BrowseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);


public:

	CFileDialogST(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, 
				DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, 
					CWnd* pParentWnd = NULL);

	virtual ~CFileDialogST();

	virtual		int DoModal();
	virtual		BOOL OnInitDialog( );

	CString GetPathName() const;
	CString GetFileName() const;
	CString GetFileTitle() const;
	CString GetFileExt() const;
	CString GetFileDir() const;
	CString GetFileDrive() const;

	POSITION GetStartPosition() const;
	CString GetNextPathName(POSITION& pos) const;

	int SelectFolder(LPCTSTR lpszTitle = NULL, LPCTSTR lpszStartPath = NULL, UINT ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS, CWnd* pParentWnd = NULL);
	CString GetSelectedFolder() const;

	static short GetVersionI()		{return 10;}
	static LPCTSTR GetVersionC()	{return (LPCTSTR)_T("1.0");}

private:
	struct OPENFILENAMEEX : public OPENFILENAME 
	{ 
		void*	pvReserved;
		DWORD	dwReserved;
		DWORD	FlagsEx;
	};

public:
	OPENFILENAMEEX	m_ofn;
	BOOL			m_bOpenFileDialog;
	int				m_thumbs;
	
private:

	void			xInitVars();

	TCHAR			m_szFile[MAX_PATH];
	TCHAR			m_szFileTitle[MAX_PATH];
	TCHAR			m_szSelectedFolder[MAX_PATH];
};

#endif 
