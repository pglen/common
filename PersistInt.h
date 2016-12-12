
/* =====[ PersistInt.h ]========================================== 
                                                                             
   Description:     The CryptoSticky project, implementation of the PersistInt.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  6/4/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

// PersistInt.h: interface for the PersistInt class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSISTINT_H__C0DD613D_DDAE_4E8F_9EE8_35C7589ADAAE__INCLUDED_)
#define AFX_PERSISTINT_H__C0DD613D_DDAE_4E8F_9EE8_35C7589ADAAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mxpad.h"
#include "xPtrArr.h"


class PersistInt
  
{
	static	CxPtrArr	*glob;

public:

	static	void Load();						// Call this at init app time
	
	PersistInt(const TCHAR *_dir, const TCHAR *_name, int _def = 0);
	virtual ~PersistInt();
	
	operator int& () 
		{
		//P2N(_T("PersistInt operator int&\r\n"));
		if(!initr)
			{
			initr = true;
			val = AfxGetApp()->GetProfileInt(dir, name, def);
			}
		return val;
		}

	int operator = (int vv)
		{
		//P2N(_T("PersistInt operator =int\r\n"));
		val = vv;
		AfxGetApp()->WriteProfileInt(dir, name, val);

		return val;
		}

	void	PersistInt::Default();

protected:

	int		val;

	CString name, dir;
	int		def;

private:
	BOOL	initr, initw;
};

#endif // !defined(AFX_PERSISTINT_H__C0DD613D_DDAE_4E8F_9EE8_35C7589ADAAE__INCLUDED_)
