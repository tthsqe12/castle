#pragma once
#include "ex_types.h"
#include "hash.h"

/*
    classes that act like unique_ptr's to expressions
*/

/* in uex release() allowed; nullptr could appear */
class uex {

public:
    er data;

    uex(): data(nullptr) {}
    uex(ex d): data(etor(d)) {}
//  uex(er d): data(eto_er(ecopy(d))) {}
    uex (const uex & other): data(etor(ecopy(other.data))) {}
    ~uex() {if (data != nullptr) eclear(etox(data));}

    uex (uex && other) noexcept: data(other.data) {
        other.data = nullptr;
    }
    uex& operator=(uex&& other) noexcept {
        data = other.data;
        other.data = nullptr;
//        assert(data != nullptr);
        return *this;  
    }
    uex& operator=(const uex& other) noexcept {
        if (data != nullptr) eclear(etox(data));
        data = etor(ecopy(other.data));
        return *this;  
    }

    er get() const {
        return data;
    }

    fmpz* int_data() const {
        return eint_data(data);
    }

    void swap(uex & other) {
        er t = other.data;
        other.data = data;
        data = t;
    }

    ex copy() const {
        return ecopy(data);
    }
    er copyr() const {
        return etor(ecopy(data));
    }
    ex copynull() const {
        return data == nullptr ? nullptr : ecopy(data);
    }

    ex copychild(size_t i) const {
        return ecopychild(data, i);
    }

    er child(size_t i) const {
        return echild(data, i);
    }

    ex release() {
        ex datacopy = etox(data);
        data = nullptr;
        return datacopy;
    }
    er releaser() {
        er datacopy = data;
        data = nullptr;
        return datacopy;
    }
    void zrelease() {
        assert(data == nullptr);
        data = nullptr;
    }

    ex swap_out(ex other) {
        ex t = etox(data);
        data = etor(other);
        return t;
    }

    void reset(ex n) {
        if (data != nullptr)
            eclear(etox(data));
        data = etor(n);
    }

    void set(ex n) {
        assert(data == nullptr);
        data = etor(n);
    }

    void setz(ex n) {
        assert(data == nullptr);
        data = etor(n);
    }

    void setnz(ex n) {
        assert(data != nullptr);
        eclear(etox(data));
        data = etor(n);
    }

    /* push_back is meant to be used only with this constructor */
    uex(er h, size_t length) {
        data = etor(emake_node_reserve(h, length));
    }
    uex(ex h, size_t length) {
        data = etor(emake_node_reserve(h, length));
    }
    void init_push_back(er h, size_t length) {
        assert(data == nullptr);
        data = etor(emake_node_reserve(h, length));
    }
    void init_push_back(ex h, size_t length) {
        assert(data == nullptr);
        data = etor(emake_node_reserve(h, length));
    }
    void push_back(ex nextchild) {
        ex_node e = eto_node(data);
        e->length++;
        e->child[e->length] = etor(nextchild);
        e->head.bucketflags |= ebucketflags(nextchild);
    }

    void replacechild(size_t i, ex F);
    void insertchild(size_t i, ex F);
    void removechild(size_t i);
};


/* in wex release() not allowed; nullptr never appear */
class wex {
public:
    er data;

    wex(): data(nullptr) {assert(false);}
    wex(ex d) noexcept : data(etor(d)) {assert(d != nullptr);}
    wex (const wex & other) noexcept : data(etor(ecopy(other.data))) {}
    wex (wex && other) noexcept : data(etor(ecopy(other.data))) {}
    ~wex() {assert(data != nullptr); eclear(etox(data));}

    er get() const {
        return data;
    }

    fmpz* int_data() const {
        return eint_data(data);
    }

    void swap(wex & other) {
        er t = other.data;
        other.data = data;
        data = t;
    }

    ex swap_out(ex other) {
        ex t = etox(data);
        data = etor(other);
        return t;
    }

    ex copy() const {
        return ecopy(data);
    }
    er copyr() const {
        return etor(ecopy(data));
    }

    ex copychild(size_t i) const {
        return ecopychild(data, i);
    }

