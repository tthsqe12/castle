#include <stack>
#include "boxes.h"
#include "ex_parse.h"
#include "notebook.h"
#include "box_convert.h"
#include "ex_print.h"
#include "eval.h"

void graphics3dbox::print(size_t indent, int32_t offx, int32_t offy)
{
    for (size_t i = 0; i < indent; i++)
        printf("    ");

    printf("(%d,%d) graphics3d: size(%d,%d:%d)\n",
             offx, offy, sizex, sizey, sizey);
}

visitRet graphics3dbox::visit(visitArg m)
{
    return visitret_OK;
}

boxbase * graphics3dbox::copy()
{
    assert(false);
    return nullptr;
}

void graphics3dbox::key_copy(boxbase*&b)
{
    assert(false);
}

void graphics3dbox::key_paste(boxbase*&b)
{
    assert(false);
}

void graphics3dbox::insert_char(int32_t c)
{
    return;
}

moveRet graphics3dbox::move(boxbase*&b, moveArg m)
{
    moveRet r;
    switch (m)
    {
        case movearg_Left:
        case movearg_ShiftLeft:
        case movearg_Right:
        case movearg_ShiftRight:
        {
            return moveret_End;
        }
        case movearg_Last:
        case movearg_First:
        {
            return moveret_OK;
        }
        default:
        {
            assert(false);
            return moveret_OK;
        }
    }
}

insertRet graphics3dbox::insert(boxbase*&b, insertArg m)
{
    insertRet r;
    switch (m)
    {
        default:
        {
            return insertret_Done;
        }
    }
}

removeRet graphics3dbox::remove(boxbase*&b, removeArg m)
{
    assert(b == nullptr);
    removeRet r;

    switch (m)
    {
        case removearg_Left:
        {
            return removeret_End;
        }
        case removearg_Right:
        {
            return removeret_End;
        }
        default:
        {
            assert(false);
            return removeret_Bad;
        }
    }
}

ex graphics3dbox::get_ex()
{
    return emake_node(gs.sym_sGraphics3D.copy());
}


void graphics3dbox::get_cursor(aftransform * T)
{
    assert(false);
    return;
}

void graphics3dbox::measure(boxmeasurearg ma)
{
    fs = fontint_to_fontsize(ma.fi);
    sizex = image.sizex*UNITS_PER_PIXEL + 2*5;
    sizey = image.sizey*UNITS_PER_PIXEL + 2*5;
    centery = sizey/2;
}

void graphics3dbox::draw_pre(boxdrawarg da)
{
    int32_t usx = da.globx + 0;
    int32_t usy = da.globy + 0;
        double Ax = usx;
        double Ay = usy;
        double Bx = usx + sizex;
        double By = usy;
        double Cx = usx;
        double Cy = usy + sizey;
        double Dx = usx + sizex;
        double Dy = usy + sizey;
        drawtline(Ax, Ay, Bx, By, 2.0 + fontsize_size(fs)*0.02, da.nb->cCursor, da.T);
        drawtline(Bx, By, Dx, Dy, 2.0 + fontsize_size(fs)*0.02, da.nb->cCursor, da.T);
        drawtline(Dx, Dy, Cx, Cy, 2.0 + fontsize_size(fs)*0.02, da.nb->cCursor, da.T);
        drawtline(Cx, Cy, Ax, Ay, 2.0 + fontsize_size(fs)*0.02, da.nb->cCursor, da.T);
    return;
}

const double cc = 0.8;
const double aa = 0.3;
const double tt = 1.618033988749895;

