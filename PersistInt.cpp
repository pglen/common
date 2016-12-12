
/* =====[ PersistInt.cpp ]========================================== 
                                                                             
   Description:     The CryptoSticky project, implementation of the PersistInt.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  6/3/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include <stdafx.h>
#include "PersistInt.h"
#include "xPtrArr.h"

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[]=_T(__FILE__);
#define new DEBUG_NEW
#endif

#define DEBUG_NEW new

CxPtrArr	*PersistInt::glob = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PersistInt::PersistInt(const TCHAR *_dir, const TCHAR *_name, int _def)

{
	initr = initw = 0;

	dir			= _dir;	
	name		= _name;
	val  = def	= _def;
	void * ptr = NULL;

	if (!glob)
		glob = new CxPtrArr ;
	//1, __FILE__, __LINE__)

	glob->Add(this);
}

PersistInt::~PersistInt()

{
	//TRACE(_T("PersistInt::~PersistInt()\r\n"));

	int alen = glob->GetSize();
	for(int loop = 0; loop < alen; loop++)
		{
		PersistInt *ptr =  (PersistInt*)glob->GetAt(loop);

		if(ptr == this)
			{
			glob->RemoveAt(loop);
			break;
			}
		}

	if(glob->GetSize() == 0)
		{
		delete glob;
		glob = NULL;
		}
}

void	PersistInt::Default()

{
	val = def;
}

void PersistInt::Load()

{
	//P2N(_T("PersistInt::Load()\r\n"));

	int alen = glob->GetSize();
	for(int loop = 0; loop < alen; loop++)
		{
		PersistInt *ptr =  (PersistInt*)glob->GetAt(loop);
		ptr->val = ::AfxGetApp()->GetProfileInt(ptr->dir, ptr->name, ptr->def);
		ptr->initr = true;

		//P2N(_T("PersistInt::Load %s=%d\r\n"), ptr->name, ptr->val);
 		}
}
