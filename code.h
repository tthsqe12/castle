#pragma once

#include "types.h"
#include "ex.h"

typedef ex sudcode_fxn(er e) __attribute__((warn_unused_result));
typedef ex ncode_fxn(er e, slong prec) __attribute__((warn_unused_result));

sudcode_fxn dcode_sAccuracy,
			dcode_sAppend, scode_sAppend,
			dcode_sArcCos,
            dcode_sArcCosh,
            dcode_sArcCot,
            dcode_sArcCoth,
            dcode_sArcCsc,
            dcode_sArcCsch,
            dcode_sArcSec,
            dcode_sArcSech,
            dcode_sArcSin,
            dcode_sArcSinh,
            dcode_sArcTan,
            dcode_sArcTanh,
            dcode_sAssert,
            dcode_sAssociation, scode_sAssociation,
            dcode_sAttributes,
            dcode_sBlock,
            dcode_sBreak,
            dcode_sCancel,
            dcode_sCatch,
            dcode_sChampernowneNumber,
            dcode_sClearAll,
            dcode_sClearAttributes,
            dcode_sCoefficientRules,
            dcode_sCompile,
            scode_sCompiledFunction,
            dcode_sCompilePrint,
            dcode_sCompoundExpression,
            dcode_sConstantArray,
            dcode_sContinue,
            dcode_sContinuedFraction,
            dcode_sContourPlot3D,
            dcode_sCos,
            dcode_sCosh,
            dcode_sCot,
            dcode_sCoth,
            dcode_sCsc,
            dcode_sCsch,
			dcode_sCyclotomic,
            dcode_sD,
            dcode_sDedekindSum,
            dcode_sDenominator,
            dcode_sDivide,
            dcode_sDivisors,
            dcode_sDo,
            dcode_sDot,
            dcode_sDownValues,
            dcode_sEqual,
            dcode_sEulerPhi,
            dcode_sExit_Quit,
            dcode_sExp,
            dcode_sExpand,
            dcode_sExtendedGCD,
            dcode_sFactor,
            dcode_sFactorInteger,
            dcode_sFactorTerms,
            dcode_sFactorial,
            dcode_sFibonacci,
            dcode_sFirst,
            dcode_sFlatten,
            dcode_sFloor,
			dcode_sFold,
			dcode_sFoldList,
            dcode_sFor,
            dcode_sFunction, scode_sFunction,
            dcode_sGCD,
            dcode_sGaloisGroup,
            dcode_sGet,
            dcode_sGoto,
            dcode_sGreater,
            dcode_sGreaterEqual,
            dcode_sIdentityMatrix,
            dcode_sIf,
            dcode_sIn_Out,
            dcode_sIntegerQ,
            dcode_sInverse,
            dcode_sJoin,
            dcode_sLCM,
            dcode_sLast,
            dcode_sLog,
            dcode_sLog10,
            dcode_sLength,
            dcode_sLess,
            dcode_sLessEqual,
            dcode_sMap,
            dcode_sMatchQ,
            dcode_sMessage,
            dcode_sMinus,
            dcode_sMod,
            dcode_sModule,
            dcode_sMoebiusMu,
            dcode_sN,
            dcode_sNames,
            dcode_sNest,
			dcode_sNestList,
            dcode_sNextPrime,
            dcode_sNumerator,
            dcode_sOwnValues,
            dcode_sPlus,
            dcode_sPart,
            dcode_sPause,
            dcode_sPlot3D,
            dcode_sPolynomialGCD,
            dcode_sPower, dcode_sPower_2,
			dcode_sPrecision,
            dcode_sPrimeQ,
            dcode_sPrint,
            dcode_sProduct,
            dcode_sProtect,
            dcode_sQuiet,
            dcode_sRandomChoice,
            dcode_sRandomInteger,
            dcode_sRandomReal,
            dcode_sRandomSelection,
			dcode_sRationalize,
            dcode_sRealDigits,
            dcode_sReap,
            dcode_sReplace,
            dcode_sReplaceAll,
            dcode_sReplaceList,
            dcode_sReturn,
            dcode_sRootReduce,
            dcode_sRoots,
            dcode_sSameQ,
            dcode_sSeedRandom,
            dcode_sSec,
            dcode_sSech,
			dcode_sSet,
			dcode_sSetAttributes,
			dcode_sSetDelayed,
            dcode_sSin,
            dcode_sSinh,
            dcode_sSort,
            dcode_sSow,
            dcode_sSqrt,
            dcode_sStringJoin,
            dcode_sStringLength,
            dcode_sStringMatchQ,
            dcode_sStringReplace,
            dcode_sSubValues,
            dcode_sSubsets,
            dcode_sSudokuSolve,
            dcode_sSum,
            dcode_sTable,
            dcode_sTan,
            dcode_sTanh,
			dcode_sTagSet,
			dcode_sTagSetDelayed,
            dcode_sThread,
            dcode_sThrow,
            dcode_sTimes,
            dcode_sTiming,
            dcode_sToCharacterCode,
            dcode_sToExpression,
            dcode_sTogether,
            dcode_sTuples,
            dcode_sUnion,
            dcode_sUnique,
            dcode_sUnsameQ,
            dcode_sUnprotect,
            dcode_sUpValues,
            dcode_sUpdate,
            dcode_sWhich,
            dcode_sWhile,
            dcode_sZeta,
            dcode_iComparePatterns,
            dcode_iQuadraticContinuedFraction,
            dcode_iRealContinuedFraction,
            dcode_dBuckets,
            dcode_dHashCode,
            dcode_devFromPackedArray,
            dcode_devPackedArrayQ,
            dcode_devToPackedArray,
            dcode_msFromGenerators,
            dcode_msFromMemberQ,
            dcode_msGenerators,
            dcode_msGroup,
            dcode_msIndex,
            dcode_msJoin,
            dcode_msLessEqual,
            dcode_msMeet,
            dcode_msMemberQ,
            dcode_msValidGroupQ;

