#pragma once

#include "types.h"

inline float RSQRT(float x) {
//    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
    float y;
    __asm__("rsqrtss  %1, %0;"
            : "=x"(y)
            : "x" (x)
           );
    return y;
}

/************************ multiplication ******************************/

// a0:a1 = x*y
inline void MUL(uint64_t*a0, uint64_t*a1, uint64_t x, uint64_t y) {
    __asm__("mulq %3      ;"
            : "=a" (*a0), "=d" (*a1)
            : "a" (x), "rm" (y)
            : "cc");
}

// a0:a1 = x*y
#ifdef __GCC_ASM_FLAG_OUTPUTS__
inline bool IMUL(int64_t*a0, int64_t*a1, int64_t x, int64_t y) {
    bool of;
    __asm__("imulq %4     ;"
            : "=a" (*a0), "=d" (*a1), "=@cco" (of)
            : "a" (x), "rm" (y)
            : "cc");
    return of;
}
#else
inline bool IMUL(int64_t*a0, int64_t*a1, int64_t x, int64_t y) {
    bool of;
    __asm__("imulq %4     ;"
            "seto  %2     ;"
            : "=a" (*a0), "=d" (*a1), "=qm" (of)
            : "a" (x), "rm" (y)
            : "cc");
    return of;
}
#endif

inline bool IMUL(slong & a, slong c, slong b)
{
	slong Ahi, Alo, A = b*c;
	smul_ppmm(Ahi, Alo, b, c);
	a = A;
	return FLINT_SIGN_EXT(A) != Ahi;
}

inline bool UMUL(ulong & a, ulong b, ulong c)
{
	ulong Ahi, Alo, A = b*c;
	umul_ppmm(Ahi, Alo, b, c);
	a = A;
	return 0 != Ahi;
}


/************************ division ********************************/


// q, r  when dividing x0:x1 by y
inline volatile void DIV(uint64_t*q, uint64_t*r, uint64_t x0, uint64_t x1, uint64_t y) {
    __asm__("divq %4          ;"
            : "=a" (*q), "=d" (*r)
            : "a" (x0), "d" (x1), "rm" (y)
            : "cc");
}

inline slong SAR(slong a, int b) {
    return a>>b;
}

inline ulong SAR(ulong a, int b) {
    return ((slong)(a))>>b;
}


/*********************** additions *******************************/

// return (a + b + cf)/2
inline uint64_t AVG(uint64_t a, uint64_t b, uint64_t cf) {
    __asm__("negq %1       ;"
            "adcq %3, %0   ;"
            "rcrq $1, %0   ;"
            : "=&r" (a)
            : "r" (cf), "0" (a), "rm" (b)
            : "cc");
    return a;
}


// return a + b, set cf
inline uint64_t ADD(uint64_t a, uint64_t b, uint64_t*cf) {
    __asm__("addq %3, %1   ;"
            "sbbq %0, %0   ;"
            : "=&r" (*cf), "=&r" (a)
            : "1" (a), "rm" (b)
            : "cc");
    return a;
}

// return a + b + cf, set cf
inline uint64_t ADC(uint64_t a, uint64_t b, uint64_t*cf) {
    __asm__("negq %0       ;"
            "adcq %4, %1   ;"
            "sbbq %0, %0   ;"
            : "=&r" (*cf), "=&r" (a)
            : "0" (*cf), "1" (a), "rm" (b)
            : "cc");
    return a;
}

// a = x + y, return of
/*
#ifdef __GCC_ASM_FLAG_OUTPUTS__
inline bool IADD(int64_t*a, int64_t x, int64_t y) {
    bool of;
    __asm__("addq %3, %0    ;"
            : "=&r" (*a), "=@cco" (of)
            : "0" (x), "rm" (y)
            : "cc");
    return of;
}
#else
inline bool IADD(int64_t*a, int64_t x, int64_t y) {
    bool of;
    __asm__("addq %3, %0    ;"
            "seto %1        ;"
            : "=&r" (*a), "=qm" (of)
            : "0" (x), "rm" (y)
            : "cc");
    return of;
}
#endif
*/

inline bool IADD(slong & a, slong b, slong c)
{
	slong A = b + c;
	a = A;
	return (b > 0 && c > 0 && A <= 0) || (b < 0 && c < 0 && A >= 0);
}

inline bool UADD(ulong & a, ulong b, ulong c)
{
	ulong A = b + c;
	a = A;
	return A < b;
}


// a0:a1 += b
inline void ADD21(uint64_t*a0, uint64_t*a1, uint64_t b) {
    __asm__("addq %4, %0    ;"
            "adcq $0, %1    ;"
            : "=r" (*a0), "=rm" (*a1)
            : "0" (*a0), "1" (*a1), "rm" (b)
            : "cc");
}

