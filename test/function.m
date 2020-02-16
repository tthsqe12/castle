Print["function"];

Print["... pure functions"];

Assert[Fibonacci[10] === Function[If[# < 2, #, #0[#1 - 1] + #0[#1 - 2]]][10]];

Print["... downvalues"];

ClearAll[f]; f[1] = 1; f[0] = 0; f[x_] := f[x - 1] + f[x - 2];
Assert[Fibonacci[10] === f[10]];

ClearAll[f]; f[1] = 1; f[0] = 0; f[x_] := f[x] = f[x - 1] + f[x - 2];
Assert[Fibonacci[100] === f[100]];

Print["... subvalues"];

ClearAll[f]; f[a_][b_] := a + b;
Assert[f[1][2] === 3];

ClearAll[x, f, fp, fpp, fppp];
f'[x_] := fp[x];
f''[x_] := fpp[x];
f'''[x_] := fppp[x];
Assert[D[f[x^2], x] === 2*x*fp[x^2]];
Assert[D[f[x^2], x, x] === 2*fp[x^2] + 4*x^2*fpp[x^2]];
Assert[D[f[x^2], x, x, x] === 12*x*fpp[x^2] + 8*x^3*fppp[x^2]];

Print["... upvalues"];

ClearAll[arctan];
arctan /: arctan[a_] + arctan[b_] := arctan[(a + b)/(1 - a*b)];
Assert[arctan[1/10] + arctan[1/100] === arctan[110/999]];
Assert[arctan[1/10] + arctan[1/11] + arctan[1/12] === arctan[361/1287]];
