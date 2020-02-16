#pragma once

#include <cassert>
#include <string>
#include "flint/flint.h"
#include "flint/fmpz.h"
#include "flint/fmpq.h"
#include "flint/fmpz_poly.h"
#include "flint/fmpq_poly.h"
#include "flint/fmpq_mpoly.h"

#ifndef NOFACTOR
#include "flint/fmpq_mpoly_factor.h"
#include "flint/nmod_mpoly_factor.h"
#endif

#include "arb.h"
#include "acb.h"


class xfstr {
public:
    char * data;

    xfstr(char * p)
    {
        data = p;
    }

    ~xfstr()
    {
        flint_free(data);
    }
};


class xfmpz_t {
public:
    fmpz_t data;

    xfmpz_t()
    {
        fmpz_init(data);
    }

    xfmpz_t(ulong x)
    {
        fmpz_init_set_ui(data, x);
    }

    xfmpz_t(slong x)
    {
        fmpz_init(data);
        fmpz_set_si(data, x);
    }

	xfmpz_t(const fmpz_t otherdata)
	{
		fmpz_init_set(data, otherdata);
	}

    xfmpz_t(xfmpz_t&& other) noexcept
    {
        fmpz_init(data);
        fmpz_swap(data, other.data);
    }

    xfmpz_t& operator=(xfmpz_t&& other) noexcept
    {
        if (this != &other) {
            fmpz_swap(data, other.data);
            _fmpz_demote(other.data);
        }  
        return *this;  
    }

	xfmpz_t operator=(const xfmpz_t& other) noexcept
    {
		fmpz_set(data, other.data);
		return *this;  
    }

    xfmpz_t(const xfmpz_t& other)
    {
        fmpz_init_set(data, other.data);
    }

    ~xfmpz_t()
    {
        fmpz_clear(data);
    }

    std::string tostring()
    {
        xfstr s(fmpz_get_str(NULL, 10, data));
        return std::string(s.data);
    }

    std::string tostring(int base)
    {
        assert(2 <= base && base <= 36);
        xfstr s(fmpz_get_str(NULL, base, data));
        return std::string(s.data);
    }
};

class xfmpz_factor_t {
public:
    fmpz_factor_t data;

    xfmpz_factor_t()
    {
        fmpz_factor_init(data);
    }

    ~xfmpz_factor_t()
    {
        fmpz_factor_clear(data);
    }
};


class xfmpq_t {
public:
    fmpq_t data;

    xfmpq_t()
    {
        fmpq_init(data);
    }

    xfmpq_t(const fmpq_t x)
    {
        fmpq_init(data);
        fmpq_set(data, x);
    }

    xfmpq_t(slong a, slong b)
    {
        fmpq_init(data);
        fmpq_set_si(data, a, b);
    }

    xfmpq_t(const xfmpq_t& other)
    {
        fmpq_init(data);
        fmpq_set(data, other.data);
    }

    xfmpq_t(xfmpq_t&& other) noexcept
    {
        fmpq_init(data);
        fmpq_swap(data, other.data);
    }

    xfmpq_t& operator=(xfmpq_t&& other) noexcept
    {
        if (this != &other) {
            fmpq_init(data);
            fmpq_swap(data, other.data);
        }  
        return *this;  
    }  

    double getdouble()
    {
        return fmpz_get_d(fmpq_numref(data))/fmpz_get_d(fmpq_denref(data));
    }

    ~xfmpq_t()
    {
        fmpq_clear(data);
    }

    bool is_square()
    {
        return fmpz_is_square(fmpq_numref(data)) && fmpz_is_square(fmpq_denref(data));
    }

    std::string tostring() const
    {
        xfstr s(fmpq_get_str(NULL, 10, data));
        return std::string(s.data);
    }
};

class x_fmpq_ball_t {
public:
	_fmpq_ball_t data;

	x_fmpq_ball_t()
	{
		_fmpq_ball_init(data);
	}

	~x_fmpq_ball_t()
	{
		_fmpq_ball_clear(data);
	}

    x_fmpq_ball_t(const x_fmpq_ball_t & other)
    {
		fmpz_init_set(data->left_num, other.data->left_num);
		fmpz_init_set(data->left_den, other.data->left_den);
		fmpz_init_set(data->right_num, other.data->right_num);
		fmpz_init_set(data->right_den, other.data->right_den);
		data->exact = other.data->exact;
    }

    x_fmpq_ball_t(x_fmpq_ball_t && other) noexcept
    {
        _fmpq_ball_init(data);
        _fmpq_ball_swap(data, other.data);
    }

