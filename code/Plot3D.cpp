#include "globalstate.h"
#include "code.h"
#include "eval.h"
#include "code/local.h"
#include "ex_print.h"

static ex eval_plot3d(er fxn, er varx, er xmin, er xmax, er vary, er ymin, er ymax)
{
    int32_t xdivs = 70, ydivs = 70;
    uex vertices(gs.sym_sList.get(), 10+(xdivs+1)*(ydivs+1));
    uex triangles(gs.sym_sList.get(), 10+2*xdivs*ydivs);
    uex r(gs.sym_sNull.copy()), x(gs.sym_sNull.copy()), y(gs.sym_sNull.copy());
    r.setnz(eval(ecopy(xmin)));
    double Xmin = econvert_todouble(r.get());
    r.setnz(eval(ecopy(xmax)));
    double Xmax = econvert_todouble(r.get());
    sym_localize Lx(varx);
    for (int32_t i = 0; i <= xdivs; i++)
    {
        x.setnz(emake_double(((xdivs-i)*Xmin + i*Xmax)/xdivs));
        esym_set_ovalue(Lx.entry, x.copy());
        r.setnz(eval(ecopy(ymin)));
        double Ymin = econvert_todouble(r.get());
        r.setnz(eval(ecopy(ymax)));
        double Ymax = econvert_todouble(r.get());
        sym_localize Ly(vary);
        for (int32_t j = 0; j <= ydivs; j++)
        {
            y.setnz(emake_double(((ydivs-j)*Ymin + j*Ymax)/ydivs));
            esym_set_ovalue(Ly.entry, y.copy());
            r.setnz(eval(ecopy(fxn)));
            vertices.push_back(emake_node(gs.sym_sList.copy(), x.copy(), y.copy(), r.copy()));
            if (i < xdivs && j < ydivs)
            {
                triangles.push_back(emake_node(gs.sym_sList.copy(),
                                                emake_int_si((xdivs+1)*(j+0) + (i+0) + 1),
                                                emake_int_si((xdivs+1)*(j+0) + (i+1) + 1),
                                                emake_int_si((xdivs+1)*(j+1) + (i+0) + 1)
                                    ));
                triangles.push_back(emake_node(gs.sym_sList.copy(),
                                                emake_int_si((xdivs+1)*(j+1) + (i+1) + 1),
                                                emake_int_si((xdivs+1)*(j+1) + (i+0) + 1),
                                                emake_int_si((xdivs+1)*(j+0) + (i+1) + 1)));
            }
        }
    }
    return  emake_node(gs.sym_sGraphics3D.copy(),
                emake_node(gs.sym_sGraphicsComplex.copy(),
                    vertices.release(),
                    emake_node(gs.sym_sPolygon.copy(), triangles.release())
                )
            );
}
/*
Graphics3D[GraphicsComplex[{{1,0,0},{0,1,0},{0,0,1}}, Polygon[{{1,2,3}}]]]

Plot3D[Sin[x*y],{x,-2,2},{y,-2,2}]
*/
ex dcode_sPlot3D(er e)
{
    if (elength(e) != 3)
    {
        return ecopy(e);
    }

    if (!ehas_head_sym_length(echild(e,2), gs.sym_sList.get(), 3)
        || !eis_sym(echild(e,2,1)))
    {
        return ecopy(e);
    }

    if (!ehas_head_sym_length(echild(e,3), gs.sym_sList.get(), 3)
        || !eis_sym(echild(e,3,1)))
    {
        return ecopy(e);
    }

    return eval_plot3d(echild(e,1), echild(e,2,1), echild(e,2,2), echild(e,2,3),
                                    echild(e,3,1), echild(e,3,2), echild(e,3,3));
}