// a0:a1 += b0:b1
inline uint64_t ADD22(uint64_t*a0, uint64_t*a1, uint64_t b0, uint64_t b1) {
    uint64_t carry;
    __asm__("addq %5, %0    ;"
            "adcq %6, %1    ;"
            "sbbq %2, %2    ;"
            : "=rm" (*a0), "=rm" (*a1), "=r" (carry)
            : "0" (*a0), "1" (*a1), "r" (b0), "r" (b1)
            : "cc");
    return carry;
}
// a0:a1 += b0:b1
inline void IADD22(int64_t*a0, int64_t*a1, int64_t b0, int64_t b1) {
    uint64_t carry;
    __asm__("addq %5, %0    ;"
            "adcq %6, %1    ;"
            : "=rm" (*a0), "=rm" (*a1), "=r" (carry)
            : "0" (*a0), "1" (*a1), "r" (b0), "r" (b1)
            : "cc");
}

// a0:a1 += b0:b1 + cf, return cf
inline uint64_t ADC22(uint64_t*a0, uint64_t*a1, uint64_t b0, uint64_t b1, uint64_t carryin) {
    uint64_t carry;
    __asm__("negq %7        ;"
            "adcq %5, %0    ;"
            "adcq %6, %1    ;"
            "sbbq %2, %2    ;"
            : "=rm" (*a0), "=rm" (*a1), "=r" (carry)
            : "0" (*a0), "1" (*a1), "r" (b0), "r" (b1), "r" (carryin)
            : "cc");
    return carry;
}

// a0:a1:a2 += b0:b1
inline void ADD32(uint64_t*a0, uint64_t*a1, uint64_t*a2, uint64_t b0, uint64_t b1) { 
    __asm__("addq %6, %0   ;"
            "adcq %7, %1   ;"
            "adcq $0, %2      ;"
            : "=rm" (*a0), "=rm" (*a1), "=rm" (*a2)
            : "0" (*a0), "1" (*a1), "2" (*a2), "r" (b0), "r" (b1)
            : "cc");
}


// a0:a1:a2 += b0:b1:b2, return cf
inline uint64_t ADD33(uint64_t*a0, uint64_t*a1, uint64_t*a2, uint64_t b0, uint64_t b1, uint64_t b2) {
    uint64_t carry;
    __asm__("addq %7, %0    ;"
            "adcq %8, %1    ;"
            "adcq %9, %2    ;"
            "sbbq %3, %3    ;"
            : "=r" (*a0), "=r" (*a1), "=r" (*a2), "=r" (carry)
            : "0" (*a0), "1" (*a1), "2" (*a2), "rm" (b0), "rm" (b1), "rm" (b1)
            : "cc");
    return carry;
}



/*********************** subtractions *******************************/



// a = x - y, return cf
#ifdef __GCC_ASM_FLAG_OUTPUTS__
inline bool USUB(uint64_t*a, uint64_t x, uint64_t y) {
    bool cf;
    __asm__("subq %3, %0    ;"
            : "=&r" (*a), "=@ccc" (cf)
            : "0" (x), "rm" (y)
            : "cc");
    return cf;
}
#else
inline bool SUB(uint64_t*a, uint64_t x, uint64_t y) {
    bool of;
    __asm__("subq %3, %0    ;"
            "setc %1        ;"
            : "=&r" (*a), "=qm" (of)
            : "0" (x), "rm" (y)
            : "cc");
    return of;
}
#endif


// return a - b, set cf
inline uint64_t SUB(uint64_t a, uint64_t b, uint64_t*cf) { 
  __asm__("subq %3, %1   ;"
          "sbbq %0, %0   ;"
          : "=&r" (*cf), "=&r" (a)
          : "1" (a), "rm" (b)
          : "cc");
    return a;
}

// return a - b - cf, set cf
inline uint64_t SBB(uint64_t a, uint64_t b, uint64_t*cf) {
  __asm__("negq %0       ;"
          "sbbq %4, %1   ;"
          "sbbq %0, %0   ;"
          : "=&r" (*cf), "=&r" (a)
          : "0" (*cf), "1" (a), "rm" (b)
          : "cc");
    return a;
}

// a = x - y, return of
#ifdef __GCC_ASM_FLAG_OUTPUTS__
inline bool ISUB(int64_t*a, int64_t x, int64_t y) {
    bool of;
    __asm__("subq %3, %0    ;"
            : "=&r" (*a), "=@cco" (of)
            : "0" (x), "rm" (y)
            : "cc");
    return of;
}
#else
inline bool ISUB(int64_t*a, int64_t x, int64_t y) {
    bool of;
    __asm__("subq %3, %0    ;"
            "seto %1        ;"
            : "=&r" (*a), "=qm" (of)
            : "0" (x), "rm" (y)
            : "cc");
    return of;
}
#endif

