Print["patterns"];

Print["... ReplaceList"];

Do[
    s = ReplaceList[ConstantArray[Pi, n], {x__..}:>{x}];
    Assert[Union[Length /@ s] === Divisors[n]];
, {n, 1, 50}];

Print["... product"];

ClearAll[a, x, y, z];
Assert[(2*x*y*z/. 2*y->a) === a*x*z];
