Print["modular subgroups"];

Print["... generators"];

ClearAll[checkGen];
checkGen[g_] := (
    Assert[ModularSubgroups`ValidGroupQ[g]];
    Assert[g === ModularSubgroups`FromGenerators[ModularSubgroups`Generators[g]]]
);

Do[
    checkGen[ModularSubgroups`Group[s, o]];
, {s, {{0}, {1}}}
, {o, {{0}, {1}}}];

Do[
    checkGen[ModularSubgroups`Group[s, o]];
, {s, {{0, 0}, {1, 0}, {0, 2}, {1, 2}, {2, 1}}}
, {o, {{0, 0}, {1, 0}, {0, 2}}}];

Do[
    checkGen[ModularSubgroups`Group[s, o]];
, {s, {{1, 2, 3}, {0, 2, 3}, {1, 0, 3}, {0, 0, 3}, {1, 2, 0}, {0, 2, 0}, {1, 0, 0}, {0, 0, 0},
       {2, 1, 3}, {2, 1, 3},
       {3, 2, 1}, {3, 0, 1},
       {1, 3, 2}, {0, 3, 2}}}
, {o, {{1, 2, 3}, {0, 2, 3}, {1, 0, 3}, {0, 0, 3}, {1, 2, 0}, {0, 2, 0}, {1, 0, 0}, {0, 0, 0},
       {2, 3, 1}, {3, 1, 2}}}];

Print["... arithmetic"];

ClearAll[Gamma, Gamma1, Gamma0];
Gamma[n_] := Gamma[n] = ModularSubgroups`FromMemberQ[Function[Mod[#, n] === IdentityMatrix[2]]];
Gamma1[n_] := Gamma1[n] = ModularSubgroups`FromMemberQ[Function[Mod[#[[1,2]], n] === Mod[#[[2,1]], n] === 0]];
Gamma0[n_] := Gamma0[n] = ModularSubgroups`FromMemberQ[Function[Mod[#[[1,2]], n] === 0]];

Do[
    Assert[ModularSubgroups`Index[Gamma[n]] === n^3/2*Product[1 - 1/p^2, {p, First/@FactorInteger[n]}]];
, {n, 3, 5}];

Do[
    Assert[ModularSubgroups`Index[Gamma1[n]] === n^2*Product[1 + 1/p, {p, First/@FactorInteger[n]}]];
, {n, 1, 10}];

Do[
    Assert[ModularSubgroups`Index[Gamma0[n]] === n*Product[1 + 1/p, {p, First/@FactorInteger[n]}]];
, {n, 1, 10}];

Do[
    Assert[ModularSubgroups`Join[Gamma0[n], Gamma0[m]] === Gamma0[GCD[n, m]]];
    Assert[ModularSubgroups`Meet[Gamma0[n], Gamma0[m]] === Gamma0[LCM[n, m]]];
, {n, 1, 10}, {m, 1, 10}];

Print["... generator relations"];

ClearAll[RandSL2Z];
RandSL2Z[] :=
    Block[{a, b, c, d, g},
        c = RandomInteger[{0, 20}];
        d = RandomInteger[{If[c === 0, 1, -20], 20}];
        {g, {a, b}} = ExtendedGCD[c, d];
        {{b, -a}, {c/g, d/g}}
    ];

Do[
    ag = Table[RandSL2Z[], {RandomInteger[{0, 5}]}];
    a = ModularSubgroups`FromGenerators[ag];
    bg = Table[RandSL2Z[], {RandomInteger[{0, 5}]}];
    b = ModularSubgroups`FromGenerators[bg];
    ab = ModularSubgroups`FromGenerators[Join[ag, bg]];
    Assert[ab === ModularSubgroups`Join[a, b]];
, {20}];

Print["... membership"];

Do[
    ag = Table[RandSL2Z[], {RandomInteger[{0, 6}]}];
    a = ModularSubgroups`FromGenerators[ag];
    ag = Join[ag, {IdentityMatrix[2]}, Inverse/@ag];
    m = n = RandSL2Z[];
    Do[
        n = n.RandomChoice[ag];
        Assert[ModularSubgroups`MemberQ[a, m] === ModularSubgroups`MemberQ[a, n]];
    , {10}];
, {10}];

Do[
    a = ModularSubgroups`FromGenerators[{RandSL2Z[], RandSL2Z[]}];
    While[ModularSubgroups`Index[a] === Infinity,
        a = ModularSubgroups`Join[a, ModularSubgroups`FromGenerators[{RandSL2Z[]}]];
    ];
    b = ModularSubgroups`FromGenerators[{RandSL2Z[], RandSL2Z[]}];
    While[ModularSubgroups`Index[b] === Infinity,
        b = ModularSubgroups`Join[b, ModularSubgroups`FromGenerators[{RandSL2Z[]}]];
    ];
    ab = ModularSubgroups`FromMemberQ[Function[ModularSubgroups`MemberQ[a, #] === ModularSubgroups`MemberQ[b, #] === True]];
    Assert[ab === ModularSubgroups`Meet[a, b]];
, {20}];

Print["... comparison"];

Do[
    a = ModularSubgroups`FromGenerators[Table[RandSL2Z[], {RandomInteger[{0, 6}]}]];
    b = ModularSubgroups`FromGenerators[Table[RandSL2Z[], {RandomInteger[{0, 6}]}]];
    b = ModularSubgroups`Join[a, b];
    Assert[ModularSubgroups`LessEqual[a, b]];
, {100}];
