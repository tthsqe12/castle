#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include <complex>
#include <cmath>
#include <cfloat>
#include <memory>
#include <exception>
#include <new>

#include "flint/flint.h"


inline char* _int_print(char* s, uint64_t x) {
    uint64_t r = x%10;
    uint64_t q = x/10;
    if (q!=0) {s = _int_print(s, q);}
    *s++ = r + '0';
    return s; 
}

inline char* _int_print(char* s, int64_t x) {
    if (x<0) {*s++='-';x=-x;}
    return _int_print(s,(uint64_t)x);
}

inline std::string stdstring_to_string(int a)
{
    char buffer[30];
    char* end = _int_print(buffer, (int64_t)a);
    *end = 0;
    std::string s(buffer);
    return s;
}

class exception_exit: public std::exception {
public:
    int retcode;
    exception_exit(int retcode_) : retcode(retcode_) {}
};

class exception_stack_overflow: public std::exception {
public:
    void * data;
    exception_stack_overflow(void * d) : data(d) {}
};

class exception_sym_sBreak: public std::exception {
public:
    void * data;
    exception_sym_sBreak(void * d = nullptr) : data(d) {}
};

class exception_sym_sContinue: public std::exception {
public:
    void * data;
    exception_sym_sContinue(void * d = nullptr) : data(d) {}
};

class exception_sym_sThrow: public std::exception {
public:
    void * data;
    exception_sym_sThrow(void * d) : data(d) {}
};

class exception_sym_sGoto: public std::exception {
public:
    void * data;
    exception_sym_sGoto(void * d) : data(d) {}
};

class exception_sym_sReturn: public std::exception  {
public:
    void * data;
    exception_sym_sReturn(void * d) : data(d) {}
};

class exception_sym_sReturn_2: public std::exception  {
public:
    void * data;
    exception_sym_sReturn_2(void * d) : data(d) {}
};


#define likely(x)       __builtin_expect(!!(x),1)
#define unlikely(x)     __builtin_expect(!!(x),0)

#define EXTRA_PRECISION_BASIC 40

#define CHAR_NONE    (-1)
#define CHAR_NEWLINE (-2)

