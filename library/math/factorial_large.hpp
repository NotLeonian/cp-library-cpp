#ifndef SUISEN_FACTORIAL_LARGE
#define SUISEN_FACTORIAL_LARGE

#include <utility>

#include "library/polynomial/shift_of_sampling_points.hpp"
#include "library/convolution/arbitrary_mod_convolution.hpp"
#include "library/math/factorial.hpp"

namespace suisen {
    // mod must be a prime number
    template <typename mint,
        std::enable_if_t<atcoder::internal::is_static_modint<mint>::value, std::nullptr_t> = nullptr>
    struct factorial_large {
        using value_type = mint;

        static constexpr int LOG_BLOCK_SIZE = 9;
        static constexpr int BLOCK_SIZE = 1 << LOG_BLOCK_SIZE;
        static constexpr int BLOCK_NUM = value_type::mod() >> LOG_BLOCK_SIZE;

        static inline int threshold = 2000000;

        static_assert(atcoder::internal::is_prime_constexpr(mint::mod()));

        static value_type fac(long long n) {
            if (n >= mint::mod()) return 0;
            return n <= threshold ? factorial<mint>{}.fac(n) : _large_fac(n);
        }
        static value_type fac_inv(long long n) {
            assert(n < (long long) mint::mod());
            return n <= threshold ? factorial<mint>{}.fac_inv(n) : _large_fac(n).inv();
        }
        static value_type binom(long long n, long long r) {
            if (n < 0 or r < 0 or n < r) return 0;
            return _binom_lucas(n, r);
        }
        // binom(n, r) の逆数
        // binom(n, r) = 0 の場合は assert 違反となる
        static value_type binom_inv(long long n, long long r) {
            assert(r >= 0 and n >= r);
            return _binom_inv_lucas(n, r);
        }
        // n 種類から重複を許して r 個選ぶ場合の数
        // x_1+x_2+...+x_n=r（x_i は非負整数）となる x の個数でもある
        // multichoose(n, r) = binom(n + r - 1, r)
        static value_type multichoose(long long n, long long r) {
            if (n < 0 or r < 0) return 0;
            if (r == 0) return value_type(1);
            if (n == 0) return value_type(0);
            return binom(n + r - 1, r);
        }
        // n 種類から重複を許して r 個選ぶ場合の数 multichoose(n, r) の逆数
        // x_1+x_2+...+x_n=r（x_i は非負整数）となる x の個数の逆数でもある
        // multichoose(n, r) = binom(n + r - 1, r)
        // multichoose(n, r) = 0 の場合は assert 違反となる
        static value_type multichoose_inv(long long n, long long r) {
            assert(n >= 0 and r >= 0);
            if (r == 0) return value_type(1);
            assert(n > 0);
            return binom_inv(n + r - 1, r);
        }
        template <typename ...Ds, std::enable_if_t<std::conjunction_v<std::is_integral<Ds>...>, std::nullptr_t> = nullptr>
        static value_type polynom(const int n, const Ds& ...ds) {
            if (n < 0) return 0;
            long long sumd = 0;
            value_type res = fac(n);
            for (int d : { ds... }) {
                if (d < 0 or d > n) return 0;
                sumd += d;
                res *= fac_inv(d);
            }
            if (sumd > n) return 0;
            res *= fac_inv(n - sumd);
            return res;
        }
        static value_type perm(int n, int r) {
            if (r < 0 or r > n) return 0;
            return fac(n) * fac_inv(n - r);
        }
    private:
        static value_type _binom_under_mod(long long n, long long r) {
            if (r < 0 or n < r) return 0;
            return fac(n) * fac_inv(r) * fac_inv(n - r);
        }
        static value_type _binom_inv_under_mod(long long n, long long r) {
            assert(r >= 0 and n >= r);
            return fac_inv(n) * fac(r) * fac(n - r);
        }
        static value_type _binom_lucas(long long n, long long r) {
            if (n < 0 or r < 0 or n < r) return 0;

            value_type res = 1;
            const int p = value_type::mod();

            while (n or r) {
                const int ni = n % p;
                const int ri = r % p;

                if (ri > ni) return 0;
                res *= _binom_under_mod(ni, ri);

                n /= p;
                r /= p;
            }
            return res;
        }
        static value_type _binom_inv_lucas(long long n, long long r) {
            assert(0 <= r and r <= n);

            value_type res = 1;
            const int p = value_type::mod();

            while (n or r) {
                const int ni = n % p;
                const int ri = r % p;

                assert(ri <= ni);
                res *= _binom_inv_under_mod(ni, ri);

                n /= p;
                r /= p;
            }
            return res;
        }

        static inline std::vector<value_type> _block_fac{};

        static void _build() {
            if (_block_fac.size()) {
                return;
            }
            std::vector<value_type> f{ 1 };
            f.reserve(BLOCK_SIZE);
            for (int i = 0; i < LOG_BLOCK_SIZE; ++i) {
                std::vector<value_type> g = shift_of_sampling_points<value_type>(f, 1 << i, 3 << i, arbitrary_mod_convolution<value_type>);
                const auto get = [&](int j) { return j < (1 << i) ? f[j] : g[j - (1 << i)]; };
                f.resize(2 << i);
                for (int j = 0; j < 2 << i; ++j) {
                    f[j] = get(2 * j) * get(2 * j + 1) * ((2 * j + 1) << i);
                }
            }
            // f_B(x) = (x+1) * ... * (x+B-1)
            if (BLOCK_NUM > BLOCK_SIZE) {
                std::vector<value_type> g = shift_of_sampling_points<value_type>(f, BLOCK_SIZE, BLOCK_NUM - BLOCK_SIZE, arbitrary_mod_convolution<value_type>);
                std::move(g.begin(), g.end(), std::back_inserter(f));
            } else {
                f.resize(BLOCK_NUM);
            }
            for (int i = 0; i < BLOCK_NUM; ++i) {
                f[i] *= value_type(i + 1) * BLOCK_SIZE;
            }
            // f[i] = (i*B + 1) * ... * (i*B + B)

            f.insert(f.begin(), 1);
            for (int i = 1; i <= BLOCK_NUM; ++i) {
                f[i] *= f[i - 1];
            }
            _block_fac = std::move(f);
        }

        static value_type _large_fac(int n) {
            _build();
            value_type res;
            int q = n / BLOCK_SIZE, r = n % BLOCK_SIZE;
            if (2 * r <= BLOCK_SIZE) {
                res = _block_fac[q];
                for (int i = 0; i < r; ++i) {
                    res *= value_type::raw(n - i);
                }
            } else if (q != factorial_large::BLOCK_NUM) {
                res = _block_fac[q + 1];
                value_type den = 1;
                for (int i = 1; i <= BLOCK_SIZE - r; ++i) {
                    den *= value_type::raw(n + i);
                }
                res /= den;
            } else {
                // Wilson's theorem
                res = value_type::mod() - 1;
                value_type den = 1;
                for (int i = value_type::mod() - 1; i > n; --i) {
                    den *= value_type::raw(i);
                }
                res /= den;
            }
            return res;
        }
    };
} // namespace suisen


#endif // SUISEN_FACTORIAL_LARGE
