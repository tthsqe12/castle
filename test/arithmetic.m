Print["arithmetic"];

Print["... integer arithmetic"];

Assert[1 + 2 == 3];

Do[

    Assert[Sum[n^0, {n, 1, m}] === m];
    Assert[Sum[n^1, {n, 1, m}] === m*(m + 1)/2];
    Assert[Sum[n^2, {n, 1, m}] === m*(m + 1)*(2*m + 1)/6];
    Assert[Sum[n^3, {n, 1, m}] === m^2*(m + 1)^2/4];
, {m, 1, 50}];

Print["... rational arithmetic"];

Do[
    t = Sum[1/n, {n, 1, m}] + Sum[1/n, {n, m + 1, m + m}];
    s = Sum[1/n, {n, 1, 2 m}];
    Assert[t === s]
, {m, 1, 50}];

Do[
    Assert[m/(m + 1) === Sum[1/(n*(n+1)), {n, 1, m}]];
    Assert[m + 1 === Product[1 + 1/n, {n, 1, m}]];
    Assert[Fibonacci[m]/Fibonacci[m + 1] === Nest[1/(1 + #)&, 0, m]]
, {m, 1, 50}];
