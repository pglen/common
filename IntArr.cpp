
/* =====[ CIntArr.cpp ]========================================== 
                                                                             
   Description:     The daco project, implementation of the CIntArr.cpp                
                                                                             
                    This does not use any operation other than integer
					operations.
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  10/5/2009  Peter Glen   Initial version.                         
      0.00  3/10/2012  Peter Glen   Moved away math dependencies                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include "IntArr.h"
#include "mxpad.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIntArr::CIntArr()

{
	//AAP2N("CIntArr::CIntArr() %p\r\n", this);

	xInit();

}

//////////////////////////////////////////////////////////////////////////

CIntArr::CIntArr(int inisize)

{
	//AAP2N("CIntArr::CIntArr() %p\r\n", this);

	xInit();
	SetSize(inisize);
}

//////////////////////////////////////////////////////////////////////////

CIntArr::CIntArr(CIntArr *org)

{
	//AAP2N("CIntArr::CIntArr() %p\r\n", this);

	xInit();

	Copy(org);
}



CIntArr::~CIntArr()

{
	ASSERT(magic == CIntArr_Magic);

	//AP2N("CIntArr::&CIntArr() %p\r\n", this);

	delete[] (BYTE*)m_pData;
}

// -----------------------------------------------------------------------

/// \brief Remove pointer by value. 

/// Will not continue after the first item is removed. 
/// (aka: will not delete a duplicate item)\n
/// Optimized to start from the end, as it assumes most likely scenario of
/// last in - first out)
	
void CIntArr::RemoveValue(int val)

{
	ASSERT(magic == CIntArr_Magic);

	int size = GetSize();
	for(int loop = size - 1; loop >= 0; loop--)
		{
		if(GetAt(loop) == val)
			{	
			//AP2N("CIntArr::Removeval removed %p\r\n", val);
			RemoveAt(loop);
			}
		}
}

/// \brief Add a pointer only if it is unique

/// Adds new entry if it is unique. (aka not in the array already)
/// Searches from the end, assuming LIFO (last in first out)

int CIntArr::AddUniq(const int val)

{
	ASSERT(magic == CIntArr_Magic);

	int ret = -1, was = 0, size = GetSize();

	for(int loop = size - 1; loop >= 0; loop--)
		{
		if(GetAt(loop) == val)
			{	
			was = true;
			break;
			}
		}
	if(!was)
		{
		//AP2N("CIntArr::AddUniq added %p\r\n", val);
		ret = Add(val);
		}	
	return ret;
}


/// \brief Find the first idx that matches val

/// Find existing (stored) pointer, return index to it if found. Return -1 if not found

int CIntArr::Find(const int val)

{
	ASSERT(magic == CIntArr_Magic);

	//AP2N("CIntArr::Find %d\r\n", val);

	int ret = -1, size = GetSize();

	for(int loop = size - 1; loop >= 0; loop--)
		{
		if(GetAt(loop) == val)
			{	
			//AP2N("CIntArr::Find idx=%d\r\n", loop);
			ret = loop;
			break;
			}
		}
	return ret;
}

// --------------------------------------------------------------
//
/// \brief   Set Size of array grow if needed 
/// 
/// \details Grows/Truncates the array as needed
/// \n  
/// Return type:      void 
/// \n  
/// \param nNewSize Arg Type: int
/// \param nGrowBy Arg Type: int optional argument

void CIntArr::SetSize(int nNewSize, int nGrowBy)

{
	ASSERT(magic == CIntArr_Magic);

	ASSERT(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		delete[] (int*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
		m_pData = (int*) new BYTE[nNewSize * sizeof(int)];

		memset(m_pData, 0, nNewSize * sizeof(int));  // zero fill

		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements

			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(int));

		}

		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = min(1024, max(4, m_nSize / 8));
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
		int* pNewData = (int*) new BYTE[nNewMax * sizeof(int)];

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(int));

		// construct remaining elements
		ASSERT(nNewSize > m_nSize);

		memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(int));

		// get rid of old stuff (note: no destructors called)
		delete[] (int*)m_pData;

		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

//////////////////////////////////////////////////////////////////////////

int CIntArr::Append(const CIntArr& src)

{
	ASSERT(magic == CIntArr_Magic);

	ASSERT(this != &src);   // cannot append to itself

	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);

	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(int));

	return nOldSize;
}

//////////////////////////////////////////////////////////////////////////

void CIntArr::Copy(const CIntArr& src)

{
	
	ASSERT(magic == CIntArr_Magic);

	ASSERT(this != &src);   // cannot copy to itself

	SetSize(src.m_nSize);

	memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(int));
}

//////////////////////////////////////////////////////////////////////////

void CIntArr::Copy(CIntArr *src)

{
	
	ASSERT(magic == CIntArr_Magic);

	ASSERT(this != src);   // cannot copy to itself

	SetSize(src->m_nSize);

	memcpy(m_pData, src->m_pData, src->m_nSize * sizeof(int));
}

/////////////////////////////////////////////////////////////////////////
/// Shrink to desired size
///
/// Frees any extra memory that was allocated while the array was grown. 
/// This function has no effect on the size or upper bound of the array.
		
void CIntArr::FreeExtra()

{
	if (m_nSize != m_nMaxSize)
		{	
		int* pNewData = NULL;

		if (m_nSize != 0)
			{
			pNewData = (int*) new BYTE[m_nSize * sizeof(int)];
		
			// Replicate Data
			memcpy(pNewData, m_pData, m_nSize * sizeof(int));
			}

		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;	m_nMaxSize = m_nSize;
		}
}

/////////////////////////////////////////////////////////////////////////////

void CIntArr::SetAtGrow(int nIndex, int newElement)

{
	ASSERT(magic == CIntArr_Magic);

	ASSERT(nIndex >= 0);

	//AP2N("CIntArr::SetAtGrow index=%d\r\n", nIndex);

	if (nIndex >= m_nSize)
		SetSize(nIndex + 1);

	m_pData[nIndex] = newElement;
}


void CIntArr::InsertAt(int nIndex, int newElement, int nCount)
{
	ASSERT(magic == CIntArr_Magic);
	
	ASSERT(nIndex >= 0);    // will expand to meet need
	ASSERT(nCount > 0);     // zero or negative size not allowed

	if (nIndex >= m_nSize)
		{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
		}
	else
		{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount);  // grow it to new size
		// shift old data up to fill gap
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(int));

		// re-init slots we copied from

		memset(&m_pData[nIndex], 0, nCount * sizeof(int));
		}

	// insert new value in the gap
	ASSERT(nIndex + nCount <= m_nSize);

	// copy elements into the empty space
	while (nCount--)
		m_pData[nIndex++] = newElement;

}


void CIntArr::RemoveAt(int nIndex, int nCount)

{	
	ASSERT(nIndex >= 0);	ASSERT(nCount >= 0);
	
	//ASSERT(nIndex + nCount <= m_nSize);

	if(nIndex + nCount >= m_nSize)
		{
		//AP2N("CIntArr::RemoveAt Removing from past the end\r\n");
		
		nCount = (m_nSize - nIndex) - 1;

		if(nCount <= 0)
			{
			//AP2N("CIntArr::RemoveAt Removing negative count\r\n");
			return;
			}
		}

	//AP2N("CIntArr::RemoveAt index=%d\r\n", nIndex);

	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(int));
	m_nSize -= nCount;
}

void CIntArr::InsertAt(int nStartIndex, CIntArr* pNewArray)

{	
	ASSERT(pNewArray != NULL);
	ASSERT(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());

		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}

//////////////////////////////////////////////////////////////////////////

void CIntArr::Dump()

{
	AP2N(
	"CIntArr::Dump data=%p m_nSize=%d m_nMaxSize=%d m_nGrowBy=%d\r\n",
					m_pData, m_nSize, m_nMaxSize, m_nGrowBy);
}


//////////////////////////////////////////////////////////////////////////

void CIntArr::DumpAll(const char *str)

{
	if(str)
		AP2N("Dumping %s %p\r\n", str, this);

	for(int loop = 0; loop < m_nSize; loop++)
		{
		AP2N("%d  ", m_pData[loop]);
		}
	
	if(str)
		AP2N("\r\nEnd Dumping %p\r\n", this);
	else
		AP2N("\r\n");
}

//////////////////////////////////////////////////////////////////////////
// The len is number of ints

int		CIntArr::Add(int *buff, int len)

{
	SetSize(m_nSize + len);

	memcpy(m_pData + m_nSize, buff, len * sizeof(int));	

	return(m_nSize + len);
}
	
//////////////////////////////////////////////////////////////////////////

#ifdef PLOT

int		CIntArr::Plot(CPlot *pplot)

{
	ASSERT(magic == CIntArr_Magic);

	for(int loopd = 0; loopd < GetSize(); loopd++)
		pplot->AddLine(GetAt(loopd));

	return 0;
}

#endif


//////////////////////////////////////////////////////////////////////////

void	CIntArr::Plus(int add)

{
	//ASSERT(normto > 0);

	if(GetSize() == 0)
		{
		//AP2N("CIntArr::Multiply empty array\r\n");
		return;
		}

	int loop, asize = GetSize();
	if(!asize)
		return;

	for(loop = 0; loop < asize; loop++)
		{
		int val = GetAt(loop);
		SetAt(loop, val + add);		
		}

	return;
}



// -----------------------------------------------------------------------
//
/// \brief  CIntArr::RunAve  Avarage array by summing with previous
/// 
/// \details CIntArr::Smooth Special arrangement made to correctly 
///						calculate first/last neighbours
/// \n  
/// Return type:      
/// \n  
/// \param passes Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::RunAve(const int passes)

{
	int nlen = GetSize();

	if(nlen <= 1)
		{
		//AP2N("CIntArr::SmoothArr empty array\r\n");
		return;
		}

	for(int loop3 = 0; loop3 < passes; loop3++)
		{
		int val2, val = GetAt(0);
			
		for(int loop = 1; loop < nlen; loop++)
			{
			val2 = GetAt(loop);						
			SetAt(loop, (val + val2) / 2);
			val = val2;
			}
		}

	return;
}


// -----------------------------------------------------------------------
//
/// \brief  CIntArr::EdgeDetect  Output edges to another array
/// 
/// \details CIntArr::EdgeDetect Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param *res Arg Type: CIntArr 
//
// -----------------------------------------------------------------------

void	CIntArr::EdgeDetect(CIntArr *res)

{
	int asize = GetSize();

	res->RemoveAll();

	int old = 0, prev = 0, old_trend = 0;

	// Iterate for second gen
	for(int aloop = 0; aloop < asize; aloop++)
		{
		int curr = GetAt(aloop);
		
		// Observe trend change
		short	trend = old_trend, ddiff = prev - curr;

		if(ddiff > 0) trend = 1;
		else if(ddiff < 0) trend = -1;
	
		if((old_trend != trend))
			{
			old_trend = trend;
		
			//res->Add(abs(curr - old));
			//res2->Add(curr - old);
			
			//if(trend == 1)
			res->Add(aloop);
			
			//old = aloop;
			old = curr;
			}

		prev = curr;	
		}
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::EdgeScript  Brief Here 
/// 
/// \details CIntArr::EdgeScript Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param *dist Arg Type: CIntArr 
/// \param *val Arg Type: CIntArr 
//
// -----------------------------------------------------------------------

void	CIntArr::EdgeScript(CIntArr *dist, CIntArr *val)

{
	int asize = GetSize();
	if(asize == 0)
		{
		AP2N("CIntArr::EdgeScript - Empty array\r\n"); return;
		}
	dist->RemoveAll(); val->RemoveAll();
	
	// Iterate for second gen
	int old = 0, oldloop = 0, prev = 0, old_trend = 0;
	for(int aloop = 0; aloop < asize; aloop++)
		{
		int curr = GetAt(aloop);
		
		// Observe trend change
		short	trend = old_trend, ddiff = prev - curr;

		if(ddiff > 0) trend = 1;
		else if(ddiff < 0) trend = -1;
	
		if((old_trend != trend))
			{
			old_trend = trend;
			
			//if(trend == 1)
				{
				//val->Add(old);
				for(int loopx = 0; loopx < aloop - oldloop; loopx++)
					val->Add(curr);

				dist->Add(aloop - oldloop);

				oldloop = aloop;
				old	= curr;
				}			
			}

		prev = curr;	
		}
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::TrimPoints  Mark trim points for array
/// 
/// \details CIntArr::TrimPoints: outputs beginning offset and end offset
/// \n  
/// Return type:      
/// \n  
/// \param treshold Arg Type: int 
/// \param *begp Arg Type: int 
/// \param *endp Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::TrimPoints(int treshold, int *begp, int *endp)

{
	*begp = *endp = 0;

	int nlen = GetSize();

	if(!nlen)
		return;

	// Find boundaries
	for(int bloop = 0; bloop < nlen - 1; bloop++)
		{
		if(GetAt(bloop) > treshold)
			break;
		}
	
	for(int eloop = nlen - 1; eloop  >= 0; eloop--)
		{
		if(GetAt(eloop) > treshold)
			break;
		}

	*begp = min(bloop, nlen);
	*endp = min(eloop, nlen);
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::Trim  Trim array to compansate for silence
/// 
/// \details CIntArr::Trim Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param treshold Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::Trim(int treshold)

{
	int nlen = GetSize();

	if(!nlen)
		return;

	// Find boundaries
	for(int bloop = 0; bloop < nlen - 1; bloop++)
		{
		if(GetAt(bloop) > treshold)
			break;
		}
	
	for(int eloop = nlen - 1; eloop  >= 0; eloop--)
		{
		if(GetAt(eloop) > treshold)
			break;
		}

	//AP2N("norm beg=%d end=%d (size=%d)\r\n",  bloop, eloop, nlen);
	SetSize(eloop);
	RemoveAt(0, bloop);	
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::Trim  Brief Here 
/// 
/// \details CIntArr::Trim Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param treshold Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::Trim(int start, int endd)

{
	if(endd < GetSize() && endd >= 0)
		SetSize(endd);

	if(start < GetSize()  && start
		>= 0)
		RemoveAt(0, start);	
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::FindTrim  Brief Here 
/// 
/// \details CIntArr::FindTrim Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param *start Arg Type: int 
/// \param *end Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::FindTrim(int *start, int *end)

{
	int maxx = 0, nlen = GetSize();

	for(int xloop = 0; xloop < nlen; xloop++)
		{
		if(GetAt(xloop) > maxx )
			maxx = GetAt(xloop);
		}

	int treshold = maxx / 20;
	for(int bloop = 0; bloop < nlen ; bloop++)
		{
		if(GetAt(bloop) > treshold)
			break;
		}

	*start = bloop;

	for(int eloop = nlen - 1; eloop  >= 0; eloop--)
		{
		if(GetAt(eloop) > treshold)
			break;
		}

	*end = eloop;
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::FindMax  Find maximum index and value
/// 
/// \details CIntArr::FindMax Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param *idx Arg Type: int 
/// \param *val Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::FindMax(int *idx, int *val)

{
	int	maxval = INT_MIN, maxidx = -1, alen = GetSize();

	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		int val = GetAt(loop3);
		if(maxval < val)
			{
			maxval = val; maxidx = loop3;
			}
		}

	if(idx)
		*idx = maxidx;

	if(val)
		*val = maxval;
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::FindMin  Brief Here 
/// 
/// \details CIntArr::FindMin Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param *idx Arg Type: int 
/// \param *val Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::FindMin(int *idx, int *val)

{
	int	minval = INT_MAX, maxidx = -1, alen = GetSize();

	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		int val = GetAt(loop3);
		if(minval > val)
			{
			minval = val; maxidx = loop3;
			}
		}

	*idx = maxidx;
	*val = minval;
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::FindMax2  Find the two maximal integers
/// 
/// \details CIntArr::FindMax2 Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param *pidx1 Arg Type: int 
/// \param *val1 Arg Type: int 
/// \param *pidx2 Arg Type: int 
/// \param *val2 Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::FindMax2(int *pidx1, int *val1, int *pidx2, int *val2)

{
	int	maxval1 = INT_MIN, maxval2 = INT_MIN;
	int idx1 = -1, idx2 = -1, alen = GetSize();

	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		int val = GetAt(loop3);
		if(maxval1 < val)
			{
			// Push down 
			maxval2 = maxval1; idx2 = idx1; 
			maxval1 = val; idx1 = loop3;
			}
		else if(maxval2 < val)
			{
			// Assign it
			idx2 = loop3; maxval2 = val; 
			}
		}

	*pidx1 = idx1; *val1 = maxval1;
	*pidx2 = idx2; *val2 = maxval2;
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::FindMax3  Find the two maximal integers
/// 
/// \details CIntArr::FindMax3 Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param *pidx1 Arg Type: int 
/// \param *val1 Arg Type: int 
/// \param *pidx2 Arg Type: int 
/// \param *val2 Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::FindMax3(int *pidx1, int *val1, int *pidx2, int *val2, int *pidx3, int *val3)

{
	int	maxval1 = INT_MIN, maxval2 = INT_MIN, maxval3 = INT_MIN;
	int idx1 = -1, idx2 = -1, idx3 = -1, alen = GetSize();

	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		int val = GetAt(loop3);
		if(maxval1 < val)
			{
			// Push down 
			maxval3 = maxval2;	idx3 = idx2; 
			maxval2 = maxval1;	idx2 = idx1; 
			maxval1 = val;		idx1 = loop3;
			}
		else if(maxval2 < val)
			{
			// Push down 
			maxval3 = maxval2;	idx3 = idx2; 
			maxval2 = val;		idx2 = loop3; 
			}
		else if(maxval3 < val)
			{
			// Assign it
			maxval3= val; 		idx3 = loop3; 
			}
		}

	*pidx1 = idx1; *val1 = maxval1;
	*pidx2 = idx2; *val2 = maxval2;
	*pidx3 = idx3; *val3 = maxval3;
}

//////////////////////////////////////////////////////////////////////////
// Sort array by value. Array of indices on output. Bubble sort.

void	CIntArr::Sort(CIntArr *parr, int reverse)

{
	//parr->SetSize(GetSize());

	parr->RemoveAll();

	// Create linear index
	for(int loop = 0; loop < GetSize(); loop++)
		{
		parr->Add(loop);
		}

	int pass = 0;

	// Bubble sort
	while(true)
		{
		int need = false;

		//AP2N("pass %d\r\n", pass++);  // count passes (dev)

		for(int loop2 = 0; loop2 < GetSize() - 1; loop2++)
			{
			int swap = false;

			int idx  = parr->GetAt(loop2);
			int idx2 = parr->GetAt(loop2+1);
				
			if(reverse)
				{
				if(GetAt(idx2) < GetAt(idx))
					{
					swap = true;
					}
				}
			else
				{	
				if(GetAt(idx2) > GetAt(idx))
					swap = true;
				}

			// If out of order, swap it				
			if(swap)
				{
				parr->SetAt(loop2,	idx2);
				parr->SetAt(loop2+1, idx);
				need = true;
				}
			}

		if(!need)
			break;    // we are done here
		}
}

//////////////////////////////////////////////////////////////////////////

void	CIntArr::FindMax4(int *pidx1, int *val1, int *pidx2, int *val2, 
						  int *pidx3, int *val3, int *pidx4, int *val4)

{
	int	maxval1 = INT_MIN, maxval2 = INT_MIN;
	int	maxval3 = INT_MIN, maxval4 = INT_MIN;

	int idx1 = -1, idx2 = -1, idx3 = -1, idx4 = -1;
	int	alen = GetSize();

	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		int val = GetAt(loop3);
		if(maxval1 < val)
			{
			// Push down 
			maxval4 = maxval3;	idx4 = idx3; 
			maxval3 = maxval2;	idx3 = idx2; 
			maxval2 = maxval1;	idx2 = idx1; 
			maxval1 = val;		idx1 = loop3;
			}
		else if(maxval2 < val)
			{
			// Push down 
			maxval4 = maxval3;	idx4 = idx3; 
			maxval3 = maxval2;	idx3 = idx2; 
			maxval2 = val;		idx2 = loop3; 
			}
		else if(maxval3 < val)
			{
			// Assign it
			maxval4 = maxval3;	idx4 = idx3; 
			maxval3= val; 		idx3 = loop3; 
			}
		else if(maxval4 < val)
			{
			// Assign it
			maxval4 = val;	idx4 = loop3; 			
			}
		}

	*pidx1 = idx1; *val1 = maxval1;
	*pidx2 = idx2; *val2 = maxval2;
	*pidx3 = idx3; *val3 = maxval3;
	*pidx4 = idx4; *val4 = maxval4;
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::FindMin2  Find the two minimal integers
/// 
/// \details CIntArr::FindMin2 Descibe Here 
/// \n  
/// Return type:      
/// \n  
/// \param *pidx1 Arg Type: int 
/// \param *val1 Arg Type: int 
/// \param *pidx2 Arg Type: int 
/// \param *val2 Arg Type: int 
//
// -----------------------------------------------------------------------

void	CIntArr::FindMin2(int *pidx1, int *val1, int *pidx2, int *val2)

{
	int	minval = INT_MAX, minval2 = INT_MAX, alen = GetSize();
	int idx1 = -1, idx2 = -1;

	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		int val = GetAt(loop3);
		if(minval > val)
			{
			// Push down 
			minval2 = minval; minval = val; 
			idx2 = idx1; idx1 = loop3;
			}
		else if(minval2 > val)
			{
			idx2 = loop3; minval2 = val; 
			}
		}

	*val1 = minval;	*val2 = minval2;
	*pidx1 = idx1;  *pidx2 = idx2;
}

//////////////////////////////////////////////////////////////////////////
//

int		CIntArr::Max()

{
	int	mxx = INT_MIN, alen = GetSize();

	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		int val = GetAt(loop3);

		if(val > mxx) 
			mxx = val;	
		}
	return mxx;
}

//////////////////////////////////////////////////////////////////////////
//

int		CIntArr::Min()

{
	int	mxx = INT_MAX, alen = GetSize();

	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		int val = GetAt(loop3);
		if(val < mxx) 
			mxx = val;
		}
	return mxx;
}

//////////////////////////////////////////////////////////////////////////
//

void	CIntArr::Abs()

{
	int len = GetSize();
	for(int loop = 0; loop < len; loop++)
		{
		SetAt(loop, abs(GetAt(loop)));
		}
}

//////////////////////////////////////////////////////////////////////////
// Fill up array

int		CIntArr::Fill(int size, int val)

{
	SetSize(size);

	int len = GetSize();
	for(int loop = 0; loop < len; loop++)
		{
		SetAt(loop, val);
		}
	return len;
}
		

//////////////////////////////////////////////////////////////////////////

void	CIntArr::Clear(int val)

{
	int	alen = GetSize();
	for(int loop3 = 0; loop3 < alen; loop3++)
		{
		SetAt(loop3, val);
		}
}

//////////////////////////////////////////////////////////////////////////
//
/// \brief  CIntArr::Denoise -- Remove steps that are smaller then diff
/// 
/// \details 
/// \n  
/// Return type:      i
/// \n  
//
// -----------------------------------------------------------------------

void	CIntArr::DeNoise(int diff)

{
	CIntArr tmp;
	int	old = 0, alen = GetSize();
	
	for(int loop3 = 1; loop3 < alen; loop3++)
		{
		int curr = GetAt(loop3), xdiff = curr - GetAt(loop3 - 1);

		if(xdiff > diff)
			old = curr;

		tmp.Add(old);
		}

	// Commit
	RemoveAll();
	Copy(tmp);
}

//////////////////////////////////////////////////////////////////////////

//DEL void	CIntArr::Diff()
//DEL 
//DEL {
//DEL 	CIntArr tmp;
//DEL 	int	alen = GetSize();
//DEL 	
//DEL 	for(int loop3 = 1; loop3 < alen; loop3++)
//DEL 		{
//DEL 		int val = GetAt(loop3) - GetAt(loop3 - 1);
//DEL 		tmp.Add(val);
//DEL 		}
//DEL 
//DEL 	RemoveAll();
//DEL 	Copy(tmp);
//DEL }

//////////////////////////////////////////////////////////////////////////

void	CIntArr::Liftup()

{
	//ASSERT(normto > 0);

	if(GetSize() == 0)
		{
		//AP2N("CIntArr::Liftup empty array\r\n");
		return ;
		}

	int loop, asize = GetSize();
	if(!asize)
		return ;

	// Calc max neg value
	int minn = INT_MAX;
	for(loop = 0; loop < asize; loop++)
		{
		int val = GetAt(loop);
		if(val < minn)
			minn = val;
		}

	minn = abs(minn);
	for(loop = 0; loop < asize; loop++)
		{
		int val = GetAt(loop);
		SetAt(loop, val + minn);		
		}
}


// -----------------------------------------------------------------------
//
/// \brief  CIntArr::AddToVal  Add value for index, expand array if needed
/// 
/// \details CIntArr::AddToVal Returns the old value, 0 if was none
/// \n  
/// Return type:      int 
/// \n  
/// \param idx Arg Type: int 
/// \param val Arg Type: int 
//
// -----------------------------------------------------------------------

int CIntArr::AddToVal(int idx, int val)

{
	int old = 0;
	if(idx < GetSize())
		old = GetAt(idx);

	SetAtGrow(idx, old + val);

	return old;
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::SafeGetAt  Get array element, 0 if invalid index
/// 
/// \details CIntArr::SafeGetAt Descibe Here 
/// \n  
/// Return type:      int 
/// \n  
/// \param idx Arg Type: int 
//
// -----------------------------------------------------------------------


int CIntArr::SafeGetAt(int idx)

{
	int old = 0;

	//if(idx >= 0 && idx < GetSize())  -- faster directly

	if(idx >= 0 && idx < m_nSize)
		old = m_pData[idx];

	return old;
}

// -----------------------------------------------------------------------
//
/// \brief  CIntArr::TurnArr  Output turning points of an array
/// 
/// \details CIntArr::TurnArr Descibe Here 
/// \n  
/// Return type:      void 
/// \n  
/// \param *pout Arg Type: CIntArr 
/// \param *vout Arg Type: CIntArr 
/// \param peakonly Arg Type: int  
///							1 => Put only positive peaks (hills)
///							-1 => Put only negative peaks (valleys)
///							0 => Put all peaks (all)
//
// -----------------------------------------------------------------------

void CIntArr::TurnArr(CIntArr *pout, CIntArr *vout, int peakonly)

{
	int len = GetSize(), dist = 0;

	int	pprev = 0, ptrend = 0, pold_trend = 0;
	int oldpoint = 0;

	for( int loop = 1; loop < len; loop++)
		{
		int val = GetAt(loop);
		int	ptrend = pold_trend, pddiff = pprev - val;
		
		if(pddiff > 0) ptrend = 1;
		else if(pddiff < 0) ptrend = -1;

		if(pold_trend != ptrend) 
			{
			if(peakonly == 1)
				{
				if(ptrend == 1 )
					{
					//pout->Add(dist);
					// Add Current point
					vout->Add(pprev);
					pout->Add(loop - 1);
					}
				}
			if(peakonly == -1)
				{
				if(ptrend == -1 )
					{
					//pout->Add(dist);
					// Add Current point
					vout->Add(pprev);
					pout->Add(loop - 1);
					}
				}
			if(peakonly == 0)
				{
				vout->Add(pprev);
				pout->Add(loop - 1);
				}			

			oldpoint = val;
			pold_trend = ptrend;			
			dist = 0;
			}
		dist++;
		pprev = val;
		}
}

//////////////////////////////////////////////////////////////////////////

int CIntArr::ConvDist(CIntArr *varr, CIntArr *darr2, CIntArr *varr2)

{
	darr2->RemoveAll();	varr2->RemoveAll();
	
	int len = GetSize();

	if(!len)
		return -1;

	int prev = 0; //GetAt(0);

	for(int loop = 0; loop < len; loop++)
		{
		int curr = GetAt(loop);
		int diff = curr - prev;

		// It was mostly noise, conveniently ignore it
		if(diff > 1)
			{
			darr2->Add(diff);
			varr2->Add(varr->GetAt(loop));
			}
		prev = curr;
		}

	//if(((CPlay*)AfxGetApp()->m_pMainWnd)->record)
		{
		//varr2->Plot(m_plot5);
		//darr2->Plot(m_plot6);
		}

	//P2N("Bend::ConvDist outlen=%d\r\n", darr2->GetSize());

	return 0;
}



//////////////////////////////////////////////////////////////////////////
// Init internal vars

void	CIntArr::xInit()

{
	magic = CIntArr_Magic;
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}


//////////////////////////////////////////////////////////////////////////
// Kill parts of the array

void	CIntArr::KillSubArr(int start, int endd)

{
	int nlen = GetSize();

	// Contain it to reasonable defaults
	start = MIN(start, nlen - 1); endd = MIN(endd, nlen - 1);
	start = MAX(start, 1);  endd = MAX(start, endd);

	for(int loop2 = start; loop2 < endd; loop2++)
		{
		SetAt(loop2, 0);
		}
}

//////////////////////////////////////////////////////////////////////////
// Copy in a part of the array 

void	CIntArr::CopySubArr(CIntArr *parr, int start, int endd)

{
	int nlen = parr->GetSize();	
	
	// Contain it to reasonable defaults
	start = MIN(start, nlen - 1); endd = MIN(endd, nlen - 1);
	start = MAX(start, 0);  endd = MAX(start, endd);

	Clear(); SetSize(endd);

	for(int loop2 = start; loop2 < endd; loop2++)
		{
		SetAt(loop2, parr->GetAt(loop2));
		}
}