    x_fmpq_ball_t & operator=(x_fmpq_ball_t && other) noexcept
    {
        if (this != &other) {
            _fmpq_ball_init(data);
            _fmpq_ball_swap(data, other.data);
        }  
        return *this;  
    }
};

inline void _fmpz_mat22_swap(_fmpz_mat22_t x, _fmpz_mat22_t y)
{
   _fmpz_mat22_struct t = *x;
   *x = *y;
   *y = t;
}

class x_fmpz_mat22_t {
public:
	_fmpz_mat22_t data;

	x_fmpz_mat22_t()
	{
		_fmpz_mat22_init(data);
	}

	~x_fmpz_mat22_t()
	{
		_fmpz_mat22_clear(data);
	}

    x_fmpz_mat22_t(const x_fmpz_mat22_t & other)
    {
		fmpz_init_set(data->_11, other.data->_11);
		fmpz_init_set(data->_12, other.data->_12);
		fmpz_init_set(data->_21, other.data->_21);
		fmpz_init_set(data->_22, other.data->_22);
		data->det = other.data->det;
    }

    x_fmpz_mat22_t(x_fmpz_mat22_t && other) noexcept
    {
        _fmpz_mat22_init(data);
        _fmpz_mat22_swap(data, other.data);
    }

    x_fmpz_mat22_t & operator=(x_fmpz_mat22_t && other) noexcept
    {
        if (this != &other) {
            _fmpz_mat22_init(data);
            _fmpz_mat22_swap(data, other.data);
        }  
        return *this;  
    }
};

class x_fmpz_vector_t {
public:
	_fmpz_vector_t data;

	x_fmpz_vector_t()
	{
		_fmpz_vector_init(data);
	}

	~x_fmpz_vector_t()
	{
		_fmpz_vector_clear(data);
	}
};


class xfmpz_poly_t {
public:
    fmpz_poly_t data;

    xfmpz_poly_t()
    {
        fmpz_poly_init(data);
    }

    xfmpz_poly_t(slong alloc)
    {
        fmpz_poly_init2(data, alloc);
    }

    xfmpz_poly_t(xfmpz_poly_t&& other) noexcept
    {
        fmpz_poly_init(data);
        fmpz_poly_swap(data, other.data);
    }

    xfmpz_poly_t& operator=(xfmpz_poly_t&& other) noexcept
    {
        if (this != &other) {
            fmpz_poly_init(data);
            fmpz_poly_swap(data, other.data);
        }  
        return *this;  
    }

    xfmpz_poly_t(const xfmpz_poly_t& other)
    {
        fmpz_poly_init(data);
        fmpz_poly_set(data, other.data);
    }

    ~xfmpz_poly_t()
    {
        fmpz_poly_clear(data);
    }

    std::string tostring() const
    {
        xfstr s(fmpz_poly_get_str_pretty(data, "#"));
        return std::string(s.data);
    }
};

class xfmpz_poly_factor_t {
public:
    fmpz_poly_factor_t data;

    xfmpz_poly_factor_t()
    {
        fmpz_poly_factor_init(data);
    }

    ~xfmpz_poly_factor_t()
    {
        fmpz_poly_factor_clear(data);
    }
};


class xfmpq_poly_t {
private:
    fmpz_poly_t zpolydata;
public:
    fmpq_poly_t data;

    xfmpq_poly_t()
    {
        fmpq_poly_init(data);
    }

    xfmpq_poly_t(slong alloc)
    {
        fmpq_poly_init2(data, alloc);
    }

    xfmpq_poly_t(xfmpq_poly_t&& other) noexcept
    {
        fmpq_poly_init(data);
        fmpq_poly_swap(data, other.data);
    }

    xfmpq_poly_t& operator=(xfmpq_poly_t&& other) noexcept
    {
        if (this != &other) {
            fmpq_poly_init(data);
            fmpq_poly_swap(data, other.data);
        }  
        return *this;  
    }

    xfmpq_poly_t(const xfmpq_poly_t& other)
    {
        fmpq_poly_init(data);
        fmpq_poly_set(data, other.data);
    }

    ~xfmpq_poly_t()
    {
        fmpq_poly_clear(data);
    }

    const fmpz_poly_struct * zpoly()
    {
        zpolydata->coeffs = data->coeffs;
        zpolydata->length = data->length;
        zpolydata->alloc = data->alloc;
        return zpolydata;
    }

    std::string tostring() const
    {
        xfstr s(fmpq_poly_get_str_pretty(data, "#"));
        return std::string(s.data);
    }
};

class xfmpq_mpoly_ctx_t {
public:
    fmpq_mpoly_ctx_t data;

