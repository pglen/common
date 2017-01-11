
/* =====[ Support.cpp ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the Support.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  5/20/2008  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

// Support.cpp: implementation of the CSupport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <malloc.h>
#include <direct.h>
#include <aclapi.h>
#include <winsvc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>


//#include "dogtalk.h"
#include "support.h"
#include "mxpad.h"

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[]=_T(__FILE__);
#define new DEBUG_NEW
#endif

CSupport support;

//////////////////////////////////////////////////////////////////////////
// Misc number conversions

unsigned int    CSupport::hextoi(const TCHAR *str, int lim)

{
    unsigned int sum = 0;
    TCHAR chh, val;

    while (1)
        {
        chh = *str;

        if(!isxdigit(chh))
           break;

        if(!lim)
           break;

        sum *= 0x10;

        if (isdigit(chh))
            val = chh - '0';
        else
            {
            chh = tolower(chh);
            val = chh - 'a' + 10;
            }
        sum += val;
        str++;  lim--;
        }
    return(sum);
}

unsigned int    CSupport::hexlen(const TCHAR *str)

{
    unsigned int sum = 0;

    while (1)
        {
        TCHAR chh = *str;

        if(!isxdigit(chh))
           break;

        str++; sum++;
        }
    return(sum);
}

unsigned int    CSupport::dectoi(const TCHAR *str, int lim)

{
    unsigned int sum = 0;
    TCHAR chh, val;

    while (1)
        {
        chh = *str;

        if(!isdigit(chh))
           break;

        if(!lim)
           break;

        sum *= 10;
        val = chh - '0';

        sum += val;
        str++;  lim--;
        }
    return(sum);
}

unsigned int    CSupport::declen(const TCHAR *str)

{
    unsigned int sum = 0;

    while (1)
        {
        TCHAR chh = *str;

        if(!isdigit(chh))
           break;

        str++; sum++;
        }
    return(sum);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSupport::CSupport()

{
	init_perf = 0;
}

CSupport::~CSupport()

{

}

void CSupport::HeapDump(const TCHAR *str2, int showok)

{
	// LEAK
	_HEAPINFO hi;

	malloc(12);

	memset(&hi, 0, sizeof(hi));

	P2N(_T("Heap Walk\r\n"));

	int cnt = 0;
	while (true)
		{
		int ret = _heapwalk(&hi);

		switch( ret )
			{
			case _HEAPOK:
				P2N(_T("%p %4d %s, "), hi._pentry, hi._size, hi._useflag == _USEDENTRY ? "USED" : "FREE");

				if(++cnt % 6 == 0)
					P2N(_T("\r\n"));
				break;

			case _HEAPEMPTY:
				P2N(_T("%s Heap Check -- OK - heap is empty\r\n"), str2);
				break;

			case _HEAPBADBEGIN:
				P2N(_T("%s Heap Check --ERROR - bad start of heap\r\n"), str2);
				break;

			case _HEAPBADNODE:
				P2N(_T("%s Heap Check --ERROR - bad node in heap\r\n"), str2);
				break;

			case _HEAPEND:
				P2N(_T("\r\n%s Heap Check --END\r\n"), str2);
				goto endd;
				break;

			default:
				P2N(_T("%s UNKNOWN - heap ret code\r\n"), str2);
				break;
			}
		}
endd:
	//P2N(_T("\r\nEnd heap walk.\r\n"));
	;
}

void CSupport::HeapCheck(const TCHAR *str, int showok)

{
	CString str2 = str;
	
	int heapstatus = _heapchk();

	//if(str2 == "")
	//	{
	//	str2.Format(_T("%s -- line :: %d "), _T(__FILE__) , __LINE__);
	//	}

   //P2N(_T("Heap check ret = %d\r\n"), _heapchk());

	switch( heapstatus )
		{
		case _HEAPOK:
			if(showok)
				P2N(_T("%s Heap Check -- OK - heap is fine\r\n"), str2);
			break;

		case _HEAPEMPTY:
			P2N(_T("%s Heap Check -- OK - heap is empty\r\n"), str2);
			break;

		case _HEAPBADBEGIN:
			P2N(_T("%s Heap Check --ERROR - bad start of heap\r\n"), str2);
			break;

		case _HEAPBADNODE:
			P2N(_T("%s Heap Check --ERROR - bad node in heap\r\n"), str2);
			break;

		default:
			P2N(_T("%s UNKNOWN - heap ret code\r\n"), str2);
			break;
		}
}

//////////////////////////////////////////////////////////////////////////
// Performace counter routines (normally 3.5 Mhz)
// Call this to mark the time

int		CSupport::MarkPerf(int *curr)

{
	if(!init_perf)
		{
		init_perf = true;
		QueryPerformanceFrequency(&Frequency);

		//AP2N(_T("freq=%d - %d\r\n"), Frequency.HighPart, Frequency.LowPart);
		Frequency.LowPart /= 1000;  //(to msec)
		}

	if(curr)
		{
		LARGE_INTEGER tmp;
		QueryPerformanceCounter(&tmp); *curr = tmp.LowPart;
		}
	else
		{
		QueryPerformanceCounter(&PerformanceCount);
		}
	
	//AP2N(_T("count = %d - %u\r\n"),	PerformanceCount.HighPart, PerformanceCount.LowPart);

	return(0);
}

/////////////////////////////////////////////////////////////////////////////
// Get number of milliseconds since last call to mark
// Quick and dirty implement. we assume the HighPart is not wrapping
//  

int		CSupport::QueryPerf(int *old)

{
	int elapsed = 0; LARGE_INTEGER lastcount; //, lastcount2;

	QueryPerformanceCounter(&lastcount);
	
	// Just to check latency -- it was 2 microsec on my machine 
	//  It can be ignored if calls are in the msec range

#if 0
	QueryPerformanceCounter(&lastcount2);
	int ilate = lastcount2.LowPart - lastcount.LowPart;
	int late  = (ilate  * 1000000) / Frequency.LowPart;

	AP2N(_T("Call Latency: %d usec (microsec) %d clocks\r\n"), late, ilate);	
#endif

	if(old)
		elapsed = (lastcount.LowPart - *old) / Frequency.LowPart;	
	else
		elapsed = (lastcount.LowPart - PerformanceCount.LowPart) / Frequency.LowPart;	

	//AP2N(_T("Time elapsed between calls to perfcount: %d\r\n"), elapsed); 
	
	//AP2N(_T("timer diff %d %d %d\r\n"), (lastcount.LowPart - PerformanceCount.LowPart),
	//	Frequency.LowPart, (lastcount.LowPart - PerformanceCount.LowPart) / Frequency.LowPart);

	return(elapsed);
}

/////////////////////////////////////////////////////////////////////////////
// Get number of microseconds since last call to mark

int		CSupport::QueryPerfMicro()

{
	int elapsed = 0;

	LARGE_INTEGER lastcount;	QueryPerformanceCounter(&lastcount);
	
	double dval_old = LargeInt2Double(PerformanceCount);
	double dval_new = LargeInt2Double(lastcount);

	LARGE_INTEGER xxx;
	
	Double2LargeInt(dval_new, &xxx);

	int uslate = int((1000L * (dval_new - dval_old)) / PerformanceCount.LowPart);

	return uslate;
}

//////////////////////////////////////////////////////////////////////////
// Get where the application lives

void	CSupport::GetAppRoot(CString &aroot)

{
	TCHAR *tmp = aroot.GetBuffer(MAX_PATH + 1); //ASSERT(tmp);	
	GetModuleFileName(AfxGetInstanceHandle(), tmp, MAX_PATH);
	aroot.ReleaseBuffer();

	// Get application root
	int idx = aroot.ReverseFind('\\');
	if(idx >= 0)
		aroot = aroot.Mid(0, idx + 1);
}


void	CSupport::GetDocDir(CString &homedir)

{
	GetSpecialFolder(CSIDL_PERSONAL, homedir);
	RepairPath(homedir);
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::GetHomeDir(CString &homedir)

{
	//const TCHAR *env = getenv(_T("USERPROFILE"));

	TCHAR *env; size_t esize = _MAX_PATH;
	_tdupenv_s(&env, &esize, _T("USERPROFILE"));

	P2N(_T("USERPROFILE env %s\r\n"), env);

	if(env)
		{
		homedir = env;
		free(env);
		}	
	else
		{
		GetSpecialFolder(CSIDL_PERSONAL, homedir);
		}

	RepairPath(homedir);
}

//////////////////////////////////////////////////////////////////////////
// Get space for tmp data

void	CSupport::GetAppData(CString &adata)

{
	GetSpecialFolder(CSIDL_APPDATA, adata);
	RepairPath(adata);

	//data += appdirname + "\\";

	if(_taccess(adata, 0) < 0)
		{
		_tmkdir(adata);  _tchmod(adata, _S_IREAD | _S_IWRITE);
		}

	if(_taccess(adata, 0) < 0)
		{
		AfxMessageBox(_T("Cannot access application data storage, switching to c:\\tmp\\"));
		//appdata = "c:\\tmp\\";
		}
}

void	CSupport::GetSharedData(CString &sdata)

{	
	GetEnvStr(_T("ALLUSERSPROFILE"), sdata);

	RepairPath(sdata);
	//sdata += appdirname + "\\";

	if(_taccess(sdata, 0) < 0)
		{
		_tmkdir(sdata); _tchmod(sdata, _S_IREAD | _S_IWRITE);
		}

	if(_taccess(sdata, 0) < 0)
		{
		AfxMessageBox(_T("Cannot access application common data storage, switching."));
		//sdata = "c:\\tmp\\";
		}
}


bool CSupport::CreateDirectoryUserFullAccess(LPCTSTR lpPath)
{
#if 1
	int f = CreateDirectory(lpPath,NULL);
	if(!f)
		return false;
	
	HANDLE hDir = CreateFile(lpPath,READ_CONTROL|WRITE_DAC,0,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
	if(hDir == INVALID_HANDLE_VALUE)
		return true;
	
	ACL* pOldDACL=NULL;
	SECURITY_DESCRIPTOR* pSD = NULL;
	GetSecurityInfo(hDir,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL,NULL,&pOldDACL,NULL,(void**)&pSD);
	
	EXPLICIT_ACCESS ea={0};
	ea.grfAccessMode = GRANT_ACCESS;
	ea.grfAccessPermissions = GENERIC_ALL;
	ea.grfInheritance = CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE;
	ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
	ea.Trustee.ptstrName = TEXT("Users");
	
	ACL* pNewDACL = NULL;
	SetEntriesInAcl(1,&ea,pOldDACL,&pNewDACL);
	
	SetSecurityInfo(hDir,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL,NULL,pNewDACL,NULL);
	
	LocalFree(pSD);
	LocalFree(pNewDACL);
	CloseHandle(hDir);
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CSupport::ChMod(LPCTSTR lpPath, int mod)

{	
	HANDLE hDir = CreateFile(lpPath,READ_CONTROL|WRITE_DAC,0,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);

	if(hDir == INVALID_HANDLE_VALUE)
		{
		P2N(_T("Cannot open dir for access control\r\n"));
		return true;
		}
	
	ACL* pOldDACL=NULL;
	SECURITY_DESCRIPTOR* pSD = NULL;

	GetSecurityInfo(hDir, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL,&pOldDACL,NULL,(void**)&pSD);
	
	EXPLICIT_ACCESS ea={0};

	ea.grfAccessMode = GRANT_ACCESS;
	ea.grfAccessPermissions = GENERIC_ALL;
	ea.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
	ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
	ea.Trustee.ptstrName = TEXT("Users");
	
	ACL* pNewDACL = NULL;
	SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
	
	SetSecurityInfo(hDir, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pNewDACL, NULL);
	
	LocalFree(pSD);
	LocalFree(pNewDACL);
	CloseHandle(hDir);

	return true;
}

//

void	CSupport::GetTmpDir(CString &tmproot)

{
	CString newdir; 
	
	// Get temp
	TCHAR *tmp = newdir.GetBuffer(_MAX_PATH + 1); //ASSERT(tmp);	
	GetTempPath(_MAX_PATH, tmp);
	newdir.ReleaseBuffer();
	
	// Convert it
	TCHAR *tmp2 = tmproot.GetBuffer(_MAX_PATH + 1); //ASSERT(tmp2);	
	GetLongPathName(newdir, tmp2, _MAX_PATH);
	tmproot.ReleaseBuffer();
}

//////////////////////////////////////////////////////////////////////////
// Get where the application data lives

void	CSupport::GetDataRoot(CString &droot)

{
	TCHAR *tmp = droot.GetBuffer(MAX_PATH + 1); //ASSERT(tmp);	
	GetModuleFileName(AfxGetInstanceHandle(), tmp, MAX_PATH);
	droot.ReleaseBuffer();
	
	P2N(_T("Dataroot OBSOLETE\r\n"));

	// Get application root
	int idx = droot.ReverseFind('\\');
	if(idx >= 0)
		droot = droot.Mid(0, idx + 1);

	//droot = "c:\\XrayNotes\\";
}

//////////////////////////////////////////////////////////////////////////

CString	CSupport::GetDataRoot()

{
	CString ret;

	P2N(_T("Dataroot OBSOLETE\r\n"));
	TCHAR *tmp = ret.GetBuffer(MAX_PATH + 1); //ASSERT(tmp);	
	GetModuleFileName(::AfxGetInstanceHandle(), tmp, MAX_PATH);
	ret.ReleaseBuffer();
	
	// Get application root
	int idx = ret.ReverseFind('\\');

	if(idx >= 0)
		ret = ret.Mid(0, idx + 1);

	return ret;
}

//////////////////////////////////////////////////////////////////////////

long    CSupport::HashString(const TCHAR  *name)

{
    unsigned long    ret_val = 0;

    while(*name != '\0')
        {
		TCHAR chh = *name;
		if(chh != '\r' && chh != '\n') // && chh != '\\' && chh != ' '
			{
			ret_val ^= (long)chh;
			ret_val  = ROTATE_LONG_RIGHT(ret_val, 3);  /* rotate right */
			}
        *name++;
        }
    return((long)ret_val);
}

