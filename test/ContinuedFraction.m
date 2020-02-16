Print["ContinuedFraction"];

Do[
	Assert[ContinuedFraction[Fibonacci[n + 1]/Fibonacci[n]] === Append[ConstantArray[1, n - 2] , 2]]
, {n, 10, 100, 10}]