ncode_fxn ncode_sChampernowneNumber;

ex eval_diff(er e, er var);
ex apply_function1(ex F, ex E);
ex apply_function2(ex F, ex E);
ex _apply_function2(er b, er l, er e);
ex apply_compiledfunction(ex F);

ex eflatten_sym(ex E, er head);
void _gen_message(er x, const char * tag, const char * str);
void _gen_message(er x, const char * tag, const char * str, ex E1);
void _gen_message(er x, const char * tag, const char * str, ex E1, ex E2);
void _gen_message(er x, const char * tag, const char * str, ex E1, ex E2, ex E3);
void _gen_message(er x, const char * tag, const char * str, ex E1, ex E2, ex E3, ex E4);

ex _handle_message_argct(er e);
ex _handle_message_argx1(er e);
ex _handle_message_argx2(er e);
ex _handle_message_argx(er e, uint32_t n1);
ex _handle_message_argt(er e, uint32_t n1n2);
ex _handle_message_argb(er e, uint32_t n1n2);
ex _handle_message_argm(er e, uint32_t n1);
ex _handle_message_sym(er e, uint32_t n1);
ex _handle_message_intpm(er e, uint32_t n1);
ex _handle_message_intnm(er e, uint32_t n1);
ex _handle_message_intm(er e, uint32_t n1);
ex _handle_message_ilsmn(er e, uint32_t n1);

int compare_patterns(er p, er q);

void esym_assign_ovalue(er x, ex Rhs);
void esym_assign_ovalue(er x, er rhs);
void esym_assign_dvalue(er x, ex Lhs, ex Rhs);
void esym_assign_dvalue(er x, er lhs, er rhs);
void esym_assign_svalue(er x, ex Lhs, ex Rhs);
void esym_assign_svalue(er x, er lhs, er rhs);
void esym_assign_uvalue(er x, ex Lhs, ex Rhs);
void esym_assign_uvalue(er x, er lhs, er rhs);


ex ex_add(ex a, ex b);
ex ex_add(er a, er b);
ex ex_mul(ex a, ex b);
ex ex_mul(ex a, ex b, ex c);
ex ex_mul(ex a, ex b, ex c, ex d);
ex ex_mul(er a, er b);
ex ex_mul_si(ex a, slong b);
ex ex_sub(ex a, ex b);
ex ex_sub(er a, er b);
ex ex_div(ex a, ex b);
ex ex_div(er a, er b);
ex ex_pow(ex a, ex b);
ex ex_pow(er a, er b);
ex ex_floor(ex a);
ex echange_sign(er E);
