
/* =====[ GetText.cpp ]========================================================== 
                                                                             
   Description:     The wavfrag project, implementation of the GetText.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  9/29/2010  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
//#include "wavfrag.h"
#include "GetText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGetText dialog


CGetText::CGetText(CWnd* pParent /*=NULL*/)
	: CDialog(CGetText::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetText)
	m_edit = _T("");
	//}}AFX_DATA_INIT
}

void CGetText::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetText)
	DDX_Text(pDX, ID_EDIT_GETTEXT, m_edit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGetText, CDialog)
	//{{AFX_MSG_MAP(CGetText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetText message handlers

BOOL CGetText::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if(m_title != "")
		SetWindowText(m_title);

	HICON icon = AfxGetApp()->LoadStandardIcon(IDI_ASTERISK);
	
	((CStatic *)GetDlgItem(IDC_ICON1))->SetIcon(icon);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
