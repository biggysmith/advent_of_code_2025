#include <vector>

template<typename Z = long long>
struct rational {
    Z num = 0;
    Z den = 1;

    rational() = default;

    rational(Z n) : num(n), den(1) {}

    rational(Z n, Z d) : num(n), den(d) { normalize(); }

    void normalize() {
        if(den < 0) { den = -den; num = -num; }
        if(den == 0) { num = 1; return; }
        Z g = std::gcd(num, den);
        if(g != 0) { num /= g; den /= g; }
    }

    bool isZero() const { return num == 0; }

    rational operator-() const { return rational(-num, den); }

    rational& operator+=(const rational& o) {
        num = num * o.den + o.num * den;
        den *= o.den;
        normalize();
        return *this;
    }

    rational& operator-=(const rational& o) {
        num = num * o.den - o.num * den;
        den *= o.den;
        normalize();
        return *this;
    }

    rational& operator*=(const rational& o) {
        num *= o.num;
        den *= o.den;
        normalize();
        return *this;
    }

    rational& operator/=(const rational& o) {
        num *= o.den;
        den *= o.num;
        normalize();
        return *this;
    }

    friend rational operator+(rational a, const rational& b) { return a += b; }
    friend rational operator-(rational a, const rational& b) { return a -= b; }
    friend rational operator*(rational a, const rational& b) { return a *= b; }
    friend rational operator/(rational a, const rational& b) { return a /= b; }
};

template<typename T>
struct matrix {
    size_t rows = 0, cols = 0;

    std::vector<T> data;

    matrix() = default;

    matrix(size_t r, size_t c) : rows(r), cols(c), data(r*c) {}

    matrix(size_t r, size_t c, T v) : rows(r), cols(c), data(r*c, v) {}

    T& operator()(size_t r, size_t c) { return data[r * cols + c]; }

    const T& operator()(size_t r, size_t c) const { return data[r * cols + c]; }

    void swap_rows(size_t a, size_t b) {
        if(a == b) return;
        for(size_t c = 0; c < cols; ++c)
            std::swap((*this)(a, c), (*this)(b, c));
    }
};

template<typename Z>
int rref(matrix<rational<Z>>& A)
{
    size_t rows = A.rows, cols = A.cols;
    size_t lead = 0;
    int rank = 0;

    for(size_t r = 0; r < rows && lead < cols; ++r) {
        size_t pivot = r;
        while (pivot < rows && A(pivot, lead).isZero())
            ++pivot;

        if(pivot == rows) {
            ++lead;
            --r;
            continue;
        }

        A.swap_rows(r, pivot);

        auto div = A(r, lead);
        for(size_t c = 0; c < cols; ++c)
            A(r, c) /= div;

        for(size_t rr = 0; rr < rows; ++rr) {
            if(rr == r) continue;
            auto f = A(rr, lead);
            if(f.isZero()) continue;
            for(size_t c = 0; c < cols; ++c)
                A(rr, c) -= f * A(r, c);
        }

        ++lead;
        ++rank;
    }

    return rank;
}

template<typename Z>
matrix<rational<Z>> augment(const matrix<rational<Z>>& A, const std::vector<rational<Z>>& b)
{
    matrix<rational<Z>> out(A.rows, A.cols + 1);
    for(size_t r = 0; r < A.rows; ++r) {
        for(size_t c = 0; c < A.cols; ++c)
            out(r, c) = A(r, c);
        out(r, A.cols) = b[r];
    }
    return out;
}

template<typename Z>
struct linear_solution {
    bool inconsistent = false;
    size_t n_vars = 0;

    std::vector<rational<Z>> particular;
    std::vector<std::vector<rational<Z>>> dirs;

    std::vector<int> pivot_col_by_row;
    std::vector<bool> is_pivot;
    std::vector<int> free_cols;
};

template<typename Z>
linear_solution<Z> extract_solution(const matrix<rational<Z>>& Ab)
{
    linear_solution<Z> sol;
    size_t rows = Ab.rows;
    size_t n_vars = Ab.cols - 1;
    sol.n_vars = n_vars;

    sol.particular.assign(n_vars, rational<Z>(0));
    sol.pivot_col_by_row.assign(rows, -1);
    sol.is_pivot.assign(n_vars, false);

    for(size_t r = 0; r < rows; ++r) {
        size_t c = 0;
        while (c < n_vars && Ab(r, c).isZero()) ++c;

        if(c == n_vars) {
            if(!Ab(r, n_vars).isZero()) {
                sol.inconsistent = true;
                return sol;
            }
        } else {
            sol.pivot_col_by_row[r] = (int)c;
            sol.is_pivot[c] = true;
        }
    }

    for(size_t c = 0; c < n_vars; ++c)
        if(!sol.is_pivot[c]) sol.free_cols.push_back((int)c);

    for(size_t r = 0; r < rows; ++r) {
        int pc = sol.pivot_col_by_row[r];
        if(pc >= 0)
            sol.particular[pc] = Ab(r, n_vars);
    }

    size_t k = sol.free_cols.size();
    sol.dirs.resize(k);

    for(size_t j = 0; j < k; ++j) {
        int f = sol.free_cols[j];
        std::vector<rational<Z>> dir(n_vars, rational<Z>(0));
        dir[f] = rational<Z>(1);

        for(size_t r = 0; r < rows; ++r) {
            int pc = sol.pivot_col_by_row[r];
            if(pc >= 0)
                dir[pc] -= Ab(r, f);
        }

        sol.dirs[j] = dir;
    }

    return sol;
}