/*
ContourPlot3D[x^4 + y^4 + z^4 - (x^2 + y^2 + z^2)^2 + 3*(x^2 + y^2 + z^2) - 3, {x, -3, 3}, {y, -3, 3}, {z, -3, 3}]
ContourPlot3D[x^4+y^4+z^4-x^2-y^2-z^2+0.5,{x,-1,1},{y,-1,1},{z,-1,1}]
ContourPlot3D[(x^2+y^2)^3-x^2 y^2 (4z^2+1),{x,-3,3},{y,-3,3},{z,-3,3}]
ContourPlot3D[x^2+y^2+z^2-0.9,{x,-1,1},{y,-1,1},{z,-1,1}]
ContourPlot3D[x^2+y^2+z^2-3.8,{x,-2,2},{y,-2,2},{z,-2,2}]


ContourPlot3D[x^2+y^2+z^2-3.8,{x,-2.5,2.5},{y,-2.5,2.5},{z,-2.5,2.5}]

*/
ex dcode_sContourPlot3D(er e)
{
    if (elength(e) != 4)
    {
        return ecopy(e);
    }

    if (!ehas_head_sym_length(echild(e,2), gs.sym_sList.get(), 3)
        || !eis_sym(echild(e,2,1)))
    {
        return ecopy(e);
    }

    if (!ehas_head_sym_length(echild(e,3), gs.sym_sList.get(), 3)
        || !eis_sym(echild(e,3,1)))
    {
        return ecopy(e);
    }

    if (!ehas_head_sym_length(echild(e,4), gs.sym_sList.get(), 3)
        || !eis_sym(echild(e,4,1)))
    {
        return ecopy(e);
    }

    er x = echild(e,2,1);
    er y = echild(e,3,1);
    er z = echild(e,4,1);

    uex xmin(eval(ecopychild(e,2,2)));
    uex xmax(eval(ecopychild(e,2,3)));

    uex ymin(eval(ecopychild(e,3,2)));
    uex ymax(eval(ecopychild(e,3,3)));

    uex zmin(eval(ecopychild(e,4,2)));
    uex zmax(eval(ecopychild(e,4,3)));

    er f = echild(e,1);
    uex fx(eval_diff(f, x));
    uex fy(eval_diff(f, y));
    uex fz(eval_diff(f, z));
/*
std::cout << "f: " << ex_tostring(f) << std::endl;
std::cout << "fx: " << ex_tostring(fx.get()) << std::endl;
std::cout << "fy: " << ex_tostring(fy.get()) << std::endl;
std::cout << "fz: " << ex_tostring(fz.get()) << std::endl;
*/

    uex l(emake_node(gs.sym_sList.copy(),
            emake_node(gs.sym_sPattern.copy(), ecopy(x), gs.sym_sReal.copy()),
            emake_node(gs.sym_sPattern.copy(), ecopy(y), gs.sym_sReal.copy()),
            emake_node(gs.sym_sPattern.copy(), ecopy(z), gs.sym_sReal.copy())
          ));
/*
std::cout << "l: " << ex_tostring(l.get()) << std::endl;
*/

    uex cf(eval(emake_node(gs.sym_sCompile.copy(), l.copy(), ecopy(f))));
    uex cfx(eval(emake_node(gs.sym_sCompile.copy(), l.copy(), fx.copy())));
    uex cfy(eval(emake_node(gs.sym_sCompile.copy(), l.copy(), fy.copy())));
    uex cfz(eval(emake_node(gs.sym_sCompile.copy(), l.copy(), fz.copy())));
/*
std::cout << "cf: " << ex_tostring(cf.get()) << std::endl;
std::cout << "cfx: " << ex_tostring(cfx.get()) << std::endl;
std::cout << "cfy: " << ex_tostring(cfy.get()) << std::endl;
std::cout << "cfz: " << ex_tostring(cfz.get()) << std::endl;
*/
    uex r(gs.sym_sGraphics3D.copy(), 10);
    r.push_back(cf.copy());
    r.push_back(cfx.copy());
    r.push_back(cfy.copy());
    r.push_back(cfz.copy());
    r.push_back(xmin.release());
    r.push_back(xmax.release());
    r.push_back(ymin.release());
    r.push_back(ymax.release());
    r.push_back(zmin.release());
    r.push_back(zmax.release());

    return r.release();
}
