#pragma once

#include "flintarb_wrappers.h"
#include "types.h"
#include "font.h"
#include "ex.h"

void swrite_byte(FILE * fp, uint8_t b);
int  sread_byte(FILE * fp, uint8_t &b);

void swrite_ulong_nocompress(FILE * fp, ulong d);
int  sread_ulong_nocompress(FILE * fp, ulong &d);

void swrite_ulong(FILE * fp, ulong a);
int  sread_ulong(FILE * fp, ulong &a);

void swrite_slong(FILE * fp, slong a);
int  sread_slong(FILE * fp, slong &a);

void swrite_fmpz(FILE * fp, fmpz_t x);
int  sread_fmpz(FILE * fp, fmpz_t x);

void swrite_fmpq(FILE * fp, fmpq_t x);
int  sread_fmpq(FILE * fp, fmpq_t x);

void swrite_arb(FILE * fp, xarb & x);
int  sread_arb(FILE * fp, xarb & x);

void swrite_string(FILE * fp, const std::string &s);
int  sread_string(FILE * fp, std::string &s);

void swrite_double(FILE * fp, double x);
int  sread_double(FILE * fp, double &x);

void swrite_ex(FILE * fp, er e);
int  sread_ex(FILE * fp, uex &e);

//void swrite_mint_wastesignbit(FILE * fp, mintd a);
//int sread_mint_wastesignbit(mint &a, FILE * fp);

//void swrite_charrdata(FILE * pFile, charrdata & c, u64 bits);
//int sread_charrdata(charrdata & c, FILE * pFile, u64 bits);