void graphics3dbox::evalgrad(point3d_t g, double t, point3d_t pos, point3d_t dir)
{
grad_evals++;

    double s, x, y, z;
    x = center_x + pos->x + t*dir->x;
    y = center_y + pos->y + t*dir->y;
    z = center_z + pos->z + t*dir->z;

//    g->x = -1.0*(1 + 2*tt)*x*(-1 + x*x + y*y + z*z) - 2*x*(tt*tt*x*x - y*y)*(tt*tt*y*y - z*z) + 2*tt*tt*x*(tt*tt*y*y - z*z)*(-x*x + tt*tt*z*z);
//    g->y = -1.0*(1 + 2*tt)*y*(-1 + x*x + y*y + z*z) + 2*tt*tt*y*(tt*tt*x*x - y*y)*(-x*x + tt*tt*z*z) - 2*y*(tt*tt*y*y - z*z)*(-x*x + tt*tt*z*z);
//    g->z = -1.0*(1 + 2*tt)*z*(-1 + x*x + y*y + z*z) + 2*tt*tt*(tt*tt*x*x - y*y)*z*(tt*tt*y*y - z*z) - 2*(tt*tt*x*x - y*y)*z*(-x*x + tt*tt*z*z);

//    g->x = 6*x*(x*x+y*y)*(x*x+y*y) - 2*x*y*y*(4*z*z+1);
//    g->y = 6*y*(x*x+y*y)*(x*x+y*y) - 2*x*x*y*(4*z*z+1);
//    g->z = -8*x*x*y*y*z;

    g->x = 4*x*x*x-2*x;
    g->y = 4*y*y*y-2*y;
    g->z = 4*z*z*z-2*z;

//    uex r(eval(emake_node(fxn_x.copy(), emake_double(x), emake_double(y), emake_double(z))));
//    g->x = econvert_todouble(r.get());
//    r.setnz(eval(emake_node(fxn_y.copy(), emake_double(x), emake_double(y), emake_double(z))));
//    g->y = econvert_todouble(r.get());
//    r.setnz(eval(emake_node(fxn_z.copy(), emake_double(x), emake_double(y), emake_double(z))));
//    g->z = econvert_todouble(r.get());

//    g->x = 2*x;
//    g->y = 2*y;
//    g->z = 2*z;

//    s = sqrt(0.00001 + x*x+y*y);
//    g->x = 2*x - 2*cc*x/s;
//    g->y = 2*y - 2*cc*y/s;
//    g->z = 2*z;

//    g->x = 2*x;
//    g->y = 6*y*(1-y*y)*(1-y*y);
//    g->z = 2*z;

//    g->x = cos(2*x)*cos(2*z) - sin(2*x)*sin(2*y);
//    g->y = cos(2*x)*cos(2*y) - sin(2*y)*sin(2*z);
//    g->z = cos(2*y)*cos(2*z) - sin(2*x)*sin(2*z);

//    g->x = 6*x + 4*x*x*x - 4*x*(x*x + y*y + z*z);
//    g->y = 6*y + 4*y*y*y - 4*y*(x*x + y*y + z*z);
//    g->z = 6*z + 4*z*z*z - 4*z*(x*x + y*y + z*z);

//    double a = 1.618033988749895;
//    g->x = -sin(x - a*y) - sin(x + a*y) + a*sin(-a*x + z) - a*sin(a*x + z);
//    g->y = a*sin(x - a*y) - a*sin(x + a*y) - sin(y - a*z) - sin(y + a*z);
//    g->z = -sin(-a*x + z) - sin(a*x + z) + a*sin(y - a*z) - a*sin(y + a*z);
}

double graphics3dbox::evalfxn(double t, point3d_t pos, point3d_t dir)
{
fxn_evals++;

    double x, y, z;
    x = center_x + pos->x + t*dir->x;
    y = center_y + pos->y + t*dir->y;
    z = center_z + pos->z + t*dir->z;

//    return (tt*tt*x*x - y*y)*(tt*tt*y*y - z*z)*(tt*tt*z*z - x*x) - 0.25*(1+2*tt)*(x*x + y*y + z*z - 1)*(x*x + y*y + z*z - 1);

//    return (x*x+y*y)*(x*x+y*y)*(x*x+y*y) - (x*x)*(y*y)*(4*z*z+1);

    return (x*x)*(x*x) + (y*y)*(y*y) + (z*z)*(z*z) - x*x - y*y - z*z + 0.5;

//    uex r(eval(emake_node(fxn.copy(), emake_double(x), emake_double(y), emake_double(z))));
//    return econvert_todouble(r.get());

//    return x*x + y*y + z*z - 23;

//    double s = sqrt(0.00001 + x*x+y*y);
//    return (cc-s)*(cc-s) + z*z - aa*aa;

//    return x*x + z*z - (1-y*y)*(1-y*y)*(1-y*y);

//    return 0.5*(cos(2*x)*sin(2*y) + cos(2*y)*sin(2*z) + cos(2*z)*sin(2*x));

//    return x*x*x*x + y*y*y*y + z*z*z*z - (x*x + y*y + z*z)*(x*x + y*y + z*z) + 3*(x*x + y*y + z*z) - 3;

//    double a = 1.618033988749895;
//    return cos(x + a*y) + cos(x - a*y) + cos(y + a*z) + cos(y - a*z) + cos(z + a*x) + cos(z - a*x) - 2;
}