    er child(size_t i) const {
        return echild(data, i);
    }

    wex& operator=(const wex& other) noexcept {
        assert(data != nullptr);
        eclear(etox(data));
        data = etor(ecopy(other.data));
        return *this; 
    }
    wex& operator=(wex&& other) noexcept {
        assert(data != nullptr);
        eclear(etox(data));
        data = etor(ecopy(other.data));
        return *this; 
    }
    void reset(ex n) {
        assert(data != nullptr);
        eclear(etox(data));
        data = etor(n);
    }
    void replacechild(size_t i, ex F);
    void insertchild(size_t i, ex F);
    void removechild(size_t i);
};


int ex_cmp(er e1, er e2);

class exaddr_hasher
{
public:
    size_t operator() (er const& key) const
    {
        size_t hash = (size_t)(key);
//std::cout << "addr hashing " << hash;
        hash = (hash >> 4) | (hash << (8*sizeof(size_t) - 4));
        hash ^= hash >> 12;
        hash ^= hash << 25;
        hash ^= hash >> 27;
//std::cout << " returning " << hash << std::endl;
        return hash;
    }
};

class exaddr_equal
{
public:
    bool operator() (er const & e1, er const & e2) const
    {
/*        std::cout << "addr equal called" << std::endl;*/
        return e1 == e2;
    }
};

class uexmap_hasher
{
public:
    size_t operator() (const uex & key) const
    {
//        std::cout << "map hashing " << ex_tostring_full(key.get());
        size_t hash = ex_hash(key.get());
//        std::cout << " returning " << hash << std::endl;
        return hash;
    }
};




struct ex_compare {
    bool operator() (const er& lhs, const er& rhs) const {
        return ex_cmp(lhs, rhs) < 0;
    }
};

struct uex_compare {
    bool operator() (const uex& lhs, const uex& rhs) const {
        return ex_cmp(lhs.get(), rhs.get()) < 0;
    }
};

struct uex_str_compare {
    bool operator() (const uex& e1, const uex& e2) const
    {
        return estr_string(e1.get()) < estr_string(e2.get());
    }
};

struct uex_sym_compare {
    bool operator() (const uex& e1, const uex& e2) const
    {
        int c = (estr_string(esym_name(e1.get()))).compare(estr_string(esym_name(e2.get())));
        if (c != 0)
            return c < 0;
        return (estr_string(esym_context(e1.get()))).compare(estr_string(esym_context(e2.get()))) < 0;
    }
};

struct er_sym_compare {
    bool operator() (const er& e1, const er& e2) const
    {
        int c = (estr_string(esym_name(e1))).compare(estr_string(esym_name(e2)));
        if (c != 0)
            return c < 0;
        return (estr_string(esym_context(e1))).compare(estr_string(esym_context(e2))) < 0;
    }
};


struct wex_hasher {
    size_t operator() (const wex & key) const
    {
        size_t hash = ex_hash(key.get());
//        std::cout << "hash of " << ex_tostring_full(key.get()) << std::endl;
//        std::cout << "is " << hash << std::endl;
        return hash;
    }
};

struct wex_same {
    bool operator() (const wex& lhs, const wex& rhs) const {
        return ex_same(lhs.get(), rhs.get());
    }
};




ex emake_node(ex H, std::vector<uex>& argl);
ex emake_node(ex H, std::vector<wex>& argl);
ex emake_node(ex H, size_t start, std::vector<uex>& argl);
ex emake_node(ex H, std::vector<uex>& argl1, std::vector<uex>& argl2);

ex emake_node(ex H, std::vector<uex>& argl1);
ex emake_node(ex H, std::vector<uex>& argl1, std::vector<uex>& argl2);
ex emake_node(ex H, std::vector<uex>& argl1, std::vector<uex>& argl2, std::vector<uex>& argl3);

ex emake_node_plus(std::vector<uex>& argl);
ex emake_node_times(std::vector<uex>& argl);
ex emake_node_times(std::vector<wex>& argl);

void print_exstack(std::vector<uex> &estack);
void print_exstack(std::vector<wex> &estack);