    xfmpq_mpoly_ctx_t(slong nvars)
    {
        fmpq_mpoly_ctx_init(data, nvars, ORD_LEX);
    }

    ~xfmpq_mpoly_ctx_t()
    {
        fmpq_mpoly_ctx_clear(data);
    }
};

class xfmpq_mpoly_t {
public:
    fmpq_mpoly_t data;
	const fmpq_mpoly_ctx_struct * ctx;

    xfmpq_mpoly_t(const fmpq_mpoly_ctx_t ctx_)
    {
		ctx = ctx_;
        fmpq_mpoly_init(data, ctx);
    }

    xfmpq_mpoly_t(xfmpq_mpoly_t&& other) noexcept
    {
		ctx = other.ctx;
        fmpq_mpoly_init(data, ctx);
        fmpq_mpoly_swap(data, other.data, ctx);
    }

    xfmpq_mpoly_t& operator=(xfmpq_mpoly_t&& other) noexcept
    {
        if (this != &other) {
			ctx = other.ctx;
            fmpq_mpoly_init(data, ctx);
            fmpq_mpoly_swap(data, other.data, ctx);
        }  
        return *this;  
    }

    xfmpq_mpoly_t(const xfmpq_mpoly_t& other)
    {
		ctx = other.ctx;
        fmpq_mpoly_init(data, ctx);
        fmpq_mpoly_set(data, other.data, ctx);
    }

    ~xfmpq_mpoly_t()
    {
        fmpq_mpoly_clear(data, ctx);
    }

    std::string tostring() const
    {
        xfstr s(fmpq_mpoly_get_str_pretty(data, NULL, ctx));
        return std::string(s.data);
    }
};


class xnmod_mpoly_ctx_t {
public:
    nmod_mpoly_ctx_t data;

    xnmod_mpoly_ctx_t(slong nvars, mp_limb_t modulus)
    {
        nmod_mpoly_ctx_init(data, nvars, ORD_LEX, modulus);
    }

    ~xnmod_mpoly_ctx_t()
    {
        nmod_mpoly_ctx_clear(data);
    }
};

class xnmod_mpoly_t {
public:
    nmod_mpoly_t data;
	const nmod_mpoly_ctx_struct * ctx;

    xnmod_mpoly_t(const nmod_mpoly_ctx_t ctx_)
    {
		ctx = ctx_;
        nmod_mpoly_init(data, ctx);
    }

    xnmod_mpoly_t(xnmod_mpoly_t && other) noexcept
    {
		ctx = other.ctx;
        nmod_mpoly_init(data, ctx);
        nmod_mpoly_swap(data, other.data, ctx);
    }

    xnmod_mpoly_t & operator=(xnmod_mpoly_t && other) noexcept
    {
        if (this != &other) {
			ctx = other.ctx;
            nmod_mpoly_init(data, ctx);
            nmod_mpoly_swap(data, other.data, ctx);
        }  
        return *this;  
    }

    xnmod_mpoly_t(const xnmod_mpoly_t & other)
    {
		ctx = other.ctx;
        nmod_mpoly_init(data, ctx);
        nmod_mpoly_set(data, other.data, ctx);
    }

    ~xnmod_mpoly_t()
    {
        nmod_mpoly_clear(data, ctx);
    }

    std::string tostring() const
    {
        xfstr s(nmod_mpoly_get_str_pretty(data, NULL, ctx));
        return std::string(s.data);
    }
};


class xfmpq_mpoly_factor_t {
public:
    fmpq_mpoly_factor_t data;
	const fmpq_mpoly_ctx_struct * ctx;

    xfmpq_mpoly_factor_t(const fmpq_mpoly_ctx_t ctx_)
    {
		ctx = ctx_;
        fmpq_mpoly_factor_init(data, ctx);
    }

    xfmpq_mpoly_factor_t(xfmpq_mpoly_factor_t && other) noexcept
    {
		ctx = other.ctx;
        fmpq_mpoly_factor_init(data, ctx);
        //fmpq_mpoly_factor_swap(data, other.data, ctx);
    }

    xfmpq_mpoly_factor_t & operator=(xfmpq_mpoly_factor_t && other) noexcept
    {
        if (this != &other) {
			ctx = other.ctx;
            fmpq_mpoly_factor_init(data, ctx);
            //fmpq_mpoly_factor_swap(data, other.data, ctx);
        }  
        return *this;  
    }

    xfmpq_mpoly_factor_t(const xfmpq_mpoly_factor_t & other)
    {
		ctx = other.ctx;
        fmpq_mpoly_factor_init(data, ctx);
        fmpq_mpoly_factor_set(data, other.data, ctx);
    }