double graphics3dbox::find_sign_change2(double a, double a_eval, point3d_t a_grad, double b, double b_eval, point3d_t pos, point3d_t dir)
{
    assert(a_eval*b_eval <= 0);

    if (b - a < 0.000000001)
        return (a + b)/2;

    double m = a - a_eval/(a_grad->x*dir->x + a_grad->y*dir->y + a_grad->z*dir->z);

    if (m <= a || m >= b)
        m = (a + b)/2;

    m = std::min(m, (1*a + 7*b)/8);
    m = std::max(m, (7*a + 1*b)/8);

    double m_eval = evalfxn(m, pos, dir);

    if (a_eval*m_eval <= 0)
    {
        if (m - a < 0.000000001)
            return (a + m)/2;

        return find_sign_change2(a, a_eval, a_grad, m, m_eval, pos, dir);
    }
    else
    {
        if (b - m < 0.000000001)
            return (m + b)/2;

        point3d_t grad;
        evalgrad(grad, m, pos, dir);

        return find_sign_change2(m, m_eval, grad, b, b_eval, pos, dir);
    }
}


double graphics3dbox::find_sign_change(double min, double max, point3d_t pos, point3d_t dir)
{
    double min_step = 0.00000001;
    double max_step = std::max(min_step, (max - min)/15);
    double a = min;
    double a_eval = evalfxn(a, pos, dir);

//std::cout << "find_sign_change  min = " << min << "  max = " << max << std::endl;

    while (a < max)
    {
        point3d_t grad;
        evalgrad(grad, a, pos, dir);

        double e = 1.5*std::abs(a_eval)/sqrt(0.000001 + (grad->x*grad->x + grad->y*grad->y + grad->z*grad->z) * (dir->x*dir->x + dir->y*dir->y + dir->z*dir->z));
        e = std::min(e, max_step);
        e = std::max(e, min_step);

//std::cout << "a: " << a << std::endl;

        double b = a + e;
        double b_eval = evalfxn(b, pos, dir);

        if (a_eval*b_eval <= 0)
        {
            double t = find_sign_change2(a, a_eval, grad, b, b_eval, pos, dir);
            if (a <= t && t <= b)
                return t;
        }

        double m = (a + b)/2;
        double m_eval = evalfxn(m, pos, dir);
        if (a_eval*m_eval <= 0)
        {
            double t = find_sign_change2(a, a_eval, grad, m, m_eval, pos, dir);
            if (a <= t && t <= b)
                return t;
        }
        if (m_eval*b_eval <= 0)
        {
            evalgrad(grad, m, pos, dir);
            double t = find_sign_change2(m, m_eval, grad, b, b_eval, pos, dir);
            if (a <= t && t <= b)
                return t;
        }

        a = b;
        a_eval = b_eval;
    }

    return max + 1;
}

template <class T> class vec3 {
public:
    T x, y, z;

    vec3() : x(0), y(0), z(0) {};
    vec3(T X, T Y, T Z) : x(X), y(Y), z(Z) {};

    void set(double x_, double y_, double z_)
    {
        x = x_;
        y = y_;
        z = z_;
    }

    void sub(const vec3& a, const vec3& b)
    {
        x = a.x - b.x;
        y = a.y - b.y;
        z = a.z - b.z;
    }

    void madd(T a, const vec3<T>& b)
    {
        x += a*b.x;
        y += a*b.y;
        z += a*b.z;
    }

    void smul(T a)
    {
        x *= a;
        y *= a;
        z *= a;
    }

    void normalize()
    {
        T s = sqrt(x*x + y*y + z*z + DBL_MIN);
        x /= s;
        y /= s;
        z /= s;
    }
};