// a0:a1 -= b;
inline void SUB21(uint64_t*a0, uint64_t*a1, uint64_t b) {
    __asm__("subq %4, %0    ;"
            "sbbq $0, %1    ;"
            : "=r" (*a0), "=rm" (*a1)
            : "0" (*a0), "1" (*a1), "rm" (b)
            : "cc");
}

// a0:a1 -= b0:b1   return is carry flag
inline uint64_t SUB2(uint64_t*a0, uint64_t*a1, uint64_t b0, uint64_t b1) {
    uint64_t c;
    __asm__("subq %5, %0    ;"
            "sbbq %6, %1    ;"
            "sbbq %2, %2    ;"
            : "=r" (*a0), "=r" (*a1), "=r" (c)
            : "0" (*a0), "1" (*a1), "rm" (b0), "rm" (b1)
            : "cc");
    return c;
}

// a0:a1 -= b0:b1
inline void ISUB22(int64_t*a0, int64_t*a1, int64_t b0, int64_t b1) {
    uint64_t c;
    __asm__("subq %5, %0    ;"
            "sbbq %6, %1    ;"
            : "=r" (*a0), "=r" (*a1), "=r" (c)
            : "0" (*a0), "1" (*a1), "rm" (b0), "rm" (b1)
            : "cc");
}

// a0:a1 -= b0:b1   return is carry flag
inline uint64_t SUB3(uint64_t*a0, uint64_t*a1, uint64_t*a2, uint64_t b0, uint64_t b1, uint64_t b2) {
    uint64_t c;
    __asm__("subq %7, %0    ;"
            "sbbq %8, %1    ;"
            "sbbq %9, %2    ;"
            "sbbq %3, %3    ;"
            : "=r" (*a0), "=r" (*a1), "=r" (*a1), "=r" (c)
            : "0" (*a0), "1" (*a1), "2" (*a1), "rm" (b0), "rm" (b1), "rm" (b2)
            : "cc");
    return c;
}




/******************************** misc ****************************/

inline int64_t SIGN_EXT(int64_t x) {return -(x<0);}
//inline int64_t SIGN(int64_t x) {return x>>63;}
inline int64_t IABS(int64_t x) {return (x^(x>>63))-(x>>63);}


inline int clamp(int x, int a, int b) {
    return std::min(std::max(x,a),b);
}
inline int64_t clamp(int64_t x, int64_t a, int64_t b) {
    return std::min(std::max(x,a),b);
}

// least sig bit
inline uint64_t BSF(uint64_t x) {
    uint64_t idx;
    __asm__( "bsfq %1, %0   ;"
            : "=r"(idx)
            : "rm"(x)
            : "cc");
    return idx;
}
inline uint64_t LSB(uint64_t x) {
    uint64_t idx;
    __asm__( "bsfq %1, %0   ;"
            : "=r"(idx)
            : "rm"(x)
            : "cc");
    return idx;
}

inline uint32_t BSF(uint32_t x) {
    uint32_t idx;
    __asm__( "bsfl %1, %0   ;"
            : "=r"(idx)
            : "rm"(x)
            : "cc");
    return idx;
}


// most sig bit
inline uint64_t BSR(uint64_t x) {
    uint64_t idx;
    __asm__( "bsrq %1, %0   ;"
            : "=r"(idx)
            : "rm"(x)
            : "cc");
    return idx;
}
inline uint64_t MSB(uint64_t x) {
    uint64_t idx;
    __asm__( "bsrq %1, %0   ;"
            : "=r"(idx)
            : "rm"(x)
            : "cc");
    return idx;
}


// shift a to the right, shifting high bits in from b;
inline uint64_t SHRD(uint64_t a, uint64_t b, uint64_t k) { 
    uint64_t c;
    __asm__("shrdq %%cl, %2, %0   ;"
            : "=r" (a)
            : "0" (a), "r" (b), "c" (k)
            : "cc");
    return a;
}


// shift a to the left, shifting low bits in from b;
inline uint64_t SHLD(uint64_t a, uint64_t b, uint64_t k)
{
/*
    __asm__("shldq %%cl, %2, %0   ;"
            : "=r" (a)
            : "0" (a), "r" (b), "c" (k)
            : "cc");
    return a;
*/
    return (a << k) | (b >> (64 - k));
}

inline uint64_t ROL(uint64_t a, uint64_t k) {
    return (a<<k) | (a>>(64-k));
}

inline uint64_t ROR(uint64_t a, uint64_t k) { 
    return (a>>k) | (a<<(64-k));
}

// breakpoint
inline volatile void INT3() {
    __asm__("int3");
}

