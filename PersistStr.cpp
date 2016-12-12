// PersistStr.cpp: implementation of the PersistStr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PersistStr.h"

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[]=_T(__FILE__);
#define new DEBUG_NEW
#endif

#define DEBUG_NEW new

CPtrArray	*PersistStr::glob = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PersistStr::PersistStr(const TCHAR *_dir, const TCHAR *_name, const TCHAR * _def)

{
	initr = initw = 0;

	dir			= _dir;	
	name		= _name;
	val = def	= _def;	

	if(!glob)
		glob =  new CPtrArray;

	glob->Add(this);
}

//////////////////////////////////////////////////////////////////////////

PersistStr::~PersistStr()

{
	//TRACE(_T("PersistStr::~PersistStr()\r\n"));

	int alen = glob->GetSize();
	for(int loop = 0; loop < alen; loop++)
		{
		PersistStr *ptr =  (PersistStr*)glob->GetAt(loop);

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

void PersistStr::Load()

{
	//P2N(_T("PersistStr::Load()\r\n"));

	int alen = glob->GetSize();
	for(int loop = 0; loop < alen; loop++)
		{
		PersistStr *ptr =  (PersistStr*)glob->GetAt(loop);
		ptr->val = ::AfxGetApp()->GetProfileString(ptr->dir, ptr->name, ptr->def);
		ptr->initr = true;

		//P2N(_T("PersistStr::Load %s='%s'\r\n"), ptr->name, ptr->val);
 		}
}