template <class T> T vec3_dot(const vec3<T>& a, const vec3<T>& b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

double sqr(double x) {return sqrt(std::max(0.0001, 1 - x*x));}


graphics3dbox::graphics3dbox()
      : boxbase(BNTYPE_PLOT3D),
        image(200, 300),
        fxn(emake_cint(0)),
        fxn_x(emake_node(gs.sym_sFunction.copy(), emake_node(gs.sym_sTimes.copy(), emake_node(gs.sym_sSlot.copy(), emake_cint(1)), emake_cint(2)))),
        fxn_y(emake_node(gs.sym_sFunction.copy(), emake_node(gs.sym_sTimes.copy(), emake_node(gs.sym_sSlot.copy(), emake_cint(2)), emake_cint(2)))),
        fxn_z(emake_node(gs.sym_sFunction.copy(), emake_node(gs.sym_sTimes.copy(), emake_node(gs.sym_sSlot.copy(), emake_cint(3)), emake_cint(2))))
    {
        qangle->w = 0.8;
        qangle->x = -0.3;
        qangle->y = 0.0;
        qangle->z = 0.1;
        fov = 1.8;

        box_x = 1;
        box_y = 1;
        box_z = 1;

        center_x = 0;
        center_y = 0;
        center_z = 0;

        std::string inputline("Compile[{x:Real, y:Real, z:Real}, x^2+y^2+z^2-0.9]");
        std::vector<uex> v;
        syntax_report sr;
        ex_parse_string(v, inputline.c_str(), inputline.size(), true, sr);
        assert(!sr.have_error());
        fxn.setnz(eval(v[0].copy()));

        render();
    }


void graphics3dbox::render()
{
std::cout << "rendering " << std::endl;
std::cout << " f: " << ex_tostring(fxn.get()) << std::endl;
std::cout << "fx: " << ex_tostring(fxn_x.get()) << std::endl;
std::cout << "fy: " << ex_tostring(fxn_y.get()) << std::endl;
std::cout << "fz: " << ex_tostring(fxn_z.get()) << std::endl;

        qangle->w = 0.85;
        qangle->x = -0.6;
        qangle->y = 0.3/0.85;
        qangle->z = +0.5;

grad_evals = fxn_evals = 0;

    double s;
    point3d_t p1, p2, p3, s1, s2, s3, position, center;
    orthomatrix_t angle;

    QuatNormalize(qangle, qangle);
    QuatToOrthMat(angle, qangle);
    s = sqrt(box_x*box_x + box_y*box_y + box_z*box_z);
    position->x = 3.0*s*angle->b1;
    position->y = 3.0*s*angle->b2;
    position->z = 3.0*s*angle->b3;

    image.fill(0.0,0.0,0.0);

std::cout << "s: " << s << std::endl;

    std::vector<vec3<double>> lights, light_diff, light_spec;


    lights.push_back(vec3<double>(3*s*angle->b1 + 2*s*angle->u1 + 3*s*angle->r1,
                                  3*s*angle->b2 + 2*s*angle->u2 + 3*s*angle->r2,
                                  3*s*angle->b3 + 2*s*angle->u3 + 3*s*angle->r3));
    light_diff.push_back(vec3<double>(0.5, 0.2, 0.1));
    light_spec.push_back(vec3<double>(0.9, 0.4, 0.2));


    lights.push_back(vec3<double>(3*s*angle->b1 + 2*s*angle->u1 - 3*s*angle->r1,
                                  3*s*angle->b2 + 2*s*angle->u2 - 3*s*angle->r2,
                                  3*s*angle->b3 + 2*s*angle->u3 - 3*s*angle->r3));
    light_diff.push_back(vec3<double>(0.1, 0.2, 0.5));
    light_spec.push_back(vec3<double>(0.2, 0.4, 0.9));



/*
    lights.push_back(vec3<double>(0.020*s*angle->b1 + -0.19*s*angle->u1 - 0.2*s*angle->r1,
                                  0.020*s*angle->b2 + -0.19*s*angle->u2 - 0.2*s*angle->r2,
                                  0.020*s*angle->b3 + -0.19*s*angle->u3 - 0.2*s*angle->r3));
    light_diff.push_back(vec3<double>(0.2, 0.2, 0.1));
    light_spec.push_back(vec3<double>(0.7, 0.7, 0.5));
*/

/*
    lights.push_back(vec3<double>(3*s*angle->b1 - 6*s*angle->u1 + 1*s*angle->r1,
                                  3*s*angle->b2 - 6*s*angle->u2 + 1*s*angle->r2,
                                  3*s*angle->b3 - 6*s*angle->u3 + 1*s*angle->r3));
    light_diff.push_back(vec3<double>(0.1, 0.1, 0.4));
    light_spec.push_back(vec3<double>(0.7, 0.6, 0.7));
*/

    vec3<double> camera_pos(position->x, position->y, position->z);

    double xs = 4.0/box_x;
    double ys = 4.0/box_y;
    double zs = 4.0/box_z;

    for (int32_t j = 0; j < image.sizey; j++)
    {
if ((j%1) == 0) std::cout << "row " << j << std::endl;


double maxss = 0, minss = 10000000000;

    for (int32_t i = 0; i < image.sizex; i++)
    {

/*
if (!(j == 100 && (260 == i)))
    continue;
*/

//if (j > 100 || i > 400)
//    continue;

//std::cout << "row " << j << " col " << i << std::endl;



        point3d_t dir;
        dir->x = (2*i - image.sizex)*1.5/(fov*image.sizey)*angle->r1 + (image.sizey - 2*j)*1.5/(fov*image.sizey)*angle->u1 - 3 * angle->b1;
        dir->y = (2*i - image.sizex)*1.5/(fov*image.sizey)*angle->r2 + (image.sizey - 2*j)*1.5/(fov*image.sizey)*angle->u2 - 3 * angle->b2;
        dir->z = (2*i - image.sizex)*1.5/(fov*image.sizey)*angle->r3 + (image.sizey - 2*j)*1.5/(fov*image.sizey)*angle->u3 - 3 * angle->b3;

        double t_depth;

        {
            double tmin = (-box_x - position->x) / dir->x;
            double tmax = (+box_x - position->x) / dir->x;
            if (tmin > tmax) std::swap(tmin, tmax);

            double tymin = (-box_y - position->y) / dir->y;
            double tymax = (+box_y - position->y) / dir->y;
            if (tymin > tymax) std::swap(tymin, tymax);

            double tzmin = (-box_z - position->z) / dir->z;
            double tzmax = (+box_z - position->z) / dir->z;
            if (tzmin > tzmax) std::swap(tzmin, tzmax);

            if ((tmin > tymax) || (tymin > tmax))
                continue;

            if (tymin > tmin)
                tmin = tymin;
            if (tymax < tmax)
                tmax = tymax;

            if ((tmin > tzmax) || (tzmin > tmax))
                continue;

            if (tzmin > tmin)
                tmin = tzmin;
            if (tzmax < tmax)
                tmax = tzmax;

            if (tmax <= 0)
                continue;

            tmin = std::max(tmin, 0.0);

            double t = find_sign_change(tmin, tmax, position, dir);
            if (t <= tmin || t >= tmax)
                continue;

            t_depth = t;
        }

        vec3<double> P(position->x + t_depth*dir->x,
                       position->y + t_depth*dir->y,
                       position->z + t_depth*dir->z);

        point3d_t n;
        evalgrad(n, t_depth, position, dir);

        vec3<double> N(n->x, n->y, n->z);
        N.normalize();
        bool flipped = false;
        if (N.x*dir->x + N.y*dir->y + N.z*dir->z > 0)
        {
            flipped = true;
            N.x = -N.x;
            N.y = -N.y;
            N.z = -N.z;
        }

//std::cout << "N: " << N.x << ", " << N.y << ", " << N.z << std::endl;

        // ambient occulusion
        N.normalize();
        double ao = 1;
        if (false)
        {
            vec3<double> A, B;
            double nxy = N.x*N.x + N.y*N.y;
            double nxz = N.x*N.x + N.z*N.z;
            double nyz = N.y*N.y + N.z*N.z;
            if (nxy >= nxz && nxy >= nyz)
            {
                A.set(N.y, -N.x, 0);
                B.set(N.x*N.z, N.y*N.z, -nxy);
            }
            else if (nxz >= nyz)
            {
                A.set(-N.z, 0, N.x);
                B.set(N.x*N.y, -nxz, N.y*N.z);
            }
            else
            {
                A.set(0, N.z, -N.y);
                B.set(-nyz, N.x*N.y, N.x*N.z);
            }
            A.normalize();
            B.normalize();

/*
assert(std::abs(N.x*N.x + N.y*N.y + N.z*N.z - 1) < 0.001);
assert(std::abs(A.x*A.x + A.y*A.y + A.z*A.z - 1) < 0.001);
assert(std::abs(B.x*B.x + B.y*B.y + B.z*B.z - 1) < 0.001);
    

assert(std::abs(N.x*A.x + N.y*A.y + N.z*A.z - 0) < 0.001);
assert(std::abs(N.x*B.x + N.y*B.y + N.z*B.z - 0) < 0.001);
assert(std::abs(B.x*A.x + B.y*A.y + B.z*A.z - 0) < 0.001);
*/


//std::cout << "N: " << N.x << ", " << N.y << ", " << N.z << std::endl;
//std::cout << "A: " << A.x << ", " << A.y << ", " << A.z << std::endl;
//std::cout << "B: " << B.x << ", " << B.y << ", " << B.z << std::endl;

            double ao_num = 0;
            double ao_den = 0;
            double ao_dist_min = s*0.001;
            double ao_dist_max = s*1.5;
            ao = ao_dist_min;
            for (double phi = 0.05; phi <= 1.5; phi += 0.05)
            {
                for (double theta = 0; theta < 2; theta += 1.0/(4+2*phi))
                {
                    ao_den += ao_dist_max*ao_dist_max;

//std::cout << "phi: " << phi << "  theta: " << theta << std::endl;

                    vec3<double> D(0, 0, 0);
                    D.madd(cos(phi), N);
                    D.madd(sin(phi)*cos(3.1415926535*theta + phi), A);
                    D.madd(sin(phi)*sin(3.1415926535*theta + phi), B);

//std::cout << "D: " << D.x << ", " << D.y << ", " << D.z << std::endl;

                    double tmin = (-box_x - P.x) / D.x;
                    double tmax = (+box_x - P.x) / D.x;
                    if (tmin > tmax) std::swap(tmin, tmax);

                    double tymin = (-box_y - P.y) / D.y;
                    double tymax = (+box_y - P.y) / D.y;
                    if (tymin > tymax) std::swap(tymin, tymax);

                    double tzmin = (-box_z - P.z) / D.z;
                    double tzmax = (+box_z - P.z) / D.z;
                    if (tzmin > tzmax) std::swap(tzmin, tzmax);

                    if ((tmin > tymax) || (tymin > tmax))
                    {
                        assert(0);
                        continue;
                    }

                    if (tymin > tmin)
                        tmin = tymin;
                    if (tymax < tmax)
                        tmax = tymax;

                    if ((tmin > tzmax) || (tzmin > tmax))
                    {
                        assert(0);
                        continue;
                    }

                    if (tzmin > tmin)
                        tmin = tzmin;
                    if (tzmax < tmax)
                        tmax = tzmax;

                    if (tmax <= 0)
                    {
                        assert(0);
                        continue;
                    }

                    tmax = std::min(tmax, ao_dist_max);
                    tmin = std::max(tmin, ao_dist_min);

                    point3d_t pp, dd;
                    pp->x = P.x;
                    pp->y = P.y;
                    pp->z = P.z;
                    dd->x = D.x;
                    dd->y = D.y;
                    dd->z = D.z;

                    double t = find_sign_change(tmin, tmax, pp, dd);

                    if (t <= tmin)
                        continue;

                    if (t >= tmax)
                    {
                        ao_num += ao_dist_max*ao_dist_max;
                    }
                    else
                    {
                        ao_num += t*t;
                    }
                }
            }

            ao = ao_num/ao_den;
            ao = std::min(ao, 1.0);
        }

        double ss = t_depth/sqrt(dir->x*dir->x + dir->y*dir->y + dir->z*dir->z);

if (maxss < ss)
{
    maxss = ss;
}

if (minss > ss)
{
    minss = ss;
}


        ss = 1.5/ss;
        ss = ss*ss*ss;
        ao = std::min(ao*ss, 1.0);
        

        // grid lines
        double mulx = vec3_dot(N, vec3<double>(1,0,0));
        double muly = vec3_dot(N, vec3<double>(0,1,0));
        double mulz = vec3_dot(N, vec3<double>(0,0,1));
        mulx *= mulx;
        muly *= muly;
        mulz *= mulz;
        mulx = mulx > 0.97 ? 1.0 : 2*64/xs*std::abs((round(xs*P.x) - xs*P.x)/(sqrt(1 - mulx)));
        muly = muly > 0.97 ? 1.0 : 2*64/ys*std::abs((round(ys*P.y) - ys*P.y)/(sqrt(1 - muly)));
        mulz = mulz > 0.97 ? 1.0 : 2*64/zs*std::abs((round(zs*P.z) - zs*P.z)/(sqrt(1 - mulz)));
        double mul = std::min(std::min(mulx, muly), mulz);
        mul = std::min(mul, 1.0);
        mul = std::max(mul, 0.0);

        vec3<double> ambdir0(+1.0,0.0,-0.707107);
        vec3<double> ambdir1(-1.0,0.0,-0.707107);
        vec3<double> ambdir2(0.0,+1.0,+0.707107);
        vec3<double> ambdir3(0.0,-1.0,+0.707107);


        vec3<double> fcolor(0,0,0);

        if (flipped)
        {
        vec3<double> ambcol0(0.95,0.20,0.00);
        vec3<double> ambcol1(0.90,0.35,0.00);
        vec3<double> ambcol2(0.95,0.25,0.00);
        vec3<double> ambcol3(0.90,0.20,0.00);
/*
        vec3<double> ambcol0(0.95,0.95,0.95);
        vec3<double> ambcol1(0.95,0.95,0.95);
        vec3<double> ambcol2(0.95,0.95,0.95);
        vec3<double> ambcol3(0.95,0.95,0.95);
*/
        double w0 = std::max(0.001, vec3_dot(ambdir0,N));
        double w1 = std::max(0.001, vec3_dot(ambdir1,N));
        double w2 = std::max(0.001, vec3_dot(ambdir2,N));
        double w3 = std::max(0.001, vec3_dot(ambdir3,N));
        fcolor.madd(ao*w0, ambcol0);
        fcolor.madd(ao*w1, ambcol1);
        fcolor.madd(ao*w2, ambcol2);
        fcolor.madd(ao*w3, ambcol3);
        fcolor.smul(1/(w0+w1+w2+w3));
        }
        else
        {

        vec3<double> ambcol0(0.75,0.75,0.80);
        vec3<double> ambcol1(0.70,0.80,0.85);
        vec3<double> ambcol2(0.75,0.75,0.85);
        vec3<double> ambcol3(0.70,0.80,0.80);
/*
        vec3<double> ambcol0(0.95,0.95,0.95);
        vec3<double> ambcol1(0.95,0.95,0.95);
        vec3<double> ambcol2(0.95,0.95,0.95);
        vec3<double> ambcol3(0.95,0.95,0.95);
*/
        double w0 = std::max(0.001, vec3_dot(ambdir0,N));
        double w1 = std::max(0.001, vec3_dot(ambdir1,N));
        double w2 = std::max(0.001, vec3_dot(ambdir2,N));
        double w3 = std::max(0.001, vec3_dot(ambdir3,N));
        fcolor.madd(ao*w0, ambcol0);
        fcolor.madd(ao*w1, ambcol1);
        fcolor.madd(ao*w2, ambcol2);
        fcolor.madd(ao*w3, ambcol3);
        fcolor.smul(1/(w0+w1+w2+w3));
        }

        vec3<double> V;
        V.sub(camera_pos, P); V.normalize();
        for (size_t k = 0; k < lights.size(); k++)
        {
            vec3<double> L;
            L.sub(lights[k], P); L.normalize();
/*
            point3d_t pp, dd;
            pp->x = P.x;
            pp->y = P.y;
            pp->z = P.z;
            dd->x = lights[k].x - P.x;
            dd->y = lights[k].y - P.y;
            dd->z = lights[k].z - P.z;

                    double tmin = (-box_x - pp->x) / dd->x;
                    double tmax = (+box_x - pp->x) / dd->x;
                    if (tmin > tmax) std::swap(tmin, tmax);

                    double tymin = (-box_y - pp->y) / dd->y;
                    double tymax = (+box_y - pp->y) / dd->y;
                    if (tymin > tymax) std::swap(tymin, tymax);

                    double tzmin = (-box_z - pp->z) / dd->z;
                    double tzmax = (+box_z - pp->z) / dd->z;
                    if (tzmin > tzmax) std::swap(tzmin, tzmax);

                    if ((tmin > tymax) || (tymin > tmax))
                    {
                        assert(0);
                        continue;
                    }

                    if (tymin > tmin)
                        tmin = tymin;
                    if (tymax < tmax)
                        tmax = tymax;

                    if ((tmin > tzmax) || (tzmin > tmax))
                    {
                        assert(0);
                        continue;
                    }

                    if (tzmin > tmin)
                        tmin = tzmin;
                    if (tzmax < tmax)
                        tmax = tzmax;

            tmin = std::max(0.0001, tmin);
            tmax = std::min(1.0, tmax);

            double t = find_sign_change(tmin, tmax, pp, dd);
            if (t < tmax)
                continue;
*/
            vec3<double> R(-L.x, -L.y, -L.z);
            R.madd(2.0*vec3_dot(L,N), N); R.normalize();
            fcolor.madd(ao*std::max(0.0, vec3_dot(L,N)), light_diff[k]);
            fcolor.madd(ao*pow(std::max(0.0, vec3_dot(R,V)),24), light_spec[k]);
        }

        image.set_pixel(fcolor.x*mul, fcolor.y*mul, fcolor.z*mul, t_depth, i, j);
    }
/*
std::cout << "maxss: " << maxss;
std::cout << "    minss: " << minss << std::endl;
*/

    }

std::cout << "    pixels: " << image.sizey * image.sizex << std::endl;
std::cout << " fxn_evals: " << fxn_evals  << " per pixel " << double(fxn_evals)/(image.sizey * image.sizex) << std::endl;
std::cout << "grad_evals: " << grad_evals << " per pixel " << double(grad_evals)/(image.sizey * image.sizex) << std::endl;



/*
    image.fill(0.95,0.95,0.95);

    point3d * trans_vertices = (point3d *) malloc(verts.size()*sizeof(point3d));
    for (size_t i = 0; i < verts.size(); i++)
    {
        image.transform(trans_vertices + i, verts.data() + i, position, angle, fov);
    }

    for (size_t i = 0; i < tris.size(); i += 3)
    {
        point3d * a = verts.data() + tris[i+0];
        point3d * b = verts.data() + tris[i+1];
        point3d * c = verts.data() + tris[i+2];
        point3d * A = trans_vertices + tris[i+0];
        point3d * B = trans_vertices + tris[i+1];
        point3d * C = trans_vertices + tris[i+2];

        double x1 = b->x - a->x;
        double y1 = b->y - a->y;
        double z1 = b->z - a->z;
        double x2 = c->x - a->x;
        double y2 = c->y - a->y;
        double z2 = c->z - a->z;
        double nx = y1*z2 - y2*z1;
        double ny = x2*z1 - x1*z2;
        double nz = x1*y2 - x2*y1;
        s = sqrt(nx*nx+ny*ny+nz*nz);
        nx = -nx/s;
        ny = -ny/s;
        nz = -nz/s;
        double red   = std::abs(+0.4*nx + 0.3*ny + 0.5*nz);
        double green = std::abs(+0.2*nx - 0.4*ny + 0.4*nz);
        double blue  = std::abs(-0.2*nx + 0.2*ny + 0.7*nz);

        image.draw_triangle(A, B, C, blue,green,red);

        image.draw_line(A, B, 0.2,0.2,0.2);
        image.draw_line(A, C, 0.2,0.2,0.2);
    }
    free(trans_vertices);
*/
    point3d_t corners[8];
    point3d_t trans_corners[8];
    for (int i = 0; i < 8; i++)
    {
        corners[i]->x = (i & 1) ? box_x : -box_x;
        corners[i]->y = (i & 2) ? box_y : -box_y;
        corners[i]->z = (i & 4) ? box_z : -box_z;
        image.transform(trans_corners[i], corners[i], position, angle, fov);
    }

    for (int i = 1; i < 8; i++) {
    for (int j = 0; j < i; j++) {
        int t = i ^ j;
        if ((t&(t-1)) != 0)
            continue;

        image.draw_line(trans_corners[i], trans_corners[j], 0.4,0.4,0.4);
    }}

}



void graphics3dbox::draw_main(boxdrawarg da)
{
    uint8_t * pixels = glb_image.pixels;
    int32_t rowstride = glb_image.rowstride;
    int32_t width = glb_image.pixel_width;
    int32_t height = glb_image.pixel_height;

    for (int32_t y = 0; y < image.sizey; y++)
    for (int32_t x = 0; x < image.sizex; x++)
    {
        pixel3d * q = image.parray + image.sizex * y + x;
        uint8_t r = std::min(std::max(255*q->red,   double(0.0f)),double(255.0f));
        uint8_t g = std::min(std::max(255*q->green, double(0.0f)),double(255.0f));
        uint8_t b = std::min(std::max(255*q->blue,  double(0.0f)),double(255.0f));

        int32_t i = (5 + da.globx + x)/glb_image.units_per_pixel;
        int32_t j = (5 + da.globy + y)/glb_image.units_per_pixel;

        i = (5 + da.globx)/glb_image.units_per_pixel + x;
        j = (5 + da.globy)/glb_image.units_per_pixel + y;

        if (0 <= i && i < width && 0 <= j && j < height)
        {
            uint8_t * p;
            p = pixels + rowstride*(j) + 4*(i);
            p[0] = r;
            p[1] = g;
            p[2] = b;
            p[3] = 255;
        }
    }
}

void graphics3dbox::draw_post(boxdrawarg da)
{
    return;
}