#define NAMED_CHARS_COUNT (788+3)
#define CHAR_Sum 0x2211
#define CHAR_Product 0x220f
#define CHAR_Integral 0x222b
#define CHAR_AAcute 0x00e1
#define CHAR_ABar 0x0101
#define CHAR_ACup 0x0103
#define CHAR_ADoubleDot 0x00e4
#define CHAR_AE 0x00e6
#define CHAR_AGrave 0x00e0
#define CHAR_AHat 0x00e2
#define CHAR_Aleph 0x2135
#define CHAR_AliasDelimiter 0xf764
#define CHAR_AliasIndicator 0xf768
#define CHAR_Alpha 0x03b1
#define CHAR_AltKey 0xf7d1
#define CHAR_And 0x2227
#define CHAR_Angle 0x2220
#define CHAR_Angstrom 0x212b
#define CHAR_ARing 0x00e5
#define CHAR_AscendingEllipsis 0x22f0
#define CHAR_ATilde 0x00e3
#define CHAR_Backslash 0x2216
#define CHAR_BeamedEighthNote 0x266b
#define CHAR_BeamedSixteenthNote 0x266c
#define CHAR_Because 0x2235
#define CHAR_Bet 0x2136
#define CHAR_Beta 0x03b2
#define CHAR_BlackBishop 0x265d
#define CHAR_BlackKing 0x265a
#define CHAR_BlackKnight 0x265e
#define CHAR_BlackPawn 0x265f
#define CHAR_BlackQueen 0x265b
#define CHAR_BlackRook 0x265c
#define CHAR_Breve 0x02d8
#define CHAR_Bullet 0x2022
#define CHAR_CAcute 0x0107
#define CHAR_CapitalAAcute 0x00c1
#define CHAR_CapitalABar 0x0100
#define CHAR_CapitalACup 0x0102
#define CHAR_CapitalADoubleDot 0x00c4
#define CHAR_CapitalAE 0x00c6
#define CHAR_CapitalAGrave 0x00c0
#define CHAR_CapitalAHat 0x00c2
#define CHAR_CapitalAlpha 0x0391
#define CHAR_CapitalARing 0x00c5
#define CHAR_CapitalATilde 0x00c3
#define CHAR_CapitalBeta 0x0392
#define CHAR_CapitalCAcute 0x0106
#define CHAR_CapitalCCedilla 0x00c7
#define CHAR_CapitalCHacek 0x010c
#define CHAR_CapitalChi 0x03a7
#define CHAR_CapitalDelta 0x0394
#define CHAR_CapitalDHacek 0x010e
#define CHAR_CapitalDifferentialD 0xf74b
#define CHAR_CapitalDigamma 0x03dc
#define CHAR_CapitalEAcute 0x00c9
#define CHAR_CapitalEBar 0x0112
#define CHAR_CapitalECup 0x0114
#define CHAR_CapitalEDoubleDot 0x00cb
#define CHAR_CapitalEGrave 0x00c8
#define CHAR_CapitalEHacek 0x011a
#define CHAR_CapitalEHat 0x00ca
#define CHAR_CapitalEpsilon 0x0395
#define CHAR_CapitalEta 0x0397
#define CHAR_CapitalEth 0x00d0
#define CHAR_CapitalGamma 0x0393
#define CHAR_CapitalIAcute 0x00cd
#define CHAR_CapitalICup 0x012c
#define CHAR_CapitalIDoubleDot 0x00cf
#define CHAR_CapitalIGrave 0x00cc
#define CHAR_CapitalIHat 0x00ce
#define CHAR_CapitalIota 0x0399
#define CHAR_CapitalKappa 0x039a
#define CHAR_CapitalKoppa 0x03de
#define CHAR_CapitalLambda 0x039b
#define CHAR_CapitalLSlash 0x0141
#define CHAR_CapitalMu 0x039c
#define CHAR_CapitalNHacek 0x0147
#define CHAR_CapitalNTilde 0x00d1
#define CHAR_CapitalNu 0x039d
#define CHAR_CapitalOAcute 0x00d3
#define CHAR_CapitalODoubleAcute 0x0150
#define CHAR_CapitalODoubleDot 0x00d6
#define CHAR_CapitalOE 0x0152
#define CHAR_CapitalOGrave 0x00d2
#define CHAR_CapitalOHat 0x00d4
#define CHAR_CapitalOmega 0x03a9
#define CHAR_CapitalOmicron 0x039f
#define CHAR_CapitalOSlash 0x00d8
#define CHAR_CapitalOTilde 0x00d5
#define CHAR_CapitalPhi 0x03a6
#define CHAR_CapitalPi 0x03a0
#define CHAR_CapitalPsi 0x03a8
#define CHAR_CapitalRHacek 0x0158
#define CHAR_CapitalRho 0x03a1
#define CHAR_CapitalSampi 0x03e0
#define CHAR_CapitalSHacek 0x0160
#define CHAR_CapitalSigma 0x03a3
#define CHAR_CapitalStigma 0x03da
#define CHAR_CapitalTau 0x03a4
#define CHAR_CapitalTHacek 0x0164
#define CHAR_CapitalTheta 0x0398
#define CHAR_CapitalThorn 0x00de
#define CHAR_CapitalUAcute 0x00da
#define CHAR_CapitalUDoubleAcute 0x0170
#define CHAR_CapitalUDoubleDot 0x00dc
#define CHAR_CapitalUGrave 0x00d9
#define CHAR_CapitalUHat 0x00db
#define CHAR_CapitalUpsilon 0x03a5
#define CHAR_CapitalURing 0x016e
#define CHAR_CapitalXi 0x039e
#define CHAR_CapitalYAcute 0x00dd
#define CHAR_CapitalZeta 0x0396
#define CHAR_CapitalZHacek 0x017d
#define CHAR_Cap 0x2322
#define CHAR_CCedilla 0x00e7
#define CHAR_Cedilla 0x00b8
#define CHAR_CenterDot 0x00b7
#define CHAR_CenterEllipsis 0x22ef
#define CHAR_Cent 0x00a2
#define CHAR_CHacek 0x010d
#define CHAR_Checkmark 0x2713
#define CHAR_Chi 0x03c7
#define CHAR_CircleDot 0x2299
#define CHAR_CircleMinus 0x2296
#define CHAR_CirclePlus 0x2295
#define CHAR_CircleTimes 0x2297
#define CHAR_ClockwiseContourIntegral 0x2232
#define CHAR_CloseCurlyDoubleQuote 0x201d
#define CHAR_CloseCurlyQuote 0x2019
#define CHAR_CloverLeaf 0x2318
#define CHAR_ClubSuit 0x2663
#define CHAR_Colon 0x2236
#define CHAR_CommandKey 0xf76a
#define CHAR_Congruent 0x2261
#define CHAR_Conjugate 0xf3c8
#define CHAR_ConjugateTranspose 0xf3c9
#define CHAR_ConstantC 0xf7da
#define CHAR_Continuation 0xf3b1
#define CHAR_ContourIntegral 0x222e
#define CHAR_ControlKey 0xf763
#define CHAR_Coproduct 0x2210
#define CHAR_Copyright 0x00a9
#define CHAR_CounterClockwiseContourIntegral 0x2233
#define CHAR_Cross 0xf4a0
#define CHAR_CupCap 0x224d
#define CHAR_Cup 0x2323
#define CHAR_CurlyCapitalUpsilon 0x03d2
#define CHAR_CurlyEpsilon 0x03b5
#define CHAR_CurlyKappa 0x03f0
#define CHAR_CurlyPhi 0x03c6
#define CHAR_CurlyPi 0x03d6
#define CHAR_CurlyRho 0x03f1
#define CHAR_CurlyTheta 0x03d1
#define CHAR_Currency 0x00a4
#define CHAR_Dagger 0x2020
#define CHAR_Dalet 0x2138
#define CHAR_Dash 0x2013
#define CHAR_Degree 0x00b0
#define CHAR_DeleteKey 0xf7d0
#define CHAR_Del 0x2207
#define CHAR_Delta 0x03b4
#define CHAR_DescendingEllipsis 0x22f1
#define CHAR_DHacek 0x010f
#define CHAR_Diameter 0x2300
#define CHAR_Diamond 0x22c4
#define CHAR_DiamondSuit 0x2662
#define CHAR_DifferenceDelta 0xf4a4
#define CHAR_DifferentialD 0xf74c
#define CHAR_Digamma 0x03dd
#define CHAR_DiscreteRatio 0xf4a5
#define CHAR_DiscreteShift 0xf4a3
#define CHAR_Divide 0x00f7
#define CHAR_DotEqual 0x2250
#define CHAR_DotlessI 0x0131
#define CHAR_DotlessJ 0xf700
#define CHAR_DottedSquare 0xf751
#define CHAR_DoubleContourIntegral 0x222f
#define CHAR_DoubleDagger 0x2021
#define CHAR_DoubledGamma 0xf74a
#define CHAR_DoubleDownArrow 0x21d3
#define CHAR_DoubledPi 0xf749
#define CHAR_DoubleLeftArrow 0x21d0
#define CHAR_DoubleLeftRightArrow 0x21d4
#define CHAR_DoubleLeftTee 0x2ae4
#define CHAR_DoubleLongLeftArrow 0x27f8
#define CHAR_DoubleLongLeftRightArrow 0x27fa
#define CHAR_DoubleLongRightArrow 0x27f9
#define CHAR_DoublePrime 0x2033
#define CHAR_DoubleRightArrow 0x21d2
#define CHAR_DoubleRightTee 0x22a8
#define CHAR_DoubleStruckA 0xf6e6
#define CHAR_DoubleStruckB 0xf6e7
#define CHAR_DoubleStruckC 0xf6e8
#define CHAR_DoubleStruckCapitalA 0xf7a4
#define CHAR_DoubleStruckCapitalB 0xf7a5
#define CHAR_DoubleStruckCapitalC 0xf7a6
#define CHAR_DoubleStruckCapitalD 0xf7a7
#define CHAR_DoubleStruckCapitalE 0xf7a8
#define CHAR_DoubleStruckCapitalF 0xf7a9
#define CHAR_DoubleStruckCapitalG 0xf7aa
#define CHAR_DoubleStruckCapitalH 0xf7ab
#define CHAR_DoubleStruckCapitalI 0xf7ac
#define CHAR_DoubleStruckCapitalJ 0xf7ad
#define CHAR_DoubleStruckCapitalK 0xf7ae
#define CHAR_DoubleStruckCapitalL 0xf7af
#define CHAR_DoubleStruckCapitalM 0xf7b0
#define CHAR_DoubleStruckCapitalN 0xf7b1
#define CHAR_DoubleStruckCapitalO 0xf7b2
#define CHAR_DoubleStruckCapitalP 0xf7b3
#define CHAR_DoubleStruckCapitalQ 0xf7b4
#define CHAR_DoubleStruckCapitalR 0xf7b5
#define CHAR_DoubleStruckCapitalS 0xf7b6
#define CHAR_DoubleStruckCapitalT 0xf7b7
#define CHAR_DoubleStruckCapitalU 0xf7b8
#define CHAR_DoubleStruckCapitalV 0xf7b9
#define CHAR_DoubleStruckCapitalW 0xf7ba
#define CHAR_DoubleStruckCapitalX 0xf7bb
#define CHAR_DoubleStruckCapitalY 0xf7bc
#define CHAR_DoubleStruckCapitalZ 0xf7bd
#define CHAR_DoubleStruckD 0xf6e9
#define CHAR_DoubleStruckE 0xf6ea
#define CHAR_DoubleStruckEight 0xf7e3
#define CHAR_DoubleStruckF 0xf6eb
#define CHAR_DoubleStruckFive 0xf7e0
#define CHAR_DoubleStruckFour 0xf7df
#define CHAR_DoubleStruckG 0xf6ec
#define CHAR_DoubleStruckH 0xf6ed
#define CHAR_DoubleStruckI 0xf6ee
#define CHAR_DoubleStruckJ 0xf6ef
#define CHAR_DoubleStruckK 0xf6f0
#define CHAR_DoubleStruckL 0xf6f1
#define CHAR_DoubleStruckM 0xf6f2
#define CHAR_DoubleStruckN 0xf6f3
#define CHAR_DoubleStruckNine 0xf7e4
#define CHAR_DoubleStruckO 0xf6f4
#define CHAR_DoubleStruckOne 0xf7dc
#define CHAR_DoubleStruckP 0xf6f5
#define CHAR_DoubleStruckQ 0xf6f6
#define CHAR_DoubleStruckR 0xf6f7
#define CHAR_DoubleStruckS 0xf6f8
#define CHAR_DoubleStruckSeven 0xf7e2
#define CHAR_DoubleStruckSix 0xf7e1
#define CHAR_DoubleStruckT 0xf6f9
#define CHAR_DoubleStruckThree 0xf7de
#define CHAR_DoubleStruckTwo 0xf7dd
#define CHAR_DoubleStruckU 0xf6fa
#define CHAR_DoubleStruckV 0xf6fb
#define CHAR_DoubleStruckW 0xf6fc
#define CHAR_DoubleStruckX 0xf6fd
#define CHAR_DoubleStruckY 0xf6fe
#define CHAR_DoubleStruckZ 0xf6ff
#define CHAR_DoubleStruckZero 0xf7db
#define CHAR_DoubleUpArrow 0x21d1
#define CHAR_DoubleUpDownArrow 0x21d5
#define CHAR_DoubleVerticalBar 0x2225
#define CHAR_DownArrowBar 0x2913
#define CHAR_DownArrow 0x2193
#define CHAR_DownArrowUpArrow 0x21f5
#define CHAR_DownBreve 0xf755
#define CHAR_DownExclamation 0x00a1
#define CHAR_DownLeftRightVector 0x2950
#define CHAR_DownLeftTeeVector 0x295e
#define CHAR_DownLeftVector 0x21bd
#define CHAR_DownLeftVectorBar 0x2956
#define CHAR_DownPointer 0x25be
#define CHAR_DownQuestion 0x00bf
#define CHAR_DownRightTeeVector 0x295f
#define CHAR_DownRightVector 0x21c1
#define CHAR_DownRightVectorBar 0x2957
#define CHAR_DownTeeArrow 0x21a7
#define CHAR_DownTee 0x22a4
#define CHAR_EAcute 0x00e9
#define CHAR_Earth 0xf3df
#define CHAR_EBar 0x0113
#define CHAR_ECup 0x0115
#define CHAR_EDoubleDot 0x00eb
#define CHAR_EGrave 0x00e8
#define CHAR_EHacek 0x011b
#define CHAR_EHat 0x00ea
#define CHAR_EighthNote 0x266a
#define CHAR_Element 0x2208
#define CHAR_Ellipsis 0x2026
#define CHAR_EmptyCircle 0x25cb
#define CHAR_EmptyDiamond 0x25c7
#define CHAR_EmptyDownTriangle 0x25bd
#define CHAR_EmptyRectangle 0x25af
#define CHAR_EmptySet 0x2205
#define CHAR_EmptySmallCircle 0x25e6
#define CHAR_EmptySmallSquare 0x25fb
#define CHAR_EmptySquare 0x25a1
#define CHAR_EmptyUpTriangle 0x25b3
#define CHAR_EmptyVerySmallSquare 0x25ab
#define CHAR_EnterKey 0xf7d4
#define CHAR_EntityEnd 0xf3b9
#define CHAR_EntityStart 0xf3b8
#define CHAR_Epsilon 0x03f5
#define CHAR_Equal 0xf431
#define CHAR_EqualTilde 0x2242
#define CHAR_Equilibrium 0x21cc
#define CHAR_Equivalent 0x29e6
#define CHAR_ErrorIndicator 0xf767
#define CHAR_EscapeKey 0xf769
#define CHAR_Eta 0x03b7
#define CHAR_Eth 0x00f0
#define CHAR_Euro 0x20ac
#define CHAR_Exists 0x2203
#define CHAR_ExponentialE 0xf74d
#define CHAR_FiLigature 0xfb01
#define CHAR_FilledCircle 0x25cf
#define CHAR_FilledDiamond 0x25c6
#define CHAR_FilledDownTriangle 0x25bc
#define CHAR_FilledLeftTriangle 0x25c0
#define CHAR_FilledRectangle 0x25ae
#define CHAR_FilledRightTriangle 0x25b6
#define CHAR_FilledSmallCircle 0xf750
#define CHAR_FilledSmallSquare 0x25fc
#define CHAR_FilledSquare 0x25a0
#define CHAR_FilledUpTriangle 0x25b2
#define CHAR_FilledVerySmallSquare 0x25aa
#define CHAR_FinalSigma 0x03c2
#define CHAR_FirstPage 0xf7fa
#define CHAR_FivePointedStar 0x2605
#define CHAR_Flat 0x266d
#define CHAR_FlLigature 0xfb02
#define CHAR_Florin 0x0192
#define CHAR_ForAll 0x2200
#define CHAR_FreakedSmiley 0xf721
#define CHAR_Function 0xf4a1
#define CHAR_Gamma 0x03b3
#define CHAR_Gimel 0x2137
#define CHAR_GothicA 0xf6cc
#define CHAR_GothicB 0xf6cd
#define CHAR_GothicC 0xf6ce
#define CHAR_GothicCapitalA 0xf78a
#define CHAR_GothicCapitalB 0xf78b
#define CHAR_GothicCapitalC 0x212d
#define CHAR_GothicCapitalD 0xf78d
#define CHAR_GothicCapitalE 0xf78e
#define CHAR_GothicCapitalF 0xf78f
#define CHAR_GothicCapitalG 0xf790
#define CHAR_GothicCapitalH 0x210c
#define CHAR_GothicCapitalI 0x2111
#define CHAR_GothicCapitalJ 0xf793
#define CHAR_GothicCapitalK 0xf794
#define CHAR_GothicCapitalL 0xf795
#define CHAR_GothicCapitalM 0xf796
#define CHAR_GothicCapitalN 0xf797
#define CHAR_GothicCapitalO 0xf798
#define CHAR_GothicCapitalP 0xf799
#define CHAR_GothicCapitalQ 0xf79a
#define CHAR_GothicCapitalR 0x211c
#define CHAR_GothicCapitalS 0xf79c
#define CHAR_GothicCapitalT 0xf79d
#define CHAR_GothicCapitalU 0xf79e
#define CHAR_GothicCapitalV 0xf79f
#define CHAR_GothicCapitalW 0xf7a0
#define CHAR_GothicCapitalX 0xf7a1
#define CHAR_GothicCapitalY 0xf7a2
#define CHAR_GothicCapitalZ 0x2128
#define CHAR_GothicD 0xf6cf
#define CHAR_GothicE 0xf6d0
#define CHAR_GothicEight 0xf7ed
#define CHAR_GothicF 0xf6d1
#define CHAR_GothicFive 0xf7ea
#define CHAR_GothicFour 0xf7e9
#define CHAR_GothicG 0xf6d2
#define CHAR_GothicH 0xf6d3
#define CHAR_GothicI 0xf6d4
#define CHAR_GothicJ 0xf6d5
#define CHAR_GothicK 0xf6d6
#define CHAR_GothicL 0xf6d7
#define CHAR_GothicM 0xf6d8
#define CHAR_GothicN 0xf6d9
#define CHAR_GothicNine 0xf7ef
#define CHAR_GothicO 0xf6da
#define CHAR_GothicOne 0xf7e6
#define CHAR_GothicP 0xf6db
#define CHAR_GothicQ 0xf6dc
#define CHAR_GothicR 0xf6dd
#define CHAR_GothicS 0xf6de
#define CHAR_GothicSeven 0xf7ec
#define CHAR_GothicSix 0xf7eb
#define CHAR_GothicT 0xf6df
#define CHAR_GothicThree 0xf7e8
#define CHAR_GothicTwo 0xf7e7
#define CHAR_GothicU 0xf6e0
#define CHAR_GothicV 0xf6e1
#define CHAR_GothicW 0xf6e2
#define CHAR_GothicX 0xf6e3
#define CHAR_GothicY 0xf6e4
#define CHAR_GothicZ 0xf6e5
#define CHAR_GothicZero 0xf7e5
#define CHAR_GreaterEqualLess 0x22db
#define CHAR_GreaterEqual 0x2265
#define CHAR_GreaterFullEqual 0x2267
#define CHAR_GreaterGreater 0x226b
#define CHAR_GreaterLess 0x2277
#define CHAR_GreaterSlantEqual 0x2a7e
#define CHAR_GreaterTilde 0x2273
#define CHAR_Hacek 0x02c7
#define CHAR_HappySmiley 0x263a
#define CHAR_HBar 0x210f
#define CHAR_HeartSuit 0x2661
#define CHAR_HermitianConjugate 0xf3ce
#define CHAR_HumpDownHump 0x224e
#define CHAR_HumpEqual 0x224f
#define CHAR_Hyphen 0x2010
#define CHAR_IAcute 0x00ed
#define CHAR_ICup 0x012d
#define CHAR_IDoubleDot 0x00ef
#define CHAR_IGrave 0x00ec
#define CHAR_IHat 0x00ee
#define CHAR_ImaginaryI 0xf74e
#define CHAR_ImaginaryJ 0xf74f
#define CHAR_Implies 0xf523
#define CHAR_Infinity 0x221e
#define CHAR_Intersection 0x22c2
#define CHAR_Iota 0x03b9
#define CHAR_Jupiter 0x2643
#define CHAR_Kappa 0x03ba
#define CHAR_KernelIcon 0xf756
#define CHAR_Koppa 0x03df
#define CHAR_Lambda 0x03bb
#define CHAR_LastPage 0xf7fb
#define CHAR_LeftAngleBracket 0x2329
#define CHAR_LeftArrowBar 0x21e4
#define CHAR_LeftArrow 0x2190
#define CHAR_LeftArrowRightArrow 0x21c6
#define CHAR_LeftAssociation 0xf113
#define CHAR_LeftBracketingBar 0xf603
#define CHAR_LeftCeiling 0x2308
#define CHAR_LeftDoubleBracket 0x301a
#define CHAR_LeftDoubleBracketingBar 0xf605
#define CHAR_LeftDownTeeVector 0x2961
#define CHAR_LeftDownVectorBar 0x2959
#define CHAR_LeftDownVector 0x21c3
#define CHAR_LeftFloor 0x230a
#define CHAR_LeftGuillemet 0x00ab
#define CHAR_LeftModified 0xf76b
#define CHAR_LeftPointer 0x25c2
#define CHAR_LeftRightArrow 0x2194
#define CHAR_LeftRightVector 0x294e
#define CHAR_LeftSkeleton 0xf761
#define CHAR_LeftTee 0x22a3
#define CHAR_LeftTeeArrow 0x21a4
#define CHAR_LeftTeeVector 0x295a
#define CHAR_LeftTriangle 0x22b2
#define CHAR_LeftTriangleBar 0x29cf
#define CHAR_LeftTriangleEqual 0x22b4
#define CHAR_LeftUpDownVector 0x2951
#define CHAR_LeftUpTeeVector 0x2960
#define CHAR_LeftUpVector 0x21bf
#define CHAR_LeftUpVectorBar 0x2958
#define CHAR_LeftVector 0x21bc
#define CHAR_LeftVectorBar 0x2952
#define CHAR_LessEqual 0x2264
#define CHAR_LessEqualGreater 0x22da
#define CHAR_LessFullEqual 0x2266
#define CHAR_LessGreater 0x2276
#define CHAR_LessLess 0x226a
#define CHAR_LessSlantEqual 0x2a7d
#define CHAR_LessTilde 0x2272
#define CHAR_LightBulb 0xf723
#define CHAR_LongDash 0x2014
#define CHAR_LongEqual 0xf7d9
#define CHAR_LongLeftArrow 0x27f5
#define CHAR_LongLeftRightArrow 0x27f7
#define CHAR_LongRightArrow 0x27f6
#define CHAR_LowerLeftArrow 0x2199
#define CHAR_LowerRightArrow 0x2198
#define CHAR_LSlash 0x0142
#define CHAR_Mars 0x2642
#define CHAR_MathematicaIcon 0xf757
#define CHAR_MeasuredAngle 0x2221
#define CHAR_Mercury 0x263f
#define CHAR_Mho 0x2127
#define CHAR_Micro 0x00b5
#define CHAR_MinusPlus 0x2213
#define CHAR_Mu 0x03bc
#define CHAR_Nand 0x22bc
#define CHAR_Natural 0x266e
#define CHAR_Neptune 0x2646
#define CHAR_NestedGreaterGreater 0x2aa2
#define CHAR_NestedLessLess 0x2aa1
#define CHAR_NeutralSmiley 0xf722
#define CHAR_NHacek 0x0148
#define CHAR_Nor 0x22bd
#define CHAR_NotCongruent 0x2262
#define CHAR_NotCupCap 0x226d
#define CHAR_NotDoubleVerticalBar 0x2226
#define CHAR_NotElement 0x2209
#define CHAR_NotEqual 0x2260
#define CHAR_NotEqualTilde 0xf400
#define CHAR_NotExists 0x2204
#define CHAR_NotGreater 0x226f
#define CHAR_NotGreaterEqual 0x2271
#define CHAR_NotGreaterFullEqual 0x2269
#define CHAR_NotGreaterGreater 0xf427
#define CHAR_NotGreaterLess 0x2279
#define CHAR_NotGreaterSlantEqual 0xf429
#define CHAR_NotGreaterTilde 0x2275
#define CHAR_NotHumpDownHump 0xf402
#define CHAR_NotHumpEqual 0xf401
#define CHAR_NotLeftTriangle 0x22ea
#define CHAR_NotLeftTriangleBar 0xf412
#define CHAR_NotLeftTriangleEqual 0x22ec
#define CHAR_NotLessEqual 0x2270
#define CHAR_NotLessFullEqual 0x2268
#define CHAR_NotLessGreater 0x2278
#define CHAR_NotLess 0x226e
#define CHAR_NotLessLess 0xf422
#define CHAR_NotLessSlantEqual 0xf424
#define CHAR_NotLessTilde 0x2274
#define CHAR_Not 0x00ac
#define CHAR_NotNestedGreaterGreater 0xf428
#define CHAR_NotNestedLessLess 0xf423
#define CHAR_NotPrecedes 0x2280
#define CHAR_NotPrecedesEqual 0xf42b
#define CHAR_NotPrecedesSlantEqual 0x22e0
#define CHAR_NotPrecedesTilde 0x22e8
#define CHAR_NotReverseElement 0x220c
#define CHAR_NotRightTriangle 0x22eb
#define CHAR_NotRightTriangleBar 0xf413
#define CHAR_NotRightTriangleEqual 0x22ed
#define CHAR_NotSquareSubset 0xf42e
#define CHAR_NotSquareSubsetEqual 0x22e2
#define CHAR_NotSquareSuperset 0xf42f
#define CHAR_NotSquareSupersetEqual 0x22e3
#define CHAR_NotSubset 0x2284
#define CHAR_NotSubsetEqual 0x2288
#define CHAR_NotSucceeds 0x2281
#define CHAR_NotSucceedsEqual 0xf42d
#define CHAR_NotSucceedsSlantEqual 0x22e1
#define CHAR_NotSucceedsTilde 0x22e9
#define CHAR_NotSuperset 0x2285
#define CHAR_NotSupersetEqual 0x2289
#define CHAR_NotTilde 0x2241
#define CHAR_NotTildeEqual 0x2244
#define CHAR_NotTildeFullEqual 0x2247
#define CHAR_NotTildeTilde 0x2249
#define CHAR_NotVerticalBar 0xf3d1
#define CHAR_NTilde 0x00f1
#define CHAR_Nu 0x03bd
#define CHAR_NumberSign 0xf724
#define CHAR_OAcute 0x00f3
#define CHAR_ODoubleAcute 0x0151
#define CHAR_ODoubleDot 0x00f6
#define CHAR_OE 0x0153
#define CHAR_OGrave 0x00f2
#define CHAR_OHat 0x00f4
#define CHAR_Omega 0x03c9
#define CHAR_Omicron 0x03bf
#define CHAR_OpenCurlyQuote 0x2018
#define CHAR_OptionKey 0xf7d2
#define CHAR_Or 0x2228
#define CHAR_OSlash 0x00f8
#define CHAR_OTilde 0x00f5
#define CHAR_OverBrace 0xfe37
#define CHAR_OverBracket 0x23b4
#define CHAR_OverParenthesis 0xfe35
#define CHAR_Paragraph 0x00b6
#define CHAR_PartialD 0x2202
#define CHAR_Phi 0x03d5
#define CHAR_Pi 0x03c0
#define CHAR_Piecewise 0xf361
#define CHAR_Placeholder 0xf528
#define CHAR_PlusMinus 0x00b1
#define CHAR_Pluto 0x2647
#define CHAR_Precedes 0x227a
#define CHAR_PrecedesEqual 0x2aaf
#define CHAR_PrecedesSlantEqual 0x227c
#define CHAR_PrecedesTilde 0x227e
#define CHAR_Prime 0x2032
#define CHAR_Proportion 0x2237
#define CHAR_Proportional 0x221d
#define CHAR_Psi 0x03c8
#define CHAR_QuarterNote 0x2669
#define CHAR_RegisteredTrademark 0x00ae
#define CHAR_ReturnIndicator 0x21b5
#define CHAR_ReturnKey 0xf766
#define CHAR_ReverseDoublePrime 0x2036
#define CHAR_ReverseElement 0x220b
#define CHAR_ReverseEquilibrium 0x21cb
#define CHAR_ReversePrime 0x2035
#define CHAR_ReverseUpEquilibrium 0x296f
#define CHAR_RHacek 0x0159
#define CHAR_Rho 0x03c1
#define CHAR_RightAngle 0x221f
#define CHAR_RightAngleBracket 0x232a
#define CHAR_RightArrow 0x2192
#define CHAR_RightArrowBar 0x21e5
#define CHAR_RightArrowLeftArrow 0x21c4
#define CHAR_RightAssociation 0xf114
#define CHAR_RightBracketingBar 0xf604
#define CHAR_RightCeiling 0x2309
#define CHAR_RightDoubleBracket 0x301b
#define CHAR_RightDoubleBracketingBar 0xf606
#define CHAR_RightDownTeeVector 0x295d
#define CHAR_RightDownVector 0x21c2
#define CHAR_RightDownVectorBar 0x2955
#define CHAR_RightFloor 0x230b
#define CHAR_RightGuillemet 0x00bb
#define CHAR_RightModified 0xf76c
#define CHAR_RightPointer 0x25b8
#define CHAR_RightSkeleton 0xf762
#define CHAR_RightTee 0x22a2
#define CHAR_RightTeeArrow 0x21a6
#define CHAR_RightTeeVector 0x295b
#define CHAR_RightTriangle 0x22b3
#define CHAR_RightTriangleBar 0x29d0
#define CHAR_RightTriangleEqual 0x22b5
#define CHAR_RightUpDownVector 0x294f
#define CHAR_RightUpTeeVector 0x295c
#define CHAR_RightUpVector 0x21be
#define CHAR_RightUpVectorBar 0x2954
#define CHAR_RightVector 0x21c0
#define CHAR_RightVectorBar 0x2953
#define CHAR_RoundImplies 0x2970
#define CHAR_RoundSpaceIndicator 0xf3b2
#define CHAR_Rule 0xf522
#define CHAR_RuleDelayed 0xf51f
#define CHAR_SadSmiley 0x2639
#define CHAR_Sampi 0x03e1
#define CHAR_Saturn 0x2644
#define CHAR_ScriptA 0xf6b2
#define CHAR_ScriptB 0xf6b3
#define CHAR_ScriptC 0xf6b4
#define CHAR_ScriptCapitalA 0xf770
#define CHAR_ScriptCapitalB 0x212c
#define CHAR_ScriptCapitalC 0xf772
#define CHAR_ScriptCapitalD 0xf773
#define CHAR_ScriptCapitalE 0x2130
#define CHAR_ScriptCapitalF 0x2131
#define CHAR_ScriptCapitalG 0xf776
#define CHAR_ScriptCapitalH 0x210b
#define CHAR_ScriptCapitalI 0x2110
#define CHAR_ScriptCapitalJ 0xf779
#define CHAR_ScriptCapitalK 0xf77a
#define CHAR_ScriptCapitalL 0x2112
#define CHAR_ScriptCapitalM 0x2133
#define CHAR_ScriptCapitalN 0xf77d
#define CHAR_ScriptCapitalO 0xf77e
#define CHAR_ScriptCapitalP 0xf77f
#define CHAR_ScriptCapitalQ 0xf780
#define CHAR_ScriptCapitalR 0x211b
#define CHAR_ScriptCapitalS 0xf782
#define CHAR_ScriptCapitalT 0xf783
#define CHAR_ScriptCapitalU 0xf784
#define CHAR_ScriptCapitalV 0xf785
#define CHAR_ScriptCapitalW 0xf786
#define CHAR_ScriptCapitalX 0xf787
#define CHAR_ScriptCapitalY 0xf788
#define CHAR_ScriptCapitalZ 0xf789
#define CHAR_ScriptD 0xf6b5
#define CHAR_ScriptDotlessI 0xf730
#define CHAR_ScriptDotlessJ 0xf731
#define CHAR_ScriptE 0x212f
#define CHAR_ScriptEight 0xf7f8
#define CHAR_ScriptF 0xf6b7
#define CHAR_ScriptFive 0xf7f5
#define CHAR_ScriptFour 0xf7f4
#define CHAR_ScriptG 0x210a
#define CHAR_ScriptH 0xf6b9
#define CHAR_ScriptI 0xf6ba
#define CHAR_ScriptJ 0xf6bb
#define CHAR_ScriptK 0xf6bc
#define CHAR_ScriptL 0x2113
#define CHAR_ScriptM 0xf6be
#define CHAR_ScriptN 0xf6bf
#define CHAR_ScriptNine 0xf7f9
#define CHAR_ScriptO 0x2134
#define CHAR_ScriptOne 0xf7f1
#define CHAR_ScriptP 0xf6c1
#define CHAR_ScriptQ 0xf6c2
#define CHAR_ScriptR 0xf6c3
#define CHAR_ScriptS 0xf6c4
#define CHAR_ScriptSeven 0xf7f7
#define CHAR_ScriptSix 0xf7f6
#define CHAR_ScriptT 0xf6c5
#define CHAR_ScriptThree 0xf7f3
#define CHAR_ScriptTwo 0xf7f2
#define CHAR_ScriptU 0xf6c6
#define CHAR_ScriptV 0xf6c7
#define CHAR_ScriptW 0xf6c8
#define CHAR_ScriptX 0xf6c9
#define CHAR_ScriptY 0xf6ca
#define CHAR_ScriptZ 0xf6cb
#define CHAR_ScriptZero 0xf7f0
#define CHAR_Section 0x00a7
#define CHAR_SelectionPlaceholder 0xf527
#define CHAR_SHacek 0x0161
#define CHAR_Sharp 0x266f
#define CHAR_ShortLeftArrow 0xf526
#define CHAR_ShortRightArrow 0xf525
#define CHAR_Sigma 0x03c3
#define CHAR_SixPointedStar 0x2736
#define CHAR_SkeletonIndicator 0x2043
#define CHAR_SmallCircle 0x2218
#define CHAR_SpaceIndicator 0x2423
#define CHAR_SpaceKey 0xf7bf
#define CHAR_SpadeSuit 0x2660
#define CHAR_SpanFromAbove 0xf3bb
#define CHAR_SpanFromBoth 0xf3bc
#define CHAR_SpanFromLeft 0xf3ba
#define CHAR_SphericalAngle 0x2222
#define CHAR_Sqrt 0x221a
#define CHAR_Square 0xf520
#define CHAR_SquareIntersection 0x2293
#define CHAR_SquareSubset 0x228f
#define CHAR_SquareSubsetEqual 0x2291
#define CHAR_SquareSuperset 0x2290
#define CHAR_SquareSupersetEqual 0x2292
#define CHAR_SquareUnion 0x2294
#define CHAR_Star 0x22c6
#define CHAR_Sterling 0x00a3
#define CHAR_Stigma 0x03db
#define CHAR_Subset 0x2282
#define CHAR_SubsetEqual 0x2286
#define CHAR_Succeeds 0x227b
#define CHAR_SucceedsEqual 0x2ab0
#define CHAR_SucceedsSlantEqual 0x227d
#define CHAR_SucceedsTilde 0x227f
#define CHAR_SuchThat 0x220d
#define CHAR_Superset 0x2283
#define CHAR_SupersetEqual 0x2287
#define CHAR_SystemEnterKey 0xf75f
#define CHAR_SZ 0x00df
#define CHAR_TabKey 0xf7be
#define CHAR_Tau 0x03c4
#define CHAR_THacek 0x0165
#define CHAR_Therefore 0x2234
#define CHAR_Theta 0x03b8
#define CHAR_Thorn 0x00fe
#define CHAR_Tilde 0x223c
#define CHAR_TildeEqual 0x2243
#define CHAR_TildeFullEqual 0x2245
#define CHAR_TildeTilde 0x2248
#define CHAR_Times 0x00d7
#define CHAR_Trademark 0x2122
#define CHAR_Transpose 0xf3c7
#define CHAR_UAcute 0x00fa
#define CHAR_UDoubleAcute 0x0171
#define CHAR_UDoubleDot 0x00fc
#define CHAR_UGrave 0x00f9
#define CHAR_UHat 0x00fb
#define CHAR_UnderBrace 0xfe38
#define CHAR_UnderBracket 0x23b5
#define CHAR_UnderParenthesis 0xfe36
#define CHAR_Union 0x22c3
#define CHAR_UnionPlus 0x228e
#define CHAR_UpArrow 0x2191
#define CHAR_UpArrowBar 0x2912
#define CHAR_UpArrowDownArrow 0x21c5
#define CHAR_UpDownArrow 0x2195
#define CHAR_UpEquilibrium 0x296e
#define CHAR_UpperLeftArrow 0x2196
#define CHAR_UpperRightArrow 0x2197
#define CHAR_UpPointer 0x25b4
#define CHAR_Upsilon 0x03c5
#define CHAR_UpTee 0x22a5
#define CHAR_UpTeeArrow 0x21a5
#define CHAR_Uranus 0x26e2
#define CHAR_URing 0x016f
#define CHAR_Vee 0x22c1
#define CHAR_Venus 0x2640
#define CHAR_VerticalBar 0xf3d0
#define CHAR_VerticalEllipsis 0x22ee
#define CHAR_VerticalSeparator 0xf432
#define CHAR_VerticalTilde 0x2240
#define CHAR_WarningSign 0xf725
#define CHAR_WatchIcon 0x231a
#define CHAR_Wedge 0x22c0
#define CHAR_WeierstrassP 0x2118
#define CHAR_WhiteBishop 0x2657
#define CHAR_WhiteKing 0x2654
#define CHAR_WhiteKnight 0x2658
#define CHAR_WhitePawn 0x2659
#define CHAR_WhiteQueen 0x2655
#define CHAR_WhiteRook 0x2656
#define CHAR_Wolf 0xf720
#define CHAR_WolframLanguageLogo 0xf11e
#define CHAR_WolframLanguageLogoCircle 0xf11f
#define CHAR_Xi 0x03be
#define CHAR_Xnor 0xf4a2
#define CHAR_Xor 0x22bb
#define CHAR_YAcute 0x00fd
#define CHAR_YDoubleDot 0x00ff
#define CHAR_Yen 0x00a5
#define CHAR_Zeta 0x03b6
#define CHAR_ZHacek 0x017e


