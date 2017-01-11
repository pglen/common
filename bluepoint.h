
/* =====[ bluepoint.h ]========================================================== 
                                                                             
   Description:     The CryptoSticky project, implementation of the bluepoint.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
	  0.00  6/17/2010  Peter Glen   Initial version.
	  0.00  jan/2/2016 Peter Glen   C++ CString functions, wrappers

   ======================================================================= */



// Error Codes for decrypthex 

#define CRYPT_OK		0
#define CRYPT_EMPTY		1				// Empty cypher text
#define CRYPT_NOHEAD	2				// Missing #CRYPT# header
#define CRYPT_NOTAIL	3				// Missing #XCRYPT# footer
#define CRYPT_BADPASS   4				// Invalid password

// Soft define items

#ifndef ulong
typedef  unsigned long ulong;
#endif

#ifndef	ROTATE_LONG_RIGHT

#define     ROTATE_LONG_RIGHT(x, n) (((x) >> (n))  | ((x) << (32 - (n))))
#define     ROTATE_LONG_LEFT(x, n) (((x) << (n))  | ((x) >> (32 - (n))))
#define     ROTATE_CHAR_RIGHT(x, n) (((x) >> (n))  | ((x) << (8 - (n))))
#define     ROTATE_CHAR_LEFT(x, n) (((x) << (n))  | ((x) >> (8 - (n))))

#endif

// Soft define encryption limits

#define PASSLIM  32						//# maximum key length (bytes)

///////////////////////////////////////////////////////////////////////////
// The following defines are used to test multi platform steps.
// These will generate a cypher text incompatible with other implementations
// FOR TESTING ONLY

//define NOROTATE        1   // uncomment this if you want no rotation
//define NOPASSCRYPT     1   // uncomment this if you want no pass crypt

#define DEF_DUMPHEX  1   // undefine this if you do not want bluepoint_dumphex

//////////////////////////////////////////////////////////////////////////
//# -------------------------------------------------------------------------
//# Bluepoint encryption routines.
//#
//#   How it works:
//#
//#     Strings are walked chr by char with the loop:
//#         {
//#         $aa = ord(substr($_[0], $loop, 1));
//#         do something with $aa
//#         substr($_[0], $loop, 1) = pack("c", $aa);
//#         }
//#
//#   Flow:
//#         generate vector
//#         generate pass
//#
//#         walk forward with password cycling loop
//#         walk backwards with feedback encryption
//#         walk forward with feedback encryption
//#
//#  The process guarantees that a single bit change in the original text
//#  will change every byte in the resulting block.
//#
//#  The bit propagation is such a high quality, that it beats current
//#  industrial strength encryptions.
//#
//#  Please see bit distribution study.
//#  
//# -------------------------------------------------------------------------

class	xCBluePoint

{	
	//# -------------------------------------------------------------------------
	//# These vars can be set to make a custom encryption:

	char	*vector;			//# influence encryption algorythm

	char    forward;    		//# Constant propagated on forward pass
	char    backward;   		//# Constant propagated on backward pass
	char    addend;     		//# Constant used adding to encrypted values

	//# -------------------------------------------------------------------------
	//# These vars can be set show op details

public:

	int debug;                  //# Specify this to show P2Ns for test
	int verbose;                //# Specify this to show working details
	int functrace;              //# Specify this to show function args

	xCBluePoint();

	// More details, more control, more knowledge needed
	void	encrypt(char *buff, int blen, const char *pass, int plen);
	void	decrypt(char *str, int slen, const char *pass, int plen);
	ulong   hash(const char *buff, int blen);
	ulong   crypthash(const char *buff, int blen, const char *pass, int plen);

	#ifdef DEF_DUMPHEX
	char 	*bluepoint_dumphex(const char *str, int len);
	#endif

	char    *tohex(const char *str, int len, char *out, int *olen , int llen = 0);
	TCHAR   *tohexW(const TCHAR *str, int len, TCHAR *out, int *olen, int llen = 0);

	char    *fromhex(const char *str, int len, char *out, int *olen);
	TCHAR   *fromhexW(const TCHAR *str, int len, TCHAR *out, int *olen);

protected:

	// Internals, leave them alone
	void    do_encrypt(char *str, int slen, char *pass, int plen);
	void    do_decrypt(char *str, int slen, char *pass, int plen);
	void    prep_pass(const char *pass, int plen, char *newpass);

};


// This new class is a wrapper on the worker code. See 'C' code origins.

class	CBluePoint : public xCBluePoint
{

	static CString pre;
	static CString post;

public:

	CBluePoint::CBluePoint() {};

	// Encrypt, hash and hex encode. Entry points for CryptoSticky
	void    CBluePoint::encrypthex(CString &str, const CString &pass);
	int     CBluePoint::decrypthex(CString &str, const CString &pass);

	// Nice end simple versions of encryption
	void    CBluePoint::encrypt(CString &str, const CString &pass);
	void    CBluePoint::decrypt(CString &str, const CString &pass);

	// Nice end simple versions of HEX conversions
	void	CBluePoint::fromhexW(const CString &hex, CString &uhex);
	void	CBluePoint::tohexW(const CString &str, CString &out, int linelen = 0);

	// Nice end simple versions of utilities
	void    CBluePoint::killpass(CString &str);
	ulong   CBluePoint::hash(const CString &str);
	ulong   CBluePoint::crypthash(const CString &str, const CString &pass);

};

// One and only object (pre declared in source) ... you may do more, but not needed.
extern CBluePoint bluepoint;

/* EOF */
