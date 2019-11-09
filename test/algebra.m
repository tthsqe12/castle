Print["algebra"];

ClearAll[x, y, z, t, u, v, w];

Print["... factor"];

Assert[Factor[x^3 - y^3] === (x - y)*(x^2 + x*y + y^2)];

Assert[Factor[6 + 5*x + 7*x^2 + 5*x^3 + x^4 + 8*y + 6*x*y + 3*x^2*y + x^3*y + 5*y^2 + 2*x*y^2 + 
 3*x^2*y^2 + x^3*y^2 + 4*y^3 + x*y^3 + x^2*y^3 + y^4 + 9*z + 6*x*z + 4*x^2*z + 
 x^3*z + 6*y*z + 2*x*y*z + x^2*y*z + 4*y^2*z + x*y^2*z + y^3*z + 5*z^2 + 2*x*z^2 + 
 2*x^2*z^2 + x^3*z^2 + 3*y*z^2 + x*y*z^2 + y^2*z^2 + x^2*y^2*z^2 + y^3*z^2 + 3*z^3 + 
 x*z^3 + x^2*z^3 + y*z^3 + y^2*z^3 + z^4] === (1 + x^2 + y + z)*(2 + x + y^2 + z)*(3 + x + y + z^2)];

Print["... together"];

Assert[Together[1/(x - 1) + 1/(Sqrt[x] - 1)] === (2 + Sqrt[x])/((-1 + Sqrt[x])*(1 + Sqrt[x]))];