#define CMD_SYNTAX 11
#define CMD_EXPR 12

#define _CMD_IN_EVAL 13
#define _CMD_OUT_PRINT 14
#define _CMD_OUT_MESSAGE 15
#define _CMD_OUT_RESULT 16
#define _CMD_OUT_STRING 17
#define _CMD_PRINT_BOXEXPR 18
#define _CMD_EVAL_BOXEXPR 19
#define _CMD_SET_LINE 20

extern FILE * fp_in, * fp_out;
extern int fp_out_type;

#define FP_OUT_NONE 0
#define FP_OUT_BINARY 1
#define FP_OUT_TEXT 2


#define ECACHE_NODE_MAX_LENGTH 4

void*bint_alloc(slong size);
void bint_free(void*addr);

void*e_alloc(ulong size);
void e_free(void*addr);

void*ex_alloc(ulong size);
void ex_free(void*addr);

void ex_debug();



// large chunks

void*h_alloc(ulong size);
void h_free(void*addr);

void*e_alloc(ulong size);
void e_free(void*addr);

void*p_alloc(ulong size);
void p_free(void*addr);


// registers faster
// must be used in stack-like fasion

void r_mem_init();
void r_mem_destroy();
void*r_alloc(ulong size);
void r_free(void*addr, ulong size);

void check_memory_usage();
void print_memory_usage();