    ~xfmpq_mpoly_factor_t()
    {
        fmpq_mpoly_factor_clear(data, ctx);
    }
};

class xnmod_mpoly_factor_t {
public:
    nmod_mpoly_factor_t data;
	const nmod_mpoly_ctx_struct * ctx;

    xnmod_mpoly_factor_t(const nmod_mpoly_ctx_t ctx_)
    {
		ctx = ctx_;
        nmod_mpoly_factor_init(data, ctx);
    }

    xnmod_mpoly_factor_t(xnmod_mpoly_factor_t && other) noexcept
    {
		ctx = other.ctx;
        nmod_mpoly_factor_init(data, ctx);
        nmod_mpoly_factor_swap(data, other.data, ctx);
    }

    xnmod_mpoly_factor_t & operator=(xnmod_mpoly_factor_t && other) noexcept
    {
        if (this != &other) {
			ctx = other.ctx;
            nmod_mpoly_factor_init(data, ctx);
            nmod_mpoly_factor_swap(data, other.data, ctx);
        }  
        return *this;  
    }

    xnmod_mpoly_factor_t(const xnmod_mpoly_factor_t & other)
    {
		ctx = other.ctx;
        nmod_mpoly_factor_init(data, ctx);
        nmod_mpoly_factor_set(data, other.data, ctx);
    }

    ~xnmod_mpoly_factor_t()
    {
        nmod_mpoly_factor_clear(data, ctx);
    }
};


class xmag_t {
public:
    mag_t data;

    xmag_t()
    {
        mag_init(data);
    }

    xmag_t(xmag_t&& other) noexcept
    {
        mag_init(data);
        mag_swap(data, other.data);
    }

    ~xmag_t()
    {
        mag_clear(data);
    }
};


class xarb_t {
public:
    arb_t data;

    xarb_t()
    {
        arb_init(data);
    }

    xarb_t(const xarb_t& other)
    {
        arb_init(data);
        arb_set(data, other.data);
    }

    xarb_t(xarb_t&& other) noexcept
    {
        arb_init(data);
        arb_swap(data, other.data);
    }

    xarb_t& operator=(xarb_t&& other) noexcept
    {
        if (this != &other) {
            arb_init(data);
            arb_swap(data, other.data);
        }  
        return *this;  
    }

    ~xarb_t()
    {
        arb_clear(data);
    }

    slong prec_bits() const 
    {
        return std::max(slong(0), arb_rel_accuracy_bits(data) - 1);
    };

    slong wprec() const 
    {
        return std::max(slong(FLINT_BITS), arb_rel_accuracy_bits(data));
    };

    void limit_prec(slong p)
    {
        assert(p >= 0);
        if (arb_rel_accuracy_bits(data) > p)
        {
            xfmpz_t e;
            fmpz_sub_ui(e.data, e.data, p + 1);
            mag_add_2exp_fmpz(arb_radref(data), arb_radref(data), e.data);
        }
    }

    std::string nativestring(slong digits)
	{
        xfstr s(arb_get_str(data, digits, 0));
        return std::string(s.data);
	}

    std::string tostring() const;
    std::string tostring_full() const;

    void set_abcd(const fmpz_t a, const fmpz_t b, const fmpz_t c, const fmpz_t d);
    void get_abcd(xfmpz_t &a, xfmpz_t &b, xfmpz_t &c, xfmpz_t &d);

};


class xacb_t {
public:
    acb_t data;

    xacb_t()
    {
        acb_init(data);
    }

    xacb_t(const xacb_t& other)
    {
        acb_init(data);
        acb_set(data, other.data);
    }

    xacb_t(xacb_t&& other) noexcept
    {
        acb_init(data);
        acb_swap(data, other.data);
    }

    xacb_t& operator=(xacb_t&& other) noexcept
    {
        if (this != &other) {
            acb_init(data);
            acb_swap(data, other.data);
        }  
        return *this;  
    }

    ~xacb_t()
    {
        acb_clear(data);
    }

    std::string tostring() const
	{
        slong digits = 100;
        xfstr re(arb_get_str(acb_realref(data), digits, 0));
        xfstr im(arb_get_str(acb_imagref(data), digits, 0));
        return std::string(re.data) + " + I*" + std::string(im.data);
	}
};

/* extra useful missing functions */
/*
void fmpz_cdiv_qr(fmpz_t f, fmpz_t s, const fmpz_t g, const fmpz_t h);
void fmpz_poly_inflate(fmpz_poly_t result,
                                     const fmpz_poly_t input, ulong inflation);
*/
