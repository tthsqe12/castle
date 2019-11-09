Print["derivative"];

Print["... product rule"];

ClearAll[f, g, x, y, s, t];
Do[
    Assert[0 === Expand[D[f*g,x] - f*D[g,x] - g*D[f,x]]],
{f, {Sin[x], x^4 + Exp[x]}},
{g, {Tan[x] + Cos[y], Log[x] + 1/x^2, ArcCos[s] + Cos[t]}}];


