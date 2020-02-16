Print["part"];

Print["... one level"];

x = {{1, 2}, {3, 4}};
y = x;
x[[1]] = x;
Assert[y == {{1, 2}, {3, 4}}];
Assert[x == {{{1, 2}, {3, 4}}, {3, 4}}];

Print["... two levels"];

x = {{1, 2}, {3, 4}};
y = x;
x[[2, 1]] = y[[1]];
Assert[y == {{1, 2}, {3, 4}}];
Assert[x == {{1, 2}, {{1, 2}, 4}}];

