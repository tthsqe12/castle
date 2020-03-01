#pragma once

#include "flintarb_wrappers.h"
#include "types.h"


char* print_hex64short(char* s, uint64_t x);
char* print_hex64(char* s, uint64_t x);
char* int_print(char* s, uint64_t x);
char* int_print(char* s, int64_t x);
char* int_print_full19(char* s, uint64_t x);

std::string hex_tostring(uint64_t x);
std::string hex_tostring_full(uint64_t x);
std::string int_tostring(int64_t x);
std::string uint_tostring(uint64_t x);
std::string size_t_tostring(size_t x);
std::string double_tostring(double x);
std::string double_tostring_full(double x);


ulong fdiv_log2(ulong a, ulong radix);

slong double_digitslen_in_base(ulong Radix);

void my_arb_fmpz_pow_fmpz(arb_t res, const fmpz_t base, const fmpz_t e, slong prec);

void round_digit_string(xfstr & digits, slong max_digits, slong digits_len, xfmpz_t & d);

//void bint_digits(std::vector<mint>& digits, uint64_t* yl, int64_t yn, mintd base);
//void rat_digits(std::vector<mint>& head, std::vector<mint>& tail, mint & e, mprat&f, mintd base);
