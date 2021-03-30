/*
Copyright (c) 2003-2010, Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	* Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* kiss_fft.h
   defines kiss_fft_scalar as either short or a float type
   and defines
   typedef struct { kiss_fft_scalar r; kiss_fft_scalar i; }kiss_fft_cpx; */
#include "kiss_fft.h"

#define MAXFACTORS 32
/* e.g. an fft of length 128 has 4 factors
 as far as kissfft is concerned
 4*4*4*2
 */

struct kiss_fft_state{
	int nfft;
	int inverse;
	int factors[2*MAXFACTORS];
	kiss_fft_cpx twiddles[1];
};

/*
  Explanation of macros dealing with complex math:

   C_MUL(m,a,b)         : m = a*b
   C_FIXDIV( c , div )  : if a fixed point impl., c /= div. noop otherwise
   C_SUB( res, a,b)     : res = a - b
   C_SUBFROM( res , a)  : res -= a
   C_ADDTO( res , a)    : res += a
 * */

#   define S_MUL(a,b) ( (a)*(b) )
#define C_MUL(m,a,b) \
	do{ (m).r = (a).r*(b).r - (a).i*(b).i;\
		(m).i = (a).r*(b).i + (a).i*(b).r; }while(0)
#   define C_FIXDIV(c,div) /* NOOP */
#   define C_MULBYSCALAR( c, s ) \
	do{ (c).r *= (s);\
		(c).i *= (s); }while(0)

#ifndef CHECK_OVERFLOW_OP
#  define CHECK_OVERFLOW_OP(a,op,b) /* noop */
#endif

#define  C_ADD( res, a,b)\
	do { \
		CHECK_OVERFLOW_OP((a).r,+,(b).r)\
		CHECK_OVERFLOW_OP((a).i,+,(b).i)\
		(res).r=(a).r+(b).r;  (res).i=(a).i+(b).i; \
	}while(0)
#define  C_SUB( res, a,b)\
	do { \
		CHECK_OVERFLOW_OP((a).r,-,(b).r)\
		CHECK_OVERFLOW_OP((a).i,-,(b).i)\
		(res).r=(a).r-(b).r;  (res).i=(a).i-(b).i; \
	}while(0)
#define C_ADDTO( res , a)\
	do { \
		CHECK_OVERFLOW_OP((res).r,+,(a).r)\
		CHECK_OVERFLOW_OP((res).i,+,(a).i)\
		(res).r += (a).r;  (res).i += (a).i;\
	}while(0)

#  define KISS_FFT_COS(phase) (kiss_fft_scalar) cos(phase)
#  define KISS_FFT_SIN(phase) (kiss_fft_scalar) sin(phase)
#  define HALF_OF(x) ((x)*.5)

#define  kf_cexp(x,phase) \
	do{ \
		(x)->r = KISS_FFT_COS(phase);\
		(x)->i = KISS_FFT_SIN(phase);\
	}while(0)

#define  KISS_FFT_TMP_ALLOC(nbytes) KISS_FFT_MALLOC(nbytes)
#define  KISS_FFT_TMP_FREE(ptr) KISS_FFT_FREE(ptr)
