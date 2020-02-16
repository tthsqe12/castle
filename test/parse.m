Print["parse"];

ClearAll[parse, parsetop, a, b, c, d];
(* parse with newlines as possible new expressions *)
parsetop[x_] := Quiet[ToExpression[x, InputForm, Hold]];
(* parse with newlines as possible multiplications *)
parse[x_] := Quiet[ToExpression[x, StandardForm, Hold]];

Print["... bang"];

Assert[parse["a+b!"] === Hold[a + Factorial[b]]];
Assert[parse["a+b!!"] === Hold[a + Factorial2[b]]];
Assert[parse["a+b! !"] === Hold[a + Factorial[Factorial[b]]]];
Assert[parse["a!=b"] === Hold[Unequal[a, b]]];
Assert[parse["a!!=b"] === Hold[Set[Factorial2[a], b]]];


Print["... newline"];

Assert[parse["a+b
              c+d"] === Hold[a + b*c + d]];
Assert[parsetop["a+b
                 c+d"] === Hold[a + b, c + d]];


Print["... dot"];

Assert[parse["a.b"] === Hold[Dot[a, b]]];
Assert[parse["a.1"] === Hold[Times[a, 0.1]]];
Assert[parse["1.b"] === Hold[Times[1.0, b]]];
Assert[parse["16^^1.b"] === Hold[1.6875]];
Assert[parse[".1/.1"] === Hold[Divide[0.1, 0.1]]];
Assert[parse[".1/. 1"] === Hold[ReplaceAll[0.1, 1]]];
Assert[parse[".1//.1"] === Hold[ReplaceRepeated[0.1, 1]]];

Assert[parse[".."] === $Failed];
Assert[parse["1."] === Hold[1.0]];
Assert[parse[".1"] === Hold[0.1]];
Assert[parse["11"] === Hold[11]];

Assert[parse["..."] === $Failed];
Assert[parse["1.."] === Hold[Repeated[1]]];
Assert[parse[".1."] === $Failed];
Assert[parse["11."] === Hold[11.0]];
Assert[parse["..1"] === $Failed];
Assert[parse["1.1"] === Hold[1.1]];
Assert[parse[".11"] === Hold[0.11]];
Assert[parse["111"] === Hold[111]];

Assert[parse["...."] === $Failed];
Assert[parse["1..."] === Hold[RepeatedNull[1]]];
Assert[parse[".1.."] === Hold[Repeated[0.1]]];
Assert[parse["11.."] === Hold[Repeated[11]]];
Assert[parse["..1."] === $Failed];
Assert[parse["1.1."] === $Failed];
Assert[parse[".11."] === $Failed];
Assert[parse["111."] === Hold[111.0]];
Assert[parse["...1"] === $Failed];
Assert[parse["1..1"] === Hold[Times[Repeated[1], 1]]];
Assert[parse[".1.1"] === Hold[Times[0.1, 0.1]]];
Assert[parse["11.1"] === Hold[11.1]];
Assert[parse["..11"] === $Failed];
Assert[parse["1.11"] === Hold[1.11]];
Assert[parse[".111"] === Hold[0.111]];
Assert[parse["1111"] === Hold[1111]];

Assert[parse["....."] === $Failed];
Assert[parse["1...."] === $Failed];
Assert[parse[".1..."] === Hold[RepeatedNull[0.1]]];
Assert[parse["11..."] === Hold[RepeatedNull[11]]];
Assert[parse["..1.."] === $Failed];
Assert[parse["1.1.."] === Hold[Repeated[1.1]]];
Assert[parse[".11.."] === Hold[Repeated[0.11]]];
Assert[parse["111.."] === Hold[Repeated[111]]];
Assert[parse["...1."] === $Failed];
Assert[parse["1..1."] === Hold[Times[Repeated[1], 1.0]]];
Assert[parse[".1.1."] === $Failed];
Assert[parse["11.1."] === $Failed];
Assert[parse["..11."] === $Failed];
Assert[parse["1.11."] === $Failed];
Assert[parse[".111."] === $Failed];
Assert[parse["1111."] === Hold[1111.0]];
Assert[parse["....1"] === $Failed];
Assert[parse["1...1"] === Hold[Times[RepeatedNull[1], 1]]];
Assert[parse[".1..1"] === Hold[Times[Repeated[0.1], 1]]];
Assert[parse["11..1"] === Hold[Times[Repeated[11], 1]]];
Assert[parse["..1.1"] === $Failed];
Assert[parse["1.1.1"] === Hold[Times[1.1, 0.1]]];
Assert[parse[".11.1"] === Hold[Times[0.11, 0.1]]];
Assert[parse["111.1"] === Hold[111.1]];
Assert[parse["...11"] === $Failed];
Assert[parse["1..11"] === Hold[Times[Repeated[1], 11]]];
Assert[parse[".1.11"] === Hold[Times[0.1, 0.11]]];
Assert[parse["11.11"] === Hold[11.11]];
Assert[parse["..111"] === $Failed];
Assert[parse["1.111"] === Hold[1.111]];
Assert[parse[".1111"] === Hold[0.1111]];
Assert[parse["11111"] === Hold[11111]];

Assert[parse["1....1"] === Hold[Times[RepeatedNull[1], 0.1]]];

ClearAll[parse, parsetop];
