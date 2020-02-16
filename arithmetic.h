#pragma once

#include "ex_types.h"


double fmpq_get_d(const fmpq_t x);
/*
ex num_AbsInt(ex X);
ex num_AbsRat(ex X);
*/
int num_Cmp2(er x, er y);
bool num_Less2(er x, er y);
bool num_LessEqual2(er x, er y);

ex num_Minus1(er X);
ex num_Plus2(er X, er Y);
ex num_Minus2(er X, er Y);
ex num_Times2(er X, er Y);
ex num_Divide1(er X);
ex num_Divide2(er X, er Y);
ex num_Power2(er X, er Y);


double num_todouble(er x);
double econvert_todouble(er e);
// this should be fixed
inline double num_todouble(ex x){ return num_todouble(etor(x));}