///////////////////////////////////////////////////////////////////////////
// YieldToWinEx
// Let every accumulated message go through
// Return 1 if ESC | Ctrl-Break pressed -- 2 if EXITING

int     CSupport::YieldToWinEx(HWND hWnd)

{
    int ret_val = 0;
    while(TRUE)
        {
        MSG msg;
        if(!PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
           break;

        // Ctrl-Break
        if(msg.message == WM_KEYDOWN)
            {
            if(msg.wParam == VK_CANCEL)
                {
                ret_val = 1; break;
                }
            }
        // Esc
        if(msg.message == WM_CHAR)
            {
            if(msg.wParam == VK_ESCAPE)
                {
                ret_val = 1; break;
                }
            }
        // App exit
        if(msg.message == WM_QUIT)
                {
                //wait_exit = TRUE;
                ret_val = 2; break;
                }
        // App kill
        if(msg.message == WM_DESTROY)
                {
                //wait_exit = TRUE;
                ret_val = 2; break;
                }
        // Every other message is processed
        TranslateMessage(&msg); DispatchMessage(&msg);
        }
    return(ret_val);
}

/////////////////////////////////////////////////////////////////////////////////////
//
// This routine is a helper that finds the path to the special folder:
//
// \param SpecialFolder			- an UINT-define (See #defines above or (MSDN))\n
// \param SpecialFolderString	- Reference to a CString that receives the path to the special folder
//
// Returns a BOOL - Found or not \n
//
// CSIDL_ALTSTARTUP  	File system directory that corresponds to the user's nonlocalized Startup program group.
// CSIDL_APPDATA  		File system directory that serves as a common repository for application-specific data.
// CSIDL_BITBUCKET  	File system directory containing file objects in the user's Recycle Bin. The location of this directory is not in the registry; it is marked with the hidden and system attributes to prevent the user from moving or deleting it.
// CSIDL_COMMON_ALTSTARTUP  File system directory that corresponds to the nonlocalized Startup program group for all users.
// CSIDL_COMMON_DESKTOPDIRECTORY  File system directory that contains files and folders that appear on the desktop for all users.
// CSIDL_COMMON_FAVORITES  File system directory that serves as a common repository for all users' favorite items.
// CSIDL_COMMON_PROGRAMS  File system directory that contains the directories for the common program groups that appear on the Start menu for all users.
// CSIDL_COMMON_STARTMENU  File system directory that contains the programs and folders that appear on the Start menu for all users.
// CSIDL_COMMON_STARTUP  File system directory that contains the programs that appear in the Startup folder for all users.
// CSIDL_CONTROLS  		Virtual folder containing icons for the Control Panel applications.
// CSIDL_COOKIES  		File system directory that serves as a common repository for Internet cookies.
// CSIDL_DESKTOP  		Windows Desktop virtual folder at the root of the namespace.
// CSIDL_DESKTOPDIRECTORY  File system directory used to physically store file objects on the desktop (not to be confused with the desktop folder itself).
// CSIDL_DRIVES  		My Computer  virtual folder containing everything on the local computer: storage devices, printers, and Control Panel. The folder may also contain mapped network drives.
// CSIDL_FAVORITES  	File system directory that serves as a common repository for the user's favorite items.
// CSIDL_FONTS  		Virtual folder containing fonts.
// CSIDL_HISTORY  		File system directory that serves as a common repository for Internet history items.
// CSIDL_INTERNET  		Virtual folder representing the Internet.
// CSIDL_INTERNET_CACHE  File system directory that serves as a common repository for temporary Internet files.
// CSIDL_NETHOOD  		File system directory containing objects that appear in the network neighborhood.
// CSIDL_NETWORK  		Network Neighborhood Folder—virtual folder representing the top level of the network hierarchy.
// CSIDL_PERSONAL  		File system directory that serves as a common repository for documents.
// CSIDL_PRINTERS  		Virtual folder containing installed printers.
// CSIDL_PRINTHOOD  	File system directory that serves as a common repository for printer links.
// CSIDL_PROGRAMS  		File system directory that contains the user's program groups (which are also file system directories).
// CSIDL_RECENT  		File system directory that contains the user's most recently used documents.
// CSIDL_SENDTO  		File system directory that contains Send To menu items.
// CSIDL_STARTMENU  	File system directory containing Start menu items.
// CSIDL_STARTUP  		File system directory that corresponds to the user's Startup program group. The system starts these programs whenever any user logs onto Windows NT or starts Windows 95.
// CSIDL_TEMPLATES  	File system directory that serves as a common repository for document templates.
//

BOOL	CSupport::GetSpecialFolder(UINT SpecialFolder, CString &SpecialFolderString)

{
	HRESULT hr;
	LPITEMIDLIST pidl;
	TCHAR szPath[_MAX_PATH];

	hr = SHGetSpecialFolderLocation(NULL, SpecialFolder, &pidl);

    if(SUCCEEDED(hr))
		{
		// Convert the item ID list's binary representation into a file system path
		
		if(SHGetPathFromIDList(pidl, szPath))
			{
			// Allocate a pointer to an IMalloc interface
			//LPMALLOC pMalloc;

			// Get the address of our task allocator's IMalloc interface
			//hr = SHGetMalloc(&pMalloc);

			// Free the item ID list allocated by SHGetSpecialFolderLocation
			//pMalloc->Free(pidl);

			// Free our task allocator
			//pMalloc->Release();

			//ILFree(pidl);
			CoTaskMemFree(pidl);

			// Work with the special folder's path (contained in szPath)
			SpecialFolderString = szPath;	SpecialFolderString += "\\";

			return TRUE;
			}
		}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Make sure dir has backslash at the end

void	CSupport::RepairPath(CString &dir)

{
	//AP2N(_T("CSupport::RepairPath %s\r\n"), dir);

	int len = dir.GetLength(); if(!len) return;

	if(dir.GetAt(len - 1) != '\\')
		dir += "\\";

	//AP2N(_T("CSupport::RepairPath out %s\r\n"), dir);
}

//////////////////////////////////////////////////////////////////////////
// Make sure dir has no backslash at the end

void	CSupport::UnRepairPath(CString &dir)

{
	int len = dir.GetLength(); if(!len) return;
		
	if(dir.GetAt(len - 1) == '\\')
		{
		dir = dir.Left(len - 1);
		}
}

//////////////////////////////////////////////////////////////////////////
// Return true if it is an admin

int CSupport::IsAdmin( )

{
    SC_HANDLE hSC;

	// Try an Admin Privileged API - if it works, return TRUE, else FALSE
    hSC = OpenSCManager(
        NULL,
        NULL,
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE  );

    if ( hSC == NULL ) 
        return FALSE;

    CloseServiceHandle( hSC );
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Shorten path to directory name

void	CSupport::PathToDir(CString &docname)

{
	int idx;
	CString filename;

	if((idx = docname.ReverseFind('\\')) != -1)
		{
		filename = docname.Left(idx+1);
		docname = filename;
		}
}

/////////////////////////////////////////////////////////////////////////////
// Shorten path to file name 

void	CSupport::PathToFname(CString &docname)

{
	int idx;
	CString filename;

	if((idx = docname.ReverseFind('\\')) != -1)
		{
		filename = docname.Right(docname.GetLength() - (idx + 1));
		docname = filename;
		}
}


void	CSupport::PathToExt(CString  &fname)

{
	int idx;
	CString filename;

	if((idx = fname.ReverseFind('.')) != -1)
		{
		filename = fname.Right(fname.GetLength() - (idx + 1));
		fname = filename;
		}
}

//////////////////////////////////////////////////////////////////////////////
// 

CString CSupport::GetPathNoExt(const CString& name)

{
	int idx = name.ReverseFind('.');

	if(idx != -1)
		{
		return name.Mid(0, idx);
		}

	return CString(_T(""));
}

//////////////////////////////////////////////////////////////////////////

CString CSupport::GetExtension(const CString& name)

{
	int len = name.GetLength();
	int i;
	for (i = len-1; i >= 0; i--){
		if (name[i] == '.'){
			return name.Mid(i+1);
		}
	}
	return CString(_T(""));
}

//////////////////////////////////////////////////////////////////////////////

CString CSupport::GetFilename(const CString& name)

{
	CString fname = GetFileNameExt(name);

	int len = fname.GetLength();
	int i;
	for (i = len-1; i >= 0; i--)
		{
		if (fname[i] == '.')
			{
			return fname.Left(i);
			}
		}
	return CString(_T(""));
}

////////////////////////////////////////////////////////////////////////////////
// Convert full path to file name and extension

CString CSupport::GetFileNameExt(const CString& name)

{
	int len = name.GetLength();
	int i;
	for (i = len-1; i >= 0; i--)
		{
		if (name[i] == '\\')
			{
			return name.Right((len - i) -1 );
			}
		}
	return CString(name);
}

////////////////////////////////////////////////////////////////////////////////
// Convert full path to directory name

CString CSupport::GetDirName(const CString& name)

{
	int len = name.GetLength();
	int i;
	for (i = len-1; i >= 0; i--)
		{
		if (name[i] == '\\')
			{
			return name.Left(i);
			}
		}
	return CString(_T(""));
}


/////////////////////////////////////////////////////////////////////////////
// Wrapper for split path

void	CSupport::SplitPath(const CString &full,  CString &drive,
					CString &dir, CString &fname, CString &ext)

{
	//TCHAR sbuffer[_MAX_PATH + 1];
	TCHAR sdrive[_MAX_DRIVE];
	TCHAR sdir[_MAX_DIR];
	TCHAR sfname[_MAX_FNAME];
	TCHAR sext[_MAX_EXT];
	
	//strncpy(sbuffer, full, MAX_PATH);

	//Parse it
	_tsplitpath_s(full, sdrive, _MAX_DRIVE, sdir,_MAX_DIR,
						sfname, _MAX_FNAME, sext, _MAX_EXT);

	//P2N(_T("path components sdrive=%s sdir=%s  sfname=%s sext=%s\r\n"),
	//						sdrive, sdir, sfname, sext);

	drive = sdrive;
    dir   = sdir;
    fname = sfname;
    ext   = sext;
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::Help()

{
	CString approot; GetAppRoot(approot);
	
	P2N(_T("Approot='%s'\r\n"), approot);

	// Execute winhelp
	CString str, file(_T("index.html"));

#ifdef _DEBUG
	str = approot + _T("..\\umanual\\") + file;
#else
	str =  approot + _T("umanual\\") + file;
#endif

	int ret = (int) ShellExecute(NULL, _T("open"), str, NULL, NULL, SW_SHOWMAXIMIZED);
	P2N(_T("Shellexec returned with %d\r\n"), ret);
}

//////////////////////////////////////////////////////////////////////////

int		CSupport::SoftMkdir(const TCHAR *str)

{
	int ret = 0;

	if(!IsDir(str))
		{
		ret = _tmkdir(str); _tchmod(str, _S_IREAD | _S_IWRITE);
		}

	return ret;
}

/////////////////////////////////////////////////////////////////////////
// Return true if it is a directory 

int		CSupport::IsDir(const TCHAR *str)

{
	int ret = 0; 
	CString tmp(str);
	
	int idx3 = tmp.ReverseFind('\\'); if(idx3 >= 0) tmp = tmp.Left(idx3 );

	//P2N(_T("CSupport::IsDir tmp='%s'\r\n"), tmp);
	
	struct _stat buff; 

	if(_tstat(tmp, &buff) < 0)
		{
		//P2N(_T("Not stat\r\n"));
		goto endd;
		}

	if(buff.st_mode & _S_IFDIR)
		{
		//P2N(_T("Is dir\r\n"));
		ret = true;
		}
endd:

	return ret;
}

/////////////////////////////////////////////////////////////////////////
// Return true if it is a file

int		CSupport::IsFile(const TCHAR *str)

{
	int ret = 0;

	struct _stat buff; 
	if(_tstat( str, &buff) < 0)
		{
		//P2N(_T("CSupport::IsFile cannot stat\r\n"));
		goto endd;
		}

	if(buff.st_mode & _S_IFREG)
		{
		//P2N(_T("CSupport::IsFile regular file\r\n"));
		ret = true;
		}
endd:

	return ret;
}

//////////////////////////////////////////////////////////////////////////

int		CSupport::GetRegRootInt(const TCHAR *str, const TCHAR *key)

{
	HKEY	Result;
	DWORD	oldtime = 0;
	DWORD	plen = sizeof(DWORD);
	DWORD	Type = 0;

	LONG ret2 = RegOpenKey(HKEY_CLASSES_ROOT, str, &Result);

	if(ret2 == ERROR_SUCCESS)
		{
		RegQueryValueEx(Result, key, NULL, &Type, 
							(unsigned char*)&oldtime, &plen);
		RegCloseKey(Result);
		}
	return oldtime;
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::SetRegRootInt(const TCHAR *str, const TCHAR *key, int val)

{
	HKEY	Result;
	DWORD	oldtime = 0;
	DWORD	plen = sizeof(DWORD);

	LONG ret2 = RegOpenKey(HKEY_CLASSES_ROOT, str, &Result);

	if(ret2 != ERROR_SUCCESS)
		{
		/// Create key
		ret2 = RegCreateKeyEx(HKEY_CLASSES_ROOT, str, 0,
									NULL, 0, KEY_ALL_ACCESS, NULL, &Result, NULL);
		}

	if(ret2 == ERROR_SUCCESS)
		{
		RegSetValueEx(Result, key, NULL, REG_DWORD, 
								(unsigned char*)&val, sizeof(DWORD));
		RegCloseKey(Result);
		}
}

//////////////////////////////////////////////////////////////////////////

int		CSupport::GetLocMachineInt(const TCHAR *str, const TCHAR *key)

{
	HKEY	Result;
	DWORD	oldtime = 0;
	DWORD	plen = sizeof(DWORD);
	DWORD	Type = 0;

	LONG ret2 = RegOpenKey(HKEY_LOCAL_MACHINE, str, &Result);

	if(ret2 == ERROR_SUCCESS)
		{
		RegQueryValueEx(Result, key, NULL, &Type, 
							(unsigned char*)&oldtime, &plen);
		RegCloseKey(Result);
		}
	return oldtime;
}

void	CSupport::SetLocMachineInt(const TCHAR *str, const TCHAR *key, int val)

{
	HKEY	Result;
	DWORD	oldtime = 0;
	DWORD	plen = sizeof(DWORD);

	LONG ret2 = RegOpenKey(HKEY_LOCAL_MACHINE, str, &Result);

	if(ret2 != ERROR_SUCCESS)
		{
		/// Create key
		ret2 = RegCreateKeyEx(HKEY_LOCAL_MACHINE, str, 0,
									NULL, 0, KEY_ALL_ACCESS, NULL, &Result, NULL);
		}

	if(ret2 == ERROR_SUCCESS)
		{
		RegSetValueEx(Result, key, NULL, REG_DWORD, 
								(unsigned char*)&val, sizeof(DWORD));
		RegCloseKey(Result);
		}
}

////////////////////////////////////////////////////////////////////////////

CString CSupport::EscapeSql(CString &str)

{
	CString  res;

	int len = str.GetLength();

	for (int loop = 0; loop < len; loop++)
		{
		TCHAR chh = str.GetAt(loop);
		res +=  chh;
		if(chh == '\'')
			res +=  chh;
		}
	return res;
}


CString CSupport::EscapeComma(CString &str)

{
	CString  res;

	int len = str.GetLength();

	for (int loop = 0; loop < len; loop++)
		{
		TCHAR chh = str.GetAt(loop);

		switch(chh)
			{
			case '\r':
				res +=  "\\r";
				break;

			case '\n':
				res +=  "\\n";
				break;

			case ',':
				res +=  ";";
				break;

			default:
				res +=  chh;
				break;
			}
		}
	return res;
}


////////////////////////////////////////////////////////////
// Find how many characters in starting

int	 	CSupport::FindNumOfChar(CString &str, CString &mus, int start)

{
	int ret = 0, loop;
	int lim = str.GetLength();

	for(loop = start; loop < lim; loop++)
		{
		if(mus.Find(str.GetAt(loop)) >=0)
			{
			ret++;
			}
		}
	return(ret);
}


/////////////////////////////////////////////////////////////////////////////
//  void	ShortenPath(CString &str, int len)
// Shorten path to len

void	CSupport::ShortenPath(CString &str, int len)

{
	int slen = str.GetLength();

	// Nothing to do
	if(slen <= len)
		return;

	CString lstr = str.Left ((len-5)/2);
	CString rstr = str.Right((len-5)/2);
	str = lstr + _T(" ... ") + rstr;
}


/////////////////////////////////////////////////////////////////////////////
// Wrapper for get current dir

CString CSupport::GetCurrentDir()

{
	CString dir;
	TCHAR buffer[_MAX_PATH + 4];

	_tgetcwd(buffer, _MAX_PATH);

	// Fix up path for split
	if(_tcslen(buffer) > 3 ) 
	 	_tcscat_s(buffer, _MAX_PATH, _T("\\"));

	dir = buffer;
	return(dir);
}

#if 0

	CString cenv; GetEnvStr(_T("PATH"), cenv);

	int len = cenv.GetLength();

	int prog = 0;
	while(true)
		{
		int idx = cenv.Find(_T(";"), prog);

		if(idx < 0)
			{
			CString sub = cenv.Mid(prog);
			P2N(_T("last sub: %s\r\n"), sub);
			TryExec(sub, fstr);
			break;
			}

		CString sub = cenv.Mid(prog, idx - prog);
		//P2N(_T("sub: %s\r\n"), sub);
		if(TryExec(sub, fstr))
			break;

		prog =  idx + 1;

		// Safety net
		if(prog >= len)
			break;
		}

tryexec:

	CString cstr(str);
	
	cstr += "\\notepad.exe";

	//P2N(_T("TryExec %s %s\r\n"), cstr, arg);

	if(_access(cstr, 00) >= 0)
		{
		P2N(_T("TryExec OK %s %s\r\n"), cstr, arg);

		int ret = _spawnl(_P_NOWAIT, cstr, arg, NULL);
		P2N(_T("spawn returned %d\r\n"), ret);

		return 1;
		}

	return 0;
#endif

//////////////////////////////////////////////////////////////////////////
// Convert the error value into a string 

const TCHAR *CSupport::Err2Str(int errcode)

{
	LPVOID lpMsgBuf = NULL;

	if(errcode == 0)
		errcode = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errcode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,	0, NULL );

	// Kill last new line
	TCHAR *zz = _tcsrchr((TCHAR*)lpMsgBuf, '\r');
	if(zz) 
		*zz = '\0';

	errstr = (char*)lpMsgBuf;
	LocalFree( lpMsgBuf );

	return errstr;
}

//////////////////////////////////////////////////////////////////////////

void CSupport::CheckPoint()

{

#ifdef _DEBUG
	
	P2N(_T("CSupport::CheckPoint\r\n"));
	oldMemState.Checkpoint();

#endif

}

void	CSupport::ShowLeaks()

{
	//malloc(10);

#ifdef _DEBUG
    
	P2N(_T("memstate dump\r\n"));

	newMemState.Checkpoint();
    
	if(diffMemState.Difference( oldMemState, newMemState ) )
		{
        TRACE( _T("Memory leaked!\n") );
		}
	
	diffMemState.DumpAllObjectsSince();

	P2N(_T("dump end\r\n"));

#endif

}

static TCHAR    asztmp[512];

//////////////////////////////////////////////////////////////////////////

int     CSupport::AfxPrintf(const TCHAR *Format, ...)

{
    TCHAR    *asc = NULL;
    va_list ArgList;  va_start(ArgList, Format);

    _vsntprintf_s(asztmp, TSIZEOF(asztmp), TSIZEOF(asztmp), Format, ArgList);

    AfxMessageBox(asztmp);

    return(0);
}

void	CSupport::GetModuleBaseName(CString &bname)

{
	CString temp;

	TCHAR *tmp = temp.GetBuffer(MAX_PATH + 1); //ASSERT(tmp);	
	GetModuleFileName(AfxGetInstanceHandle(), tmp, MAX_PATH);
	temp.ReleaseBuffer();
	
	temp = GetFileNameExt(temp);
	temp = GetFilename(temp);

	bname = temp;
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::GetSharedDocDir(CString &sdata)

{	
	CString ttt;

	GetEnvStr(_T("ALLUSERSPROFILE"), ttt); RepairPath(ttt);
	CString base; GetModuleBaseName(base); RepairPath(base);

	ttt += _T("Documents\\") + base;

	if(_taccess(ttt, 0) < 0)
		{
		_tmkdir(ttt); _tchmod(ttt, _S_IREAD | _S_IWRITE);
		}

	// Double check, switch to temp if no access
	if(_taccess(ttt, 0) < 0)
		{
		//AfxMessageBox(_T("Cannot access common document storage."));
		GetEnvStr(_T("TEMP"), ttt); RepairPath(ttt);
		_tmkdir(ttt); _tchmod(ttt, _S_IREAD | _S_IWRITE);
		}

	// No temp acces
	if(_taccess(ttt, 0) < 0)
		{
		AfxMessageBox(_T("Cannot access temp dir."));
		ttt = "";
		}

	sdata = ttt;
	
	// Overridden by xraynotes
	//sdata = "c:\\xraynotes\\";
}

#define CSIDL_COMMON_APPDATA 0x0023

//////////////////////////////////////////////////////////////////////////

void	CSupport::GetProgDataDir(CString &datadir)

{
	//TCHAR tmp[_MAX_PATH];

	//int ret = GetEnvironmentVariable(_T("PROGRAMDATA"), tmp, sizeof(tmp));	

	GetSpecialFolder(CSIDL_COMMON_APPDATA, datadir);

	// Repair
	if(datadir == "")
		{
		}

	//P2N(_T("CSupport::GetProgDataDir '%s'\r\n"), tmp);
}

//////////////////////////////////////////////////////////////////////////
// Get space for tmp data

void	CSupport::GetAppDataDir(CString &adata)

{
	GetSpecialFolder(CSIDL_APPDATA, adata);
	RepairPath(adata);
	
	CString base; GetModuleBaseName(base);

	P2N(_T("got module base name %s\r\n"), base);

	adata += base + _T("\\");
	
	if(_taccess(adata, 0) < 0)
		{
		_tmkdir(adata);  _tchmod(adata, _S_IREAD | _S_IWRITE);
		}

	if(_taccess(adata, 0) < 0)
		{
		AfxMessageBox(_T("Cannot access application data storage, switching to c:\\tmp\\"));
		adata = _T("c:\\tmp\\");
		}
}

//////////////////////////////////////////////////////////////////////////

void CSupport::GetEnvStr(const TCHAR *env, CString &str)

{
	TCHAR tmp[_MAX_PATH];
	
	int ret = GetEnvironmentVariable(env, tmp, sizeof(tmp));	

	if(ret > 0)
		{
		TCHAR *pstr = str.GetBuffer(ret); //ASSERT(pstr);
		GetEnvironmentVariable(_T("PATH"), pstr, ret);	
		}

	P2N(_T("CSupport::GetEnvStr(): '%s'\r\n"), str);
}


/*
 *
 * Expand escape sequence like 'C'.
 *
 * Spec:     esc TCHAR '\\' (backslash)
 *
 *           special characters:     a   :   alert
 *                                   b   :   backspace
 *                                   f   :   f
 *                                   r   :   return
 *                                   n   :   newline
 *                                   t   :   tab
 *                                   v   :   vertical tab
 *                                   \\  :   backslash
 *                                   xnn :   hex numbered character
 *                                   nnn :   dec numbered character
 *
 *  The dec number overflow is interpretes as 2 dec numbered character +
 *  the following character interpreted normally.
 *
 */

void	CSupport::CUnEscape(const TCHAR *str, CString *out)

{
    int     ret_val = 0;

    // Clear result string
	*out = "";

    while(1)
        {
        if(!*str)
            break;

        switch(*str)
            {
            int tmp;

            case '\\':
                if(isdigit(*(str+1)))
                    {
                    unsigned int len = 3;

                    tmp  = dectoi(str+1, len);          // convert number
                    if(tmp > 255)                       // decimal overflow ?
                       len--;
                    tmp  = dectoi(str+1, len);          // re-convert number
                    len  = min(declen(str+1), len);
                    *out  += (char)tmp;
                    str += len;
                    }
                else
                    {
                    switch(*(str+1))
                        {
                        case 'x':
                            tmp = hextoi(str+2, 2);        // convert number
                            *out += (char)tmp;
                            str += min(hexlen(str+2),2);
                            str++;
                            break;

                        case 'a':   *out += (char)7;  str++;
							break;

                        case 'f':   *out += (char)12;  str++;
                            break;

                        case 'v':   *out += (char)11;  str++;
                            break;

                        case 'b':   *out += '\b';   str++;
                            break;

                        case 'n':   *out += '\r';   str++;
                            break;

                        case 'r':   *out += '\n';   str++;
                            break;

                        case 't':   *out += '\t';   str++;
                            break;

                        case '\\':  *out += '\\';   str++;
                            break;

                        case '\'':  *out += '\'';   str++;
                            break;

                        case '\"':  *out += '\"';   str++;
                            break;

                        default:
                            *out += *str;
                            str++;
                            break;
                        }
                    }
                break;

            default:
                *out += *str;
                break;
            }
        str++;  
        }
}

//////////////////////////////////////////////////////////////////////////
// Return true if eof reached

int		CSupport::ReadLine(CFile &fp, CString &str, int lim)

{
	TCHAR cc = 0, *ptr = str.GetBuffer(lim); //ASSERT(ptr);
	int ret, loop, prog = 0; 
	
	for(loop = 0; loop < lim - 1; loop++)
		{
		ret = fp.Read(&cc, 1);

		if(!ret)
			break;

		if(cc == '\r') continue;
		if(cc == '\n') break;

		ptr[prog++] = cc;
		}

	ptr[prog++] = '\0';

	str.ReleaseBuffer();

	return ret == 0;
}



//////////////////////////////////////////////////////////////////////////
//
// Function name:    largeuint2double
// Description:      <empty description>
// Return type:      
// Argument:         ULARGE_INTEGER lint
//
//////////////////////////////////////////////////////////////////////////

double	CSupport::LargeUInt2Double(ULARGE_INTEGER lint)

{
	double ret;

	ret =  (double)lint.HighPart;
	ret *= (1 << 16); ret *= (1 << 16);
	ret  += lint.LowPart;

	//AP2N(_T("LargeUInt2Double %d-%d %.f\r\n"), lint.HighPart, lint.LowPart, ret);

	return(ret);
}

double	CSupport::LargeInt2Double(LARGE_INTEGER lint)

{
	double ret = 0;

	ret =  (double)lint.HighPart;
	ret *= (1 << 16); ret *= (1 << 16);
	ret  += lint.LowPart;

	//AP2N(_T("LargeInt2Double %d-%d %.f\r\n"), lint.HighPart, lint.LowPart, ret);

	return(ret);
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::Double2LargeInt(double num, LARGE_INTEGER *lint)

{
	double hi =  num; 	hi /= 1 << 16;	hi /= 1 << 16;
	double lo = fmod(num , 0x100000000);

	lint->HighPart  =  (long)hi;
	lint->LowPart   =  (long)lo;

	//AP2N(_T("Double2LargeInt %d-%d %.f\r\n"), lint->HighPart, lint->LowPart, num);

}

//////////////////////////////////////////////////////////////////////////
//
// Function name:    uint64todouble
// Description:      <empty description>
// Return type:      
// Argument:         __int64 int64val
//
//////////////////////////////////////////////////////////////////////////

double	CSupport::uint64todouble(__int64 int64val)

{
	double ret = 0;

	unsigned int *ptr = (unsigned int*)&int64val;

	ret = *(ptr + 1);
	ret *= (1 << 16); ret *= (1 << 16);	
	ret += *(ptr);
	
	return(ret);
}



//////////////////////////////////////////////////////////////////////////

CString CSupport::time_t2str(time_t tme)

{
	CString str;

	CTime ct(tme);
	str = ct.Format(_T("%m/%d/%Y %H:%M"));
	return str;
}

//////////////////////////////////////////////////////////////////////////

CString CSupport::time_t2tstr(time_t tme)

{
	CString str;

	CTime ct(tme);
	str = ct.Format(_T("%d/%m/%y - %H:%M"));
	return str;
}

CString CSupport::ctime2str(CTime &ct)

{
	CString str;
	str = ct.Format(_T("%d/%m/%y - %H:%M"));
	return str;
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::ShortenStr(CString &str, int len)

{
	int slen = str.GetLength();

	// Nothing to do
	if(slen <= len)
		return;

	CString lstr = str.Left ((len-5)/2);
	CString rstr = str.Right((len-5)/2);
	str = lstr + _T(" ... ") + rstr;
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::ShortenStrL(CString &str, int len)

{
	CString lstr;
	int slen = str.GetLength();

	// Nothing to do
	if(slen <= len)
		return;

	// Find space right of
	int idx	= str.Find(_T(" "), len);

	if(idx >= 0)
		{
		lstr = str.Left(idx);
		}
	else
		{
		lstr = str.Left(len-5);
		}

	str = lstr + _T(" ...");
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::SetRegStr(HKEY root, const TCHAR *str, const TCHAR *key, CString &val)

{
	HKEY	Result;
	DWORD	oldtime = 0;
	
	LONG ret2 = RegOpenKey(root, str, &Result);

	if(ret2 != ERROR_SUCCESS)
		{
		/// Create key
		ret2 = RegCreateKeyEx(root, str, 0,
									NULL, 0, KEY_ALL_ACCESS, NULL, &Result, NULL);
		}

	BYTE *buff = (BYTE*)val.GetBuffer();

	if(ret2 == ERROR_SUCCESS)
		{
		RegSetValueEx(Result, key, NULL, REG_SZ, 
								(BYTE *)buff, val.GetLength());
		RegCloseKey(Result);
		}
}

//////////////////////////////////////////////////////////////////////////

CString		CSupport::GetRegStr(HKEY root, const TCHAR *str, const TCHAR *key)

{
	HKEY	Result;
	DWORD	oldtime = 0;
	DWORD	plen = _MAX_PATH;
	DWORD	Type = 0;

	CString ret;
	BYTE	*buff = (BYTE *)ret.GetBuffer(_MAX_PATH);

	//HKEY_CLASSES_ROOT

	LONG ret2 = RegOpenKey(root, str, &Result);

	if(ret2 == ERROR_SUCCESS)
		{
		RegQueryValueEx(Result, key, NULL, &Type, 
								buff, &plen);
		RegCloseKey(Result);
		}
	else
		{
		//P2N(_T("CSupport::GetRegStr cannot get key\r\n"));
		}

	ret.ReleaseBuffer();

	return ret;
}

//////////////////////////////////////////////////////////////////////////

int		CSupport::GetRegInt(HKEY root, const TCHAR *str, const TCHAR *key)

{
	HKEY	Result;
	DWORD	oldtime = 0;
	DWORD	plen = sizeof(DWORD);
	DWORD	Type = 0;

	//HKEY_CLASSES_ROOT

	LONG ret2 = RegOpenKey(root, str, &Result);

	if(ret2 == ERROR_SUCCESS)
		{
		RegQueryValueEx(Result, key, NULL, &Type, 
							(unsigned char*)&oldtime, &plen);
		RegCloseKey(Result);
		}
	else
		{
		P2N(_T("CSupport::GetRegInt cannot get key\r\n"));
		}
	return oldtime;
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::SetRegInt(HKEY root, const TCHAR *str, const TCHAR *key, int val)

{
	HKEY	Result;
	DWORD	oldtime = 0;
	DWORD	plen = sizeof(DWORD);

	LONG ret2 = RegOpenKey(root, str, &Result);

	if(ret2 != ERROR_SUCCESS)
		{
		/// Create key
		ret2 = RegCreateKeyEx(root, str, 0,
									NULL, 0, KEY_ALL_ACCESS, NULL, &Result, NULL);
		}

	if(ret2 == ERROR_SUCCESS)
		{
		RegSetValueEx(Result, key, NULL, REG_DWORD, 
								(unsigned char*)&val, sizeof(DWORD));
		RegCloseKey(Result);
		}
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::ObfuscateStr(CString &str)

{
	int len = str.GetLength();

	for(int loop = 0; loop < len; loop++)
		str.SetAt(loop, TCHAR(rand() % ('z' - 'a') + 'a'));	

	//P2N(_T("CSupport::ObfuscateStr '%s' \r\n"), str);		
}

//////////////////////////////////////////////////////////////////////////
// Generate random string, min lenght = mm max length = xx

void	CSupport::RandStr(int mm, int xx, CString &str)

{
	//P2N(_T("CSupport::RandStr(int mm=%d, int xx=%d, CString &str=%s)\r\n"), mm, xx, str);

	int lim; str = "";
	
	if(xx - mm > 0)
		{
		lim = (rand() % (xx - mm)) + mm;
		}
	else
		{
		lim = max(xx, mm);
		}
	for(int loop2 = 0; loop2 <  lim; loop2++)
		{
		str += TCHAR (rand() % ('z' - 'a') + 'a');	
		}
}

//////////////////////////////////////////////////////////////////////////

CString CSupport::Machine()

{
	CString str;

	TCHAR *mach = str.GetBuffer(MAX_COMPUTERNAME_LENGTH + 1); 
	DWORD len = MAX_COMPUTERNAME_LENGTH + 1;

	GetComputerName(mach, &len);
	str.ReleaseBuffer();

	return(str);
}


void	CSupport::NumAddCommas(CString &str) 

{
	CString tmp;

	//P2N(_T("addcomas() in='%s'  "), str);

	int len = str.GetLength();

	// Reverse string, add commas
	for(int loop = 0; loop < len ; loop++)
		{
		if(loop % 3 == 0 && loop != 0) 
			tmp += ',';

		//	if(loop < len - 1)
		//		
		
		tmp += str[len - 1 - loop];
		}

	// Get ready to reconstruct
	str = ""; len = tmp.GetLength();

	// Reverse back to original
	for(int loop2 = 0; loop2 < len ; loop2++)
		{
		str += tmp[len - 1 - loop2];
		}

	//P2N(_T("out='%s'\r\n"), str);
}

//////////////////////////////////////////////////////////////////////////
// Convert a double to a string. Add commas as convetional. 

void	CSupport::double2ascii(double num, CString &str, int prec, int commas, int radix)

{
	CString tmp;
	CString fra;

	str = "";				// init string for append operation

	int		prog = 0, digits = 0;
	double	intnum, frac, div = radix;

	// Convert fractional part
	frac = modf(num, &intnum);

#if 0
	// Fractianl part
	while(true)
		{
		double intnum;
		modf(frac/div, &intnum);

		fra += (char)(num - intnum * 10) + '0';

		P2N(_T("num %d %c\r\n"), (char)(num - intnum * 10),  (char)(num - intnum * 10) + '0');

		fra = intnum;

		if(fra == 0)
			break;
		}
#endif


	//Whole part
	while(true)
		{
		double intnum;
		modf(num/div, &intnum);

		if(digits++ % 3 == 0  && digits > 1 && commas)
			tmp +=  ',';

		TCHAR diff = (char)(num - intnum * div);

		if(diff > 10)
			diff += 'a';
		else
			diff += '0';

		tmp += diff;
		
		P2N(_T("num %d %c\r\n"), diff,  diff);

		num = intnum;

		if(num == 0)
			break;
		}

	int loop2 = 0;
	int len = tmp.GetLength();

	// Reverse string
	for(int loop = 0; loop < len ; loop++)
		{
		str += tmp[len - 1 - loop];
		}
}

/////////////////////////////////////////////////////////////////////////////
// Return estimated number of patches needed between strings

int CSupport::StrDiff(const CString &str1, const CString &str2)

{
    TCHAR chh1, chh2;
    TCHAR chh11, chh22;
    TCHAR chh33, chh44;
    int idx1 = 0, idx2 = 0;
    int idx11 = 0, idx22 = 0;
    int idx33 = 0, idx44 = 0;
    int len1 = str1.GetLength();
    int len2 = str2.GetLength();
    int diff = 0;

    //P2N(_T("strdiff - 1 : %s\r\n"), str1);
    //P2N(_T("strdiff - 2 : %s\r\n"), str2);

	if(!len1)
		return(len2);

	if(!len2)
		return(len1);

    while(TRUE)
    	{
        if(idx1 >= len1)
            break;

        if(idx2 >= len2)
            break;

        chh1 = str1.GetAt(idx1);
        chh2 = str2.GetAt(idx2);

        if(chh1 != chh2)
        {
        int diff2 = 0;
        int diff3 = 0;
        int diff4 = 0;

        idx11 = idx1;
        idx22 = idx2;
        idx33 = idx1;
        idx44 = idx2;

        // try str1, walk str2
        while(TRUE)
            {
            if(idx22 >= len2)
                break;

            chh22 = str2.GetAt(idx22);
            if(chh1 == chh22)
                break;

            diff2++;
            idx22++;
            }
        // try str2, walk str1
        while(TRUE)
            {
            if(idx11 >= len1)
                break;

            chh11 = str1.GetAt(idx11);
            if(chh11 == chh2)
                break;

            diff3++;
            idx11++;
            }

        // Walk both
        while(TRUE)
            {
            if(idx33 >= len1)
                break;

            if(idx44 >= len2)
                break;

            chh33 = str1.GetAt(idx33);
            chh44 = str2.GetAt(idx44);
            if(chh33 == chh44)
                break;

            diff4++;
            idx33++;
            idx44++;
            }

        //P2N(_T("diff2 =  %d diff3 = %d diff4 = %d\r\n"),
        //               	     diff2, diff3, diff4);

        // Assume closest match
        if(diff2 < diff3)
            {
            if(diff4 < diff2)
                {
                idx1 = idx33;
                idx2 = idx44;
                diff += diff4;
                }
            else
                {
                idx2 = idx22;
                diff += diff2;
                }
            }
        else
            {
            if(diff4 < diff3)
                {
                idx1 = idx33;
                idx2 = idx44;
                diff += diff4;
                }
            else
                {
                idx1 = idx11;
                diff += diff3;
                }
            }
        }
        idx1++;
        idx2++;
    }
    //P2N(_T("strdiff - ret : %d\r\n"), diff);
    return(diff);
}

//////////////////////////////////////////////////////////////////////////

void CSupport::ShutDownComputer(UINT command)

{	  
	//	OSVERSIONINFO osVersion;  GetVersionEx(&osVersion);

	HANDLE hToken;              // handle to process token 
	TOKEN_PRIVILEGES tkp;       // pointer to token structure 

	OpenProcessToken(GetCurrentProcess(), 
	  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) ;

	// Get the LUID for shutdown privilege. 

	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
	  &tkp.Privileges[0].Luid); 

	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	// Get shutdown privilege for this process. 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
		  (PTOKEN_PRIVILEGES) NULL, 0); 

	int nArgument = EWX_POWEROFF;
	switch(command)
		{  
		case 0: nArgument = EWX_POWEROFF; break;
		case 1: nArgument = EWX_REBOOT;   break;
		case 2: nArgument = EWX_LOGOFF;   break;
		case 3: nArgument = EWX_SHUTDOWN; break;
		}

	ExitWindowsEx(nArgument, 0);
	//EndDialog(IDOK);
}

//////////////////////////////////////////////////////////////////////////
// Get next image file name 

int		CSupport::NextImgFileName(CString &fname)

{
	//AP2N(_T("CWavfragDlg::NextImgFileName(CString &fname) %s\r\n"), fname);

	int ret = 0;
	CString lid(lastimg); support.PathToDir(lid);
	CString name(lastimg); support.PathToFname(name);

	char	olddir[_MAX_PATH]; _getcwd(olddir, sizeof(olddir));

	if(_tchdir(lid) < 0)
		{
		AP2N(_T("CWavfragDlg::NextImgFileName Cannot change dir %s\r\n"), lid);		
		return ret;
		}

	struct _tfinddata_t c_file; long hFile;	int cnt = 0, menext = 0;

	if ((hFile = _tfindfirst(_T("*.*"), &c_file)) == -1L)
	{
		P2N(_T("Cannot execute findfirst\r\n"));
		goto endd;
	}

	while(true)
		{
		P2N( _T(" %-12s %.24s  %9ld\r\n"),
			c_file.name, ctime( &( c_file.time_write ) ), c_file.size );

		CString xname = c_file.name; support.PathToExt(xname);	
		
		// Filter out images
		if(	_tcsicmp(xname, _T("jpg")) == 0 ||
			_tcsicmp(xname, _T("gif")) == 0 ||
			_tcsicmp(xname, _T("bmp")) == 0 ||
			_tcsicmp(xname, _T("png")) == 0 )
			{
			//AP2N(_T("image file %s\r\n"), c_file.name);

			if(menext)
				{				
				fname = lid; fname += c_file.name;
				AP2N(_T("Found next %s\r\n"), fname);
				
				ret = true;
				break;
				}

			// Signal we found me
			if(name == c_file.name)
				{
				//AP2N(_T("Found me! %s\r\n"), fname);
				menext = true;
				}
			}

		if( _tfindnext( hFile, &c_file ) != 0 )
			break;

		cnt++;
		}

endd:
	_findclose( hFile );
	_chdir(olddir);

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// Get next image file name 

int		CSupport::PrevImgFileName(CString &fname)

{
	//AP2N(_T("CWavfragDlg::PrevImgFileName(CString &fname) %s\r\n"), fname);

	int ret = 0;
	CString lid(lastimg);	support.PathToDir(lid);
	CString name(lastimg);	support.PathToFname(name);

	TCHAR	olddir[_MAX_PATH]; _tgetcwd(olddir, sizeof(olddir));

	if(_tchdir(lid) < 0)
		{
		AP2N(_T("CWavfragDlg::PrevImgFileName Cannot change dir %s\r\n"), lid);		
		return ret;
		}

	struct _tfinddata_t c_file;
	long hFile;

	CString oldname; int cnt = 0, menext = 0;
	
	if( (hFile = _tfindfirst(_T("*.*"), &c_file )) == -1L )
		goto endd;

	while(true)
		{
		//AP2N( " %-12s %.24s  %9ld\r\n",
		//	c_file.name, ctime( &( c_file.time_write ) ), c_file.size );

		CString xname = c_file.name; support.PathToExt(xname);	
		
		// Filter out images
		if(	_tcsicmp(xname, _T("jpg")) == 0 ||
			_tcsicmp(xname, _T("gif")) == 0 ||
			_tcsicmp(xname, _T("bmp")) == 0 ||
			_tcsicmp(xname, _T("png")) == 0 )
			{
			//AP2N(_T("image file %s\r\n"), c_file.name);

			// Signal we found me
			if(name == c_file.name)
				{
				AP2N(_T("Found me! %s\r\n"), fname);

				if(oldname != "")
					{
					fname = lid; fname += oldname;
					//AP2N(_T("Found next %s\r\n"), fname);
				
					ret = true;
					}
				break;				
				}
			oldname = c_file.name;
			}

		if( _tfindnext( hFile, &c_file ) != 0 )
			break;

		cnt++;
		}

endd:
	_findclose( hFile );
	_tchdir(olddir);

	return ret;
}

//////////////////////////////////////////////////////////////////////////

CString	CSupport::NumPad(int num, int newlen)

{
	CString ret;
	
	TCHAR *buff = ret.GetBuffer(_MAX_PATH + 1);
	_itot_s(num, buff, _MAX_PATH, 10);
	
	ret.ReleaseBuffer();

	int oldlen = ret.GetLength();

	for(int loop = oldlen; loop < newlen; loop++)
		{
		ret = _T(" ") + ret;
		}

	return ret;
}

//////////////////////////////////////////////////////////////////////////

void	CSupport::StrPad(CString *str, int newlen)

{
	int oldlen = str->GetLength();

	for(int loop = oldlen; loop < newlen; loop++)
		{
		*str += " ";
		}
}