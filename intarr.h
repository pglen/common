
/* =====[ cintarr.h ]========================================== 
                                                                             
   Description:     The daco project, implementation of the cintarr.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  10/5/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#if !defined(AFX_CIntArr_H__BBEF6A88_28E3_49C4_BA73_54244624BD14__INCLUDED_)
#define AFX_CIntArr_H__BBEF6A88_28E3_49C4_BA73_54244624BD14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef PLOT
#include "plot.h"
#endif

#define CIntArr_Magic	0x5523334

#define	ALL_TURN	0
#define	TOP_TURN	1
#define	BOTTOM_TURN -1

//#define CHECK_VALID_CINTARR(xxx) ASSERT(xxx->magic == CIntArr_Magic);

//////////////////////////////////////////////////////////////////////////
// Prevents us from pulling in a dependency

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif


///
/// \brief %CIntArr array of short values. This class is developed for dealing with 
/// sound samples and sample values, and some rudimentary DSP, and FFT output
//
/// Ported from cptrarray. Note: Not all funct are tested ...
/// \n
/// Additional Functions implemented:
/// \li Find, \li AddUnique, \li RemoveValue ... many more \li ...\n
///
/// Extensive use of the macro ASSERT(magic == CIntArr_Magic); for
///  sanity checks.\n
//
/// The DEBUG version ASSERTS on some out of bound conditions, the Release 
/// version checks for overflow, and in that case it does a NOOP
///

class CIntArr 

{

public:

	int		Fill(int size, int val);
	void	Plus(int add);
	int		SafeGetAt(int idx);
	int		Max(); 	int Min();
	void	Abs();

	void	TrimPoints(int treshold, int *begp, int *endp);
	void	Liftup();
	void	FindMin2(int *pidx1, int *val1, int *pidx2, int *val2);
	void	FindMin(int *idx, int *val);
	void	EdgeScript(CIntArr *dist, CIntArr *val);
	void	FindTrim(int *start, int *end);
	void	Trim(int start, int endd);
	void	Trim(int treshold);
	void	EdgeDetect(CIntArr *res);
	int		Find(const int val);
	int		AddUniq(const int val);
	void	RemoveValue(int val);
	void	RunAve(const int passes = 1);
	void	Sort(CIntArr *parr,  int reverse = false);
		
	void	TurnArr(CIntArr *pout, CIntArr *vout, int peakonly = 0);
	int		ConvDist(CIntArr *darr2, CIntArr *varr, CIntArr *varr2);

	void	DeNoise(int diff = 1);
	void	Clear(int val = 0);

	void	KillSubArr(int start, int endd);
	void	CopySubArr(CIntArr *parr, int start, int endd);

	//////////////////////////////////////////////////////////////////////////
	// Construct / destruct

	CIntArr();
	CIntArr(int inisize);
	CIntArr(CIntArr *org);

	virtual ~CIntArr();

	void	FindMax3(int *pidx1, int *val1, int *pidx2, int *val2, int *pidx3, int *val3);
	void	FindMax2(int *pidx1, int *val1, int *pidx2, int *val2);
	void	FindMax(int *idx, int *val);
	void	FindMax4(int *pidx1, int *val1, int *pidx2, int *val2, 
						  int *pidx3, int *val3, int *pidx4, int *val4);

	
#ifdef PLOT
	int		Plot(CPlot *pplot);
#endif

public:

	//////////////////////////////////////////////////////////////////////

	/// Set New Size, Expand if neccessary
	void	SetSize(int nNewSize, int nGrowBy = -1);

	// Operations

	/// Free extra allocated space
	void	FreeExtra();

	/// Set array value, grow it if needed
	void	SetAtGrow(int nIndex, int newElement);

	int		Append(const CIntArr& src);

	// Copy array
	void	Copy(const CIntArr& src);
	void	Copy(CIntArr *src);

	// Operations that move elements around
	void	InsertAt(int nIndex, int newElement, int nCount = 1);
	void	RemoveAt(int nIndex, int nCount = 1);
	void	InsertAt(int nStartIndex, CIntArr* pNewArray);

	// Accumulate values
	int AddToVal(int idx, int val);

	int magic;

	////////////////////////////////////////////////////////////////////////////

	void Dump();
	void DumpAll(const char *str = NULL);

	/// \brief Returns the size of the array. 
	///
	/// Since indexes are zero-based,  the size is 1 greater than the largest 
	/// index
	inline int GetSize() const
		{ ASSERT(magic == CIntArr_Magic); return m_nSize; }

	/// \brief Return the index of the upper bound (zero-based).
	///
	/// Returns the current upper bound of this array. Because array indexes are zero-based, 
	/// this function returns a value 1 less than GetSize. \n
	/// The condition %GetUpperBound( ) = -1 indicates that the array contains no elements.

	inline int GetUpperBound() const
		{ return m_nSize - 1; }

	/// \brief Removes all data from the array.

	///	Removes all the pointers from this array but does not actually delete what is behind the pointers. 
	///	If the array is already empty, the function still works. 
	///	The RemoveAll function frees all memory used for pointer storage.

	inline void RemoveAll()
		{ SetSize(0); }

	//////////////////////////////////////////////////////////////////////////
	// Accessing elements

	/// Get member helper
	inline int GetAt(int nIndex) const
		{	
			ASSERT(magic == CIntArr_Magic);
			ASSERT(nIndex >= 0 && nIndex < m_nSize);
 			return m_pData[nIndex]; }

	/// Set member helper
	inline void SetAt(int nIndex, int newElement)
		{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
			m_pData[nIndex] = newElement; }

	/// Get member helper
	inline int& ElementAt(int nIndex)
		{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
			return m_pData[nIndex]; }

	/// Get pointer to actual array. Use it with care
	inline const int* GetData() const
		{ return (const int*)m_pData; }

	/// Get pointer to actual array. Use it with care
	inline int* GetData()
		{ return (int*)m_pData; }

	// Add new element at the end
	inline int Add(int newElement)
		{ int nIndex = m_nSize;
			SetAtGrow(nIndex, newElement);
			return nIndex; }

	// Add new buffer at the end
	int Add(int *buff, int len);

	// Overloaded operator helpers

	/// Casting to integer will yield the size of the array
	inline 	operator int () 
		{ return m_nSize; }

	/// Casting to integer will yield the size of the array
	inline 	operator int() const
		{ return m_nSize; }

	/// Casting to int * will yield ptr to the actual array
	inline 	operator int * () const
		{ return m_pData; }

	// Direct Access to the element data (may return NULL)

	/// Array access to members of the class data
	inline int operator[](int nIndex) const
		{ return GetAt(nIndex); }

	/// Array access to members of the class data
	inline int& operator[](int nIndex)
		{ return ElementAt(nIndex); }


protected:

	void	xInit();

	int		*m_pData;		// the actual array of data
	int		m_nSize;		// # of elements (upperBound - 1)
	int		m_nMaxSize;		// max allocated
	int		m_nGrowBy;		// grow amount
	
};

#endif // !defined(AFX_CIntArr_H__BBEF6A88_28E3_49C4_BA73_54244624BD14__INCLUDED_)
