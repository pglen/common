
/* =====[ PersistStr.h ]========================================== 
                                                                             
   Description:     The CryptoSticky project, implementation of the PersistStr.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  6/4/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

// PersistStr.h: interface for the PersistStr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSISTSTR_H__603CEC1D_891F_4920_9A16_3BA674D79F5B__INCLUDED_)
#define AFX_PERSISTSTR_H__603CEC1D_891F_4920_9A16_3BA674D79F5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mxpad.h"

class PersistStr  

{
	static	CPtrArray	*glob;

public:

	static	void Load();						// Call this at init app time

	PersistStr(const TCHAR *_dir, const TCHAR *_name, const TCHAR * _def = 0);

	virtual ~PersistStr();

	operator const TCHAR * ()
		{
		//P2N(_T("PersistStr operator const TCHAR *\r\n"));
	
		if(!initr)
			{
			initr = true;
			val = AfxGetApp()->GetProfileString(dir, name, def);
			}

		//return (const TCHAR *)val;
		return val;
		}

	operator CString ()
		{
		//P2N(_T("PersistStr operator CString \r\n"));
	
		if(!initr)
			{
			initr = true;
			val = AfxGetApp()->GetProfileString(dir, name, def);
			}
		return val;
		}

	CString & operator = (CString ss)
		{
		//P2N(_T("PersistStr operator = PersistStr\r\n"));
		val = ss;
		AfxGetApp()->WriteProfileString(dir, name, val);

		return val;
		}

	CString & operator += (CString ss)
		{
		//P2N(_T("PersistStr operator = PersistStr\r\n"));
		val += ss;
		AfxGetApp()->WriteProfileString(dir, name, val);

		return val;
		}

protected:

	CString	val, name, dir, def;

private:
	BOOL	initr, initw;
};

#endif // !defined(AFX_PERSISTSTR_H__603CEC1D_891F_4920_9A16_3BA674D79F5B__INCLUDED_)
