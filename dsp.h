
/* =====[ dsp.h ]========================================== 
                                                                             
   Description:     The daco project, implementation of the dsp.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  8/24/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#if !defined(AFX_DSP_H__AD131BE1_33F4_4AB2_B9EA_1316AE8A910E__INCLUDED_)
#define AFX_DSP_H__AD131BE1_33F4_4AB2_B9EA_1316AE8A910E__INCLUDED_

#include <mmsystem.h>
#include <mmreg.h>

#pragma comment(lib, "winmm.lib")

#define	CDsp_Magic	0xef3432ff



class CDsp

{

public:

	CDsp();

	int		magic;

	int		LoadWave(const TCHAR *fname, char **buff, int *len, WAVEFORMATEX *pwfx);
	int		SaveWave(const TCHAR *fname, void *buff, int len, WAVEFORMATEX *pwfx = NULL);
	
	void	SplitWave(short *dorg, int len, short *lptr, short *rptr, int lim);
	void	SplitWaveLeft(short *dorg, int len, short *lptr, int lim);
	void	UnSplitWave(short *lptr, short *rptr, int len, short *dorg);
	void	NormalizeWav(char *buff, int len, char *out);

	class CSupImp

{

public:

typedef struct tagSupImp

{
	short *acc;
	int   alen;
	int   adamp;

	short *add;
	int   dlen;
	int   ddamp;
} _SupImp;

// data

	_SupImp si;

//////////////////////////////////////////////////////////////////////
// Add two buffers together

void SuperImpose(_SupImp *si)

{
	short *acc =	si->acc;
	int alen =  	si->alen;
	short *add =	si->add;
	int dlen =		si->dlen;

	while(TRUE)
	{
		//WORD sum;

		if(!alen)
			break;
		if(!dlen)
			break;

		//sum = *acc + *add;
		// Catch clipping
		//if(sum > *acc && sum > *add)
		//	*acc = sum;

		*acc = (*acc * si->adamp)/100 +
					(*add * si->ddamp)/100;

		acc++; add++; alen--; dlen--;
	}
}

};


protected:

	int CDsp::xWriteField(HMMIO hFile, const char *four, const char *str);

};

extern CDsp dsp;

#endif


