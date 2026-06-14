---
data:
  _extendedDependsOn:
  - icon: ':heavy_check_mark:'
    path: library/math/factorial.hpp
    title: "\u968E\u4E57\u30C6\u30FC\u30D6\u30EB"
  _extendedRequiredBy:
  - icon: ':heavy_check_mark:'
    path: library/sequence/stirling_number1_small_prime_mod.hpp
    title: Stirling Number of the First Kind (Small Prime Mod)
  - icon: ':heavy_check_mark:'
    path: library/sequence/stirling_number2_small_prime_mod.hpp
    title: Stirling Number2 Small Prime Mod
  _extendedVerifiedWith:
  - icon: ':heavy_check_mark:'
    path: test/src/sequence/stirling_number1_small_prime_mod/stirling_number_of_the_first_kind_small_p_large_n.test.cpp
    title: test/src/sequence/stirling_number1_small_prime_mod/stirling_number_of_the_first_kind_small_p_large_n.test.cpp
  - icon: ':heavy_check_mark:'
    path: test/src/sequence/stirling_number2_small_prime_mod/stirling_number_of_the_second_kind_small_p_large_n.test.cpp
    title: test/src/sequence/stirling_number2_small_prime_mod/stirling_number_of_the_second_kind_small_p_large_n.test.cpp
  _isVerificationFailed: false
  _pathExtension: hpp
  _verificationStatusIcon: ':heavy_check_mark:'
  attributes:
    links: []
  bundledCode: "#line 1 \"library/sequence/binomial_coefficient_small_prime_mod.hpp\"\
    \n\n\n\n#line 1 \"library/math/factorial.hpp\"\n\n\n\n#include <cassert>\n#include\
    \ <vector>\n\nnamespace suisen {\n    template <typename T, typename U = T>\n\
    \    struct factorial {\n        factorial() = default;\n        factorial(int\
    \ n) { ensure(n); }\n\n        static void ensure(const int n) {\n           \
    \ int sz = _fac.size();\n            if (n + 1 <= sz) return;\n            int\
    \ new_size = std::max(n + 1, sz * 2);\n            _fac.resize(new_size), _fac_inv.resize(new_size);\n\
    \            for (int i = sz; i < new_size; ++i) _fac[i] = _fac[i - 1] * i;\n\
    \            _fac_inv[new_size - 1] = U(1) / _fac[new_size - 1];\n           \
    \ for (int i = new_size - 1; i > sz; --i) _fac_inv[i - 1] = _fac_inv[i] * i;\n\
    \        }\n\n        T fac(const int i) {\n            ensure(i);\n         \
    \   return _fac[i];\n        }\n        T operator()(int i) {\n            return\
    \ fac(i);\n        }\n        U fac_inv(const int i) {\n            ensure(i);\n\
    \            return _fac_inv[i];\n        }\n        U binom(const int n, const\
    \ int r) {\n            if (n < 0 or r < 0 or n < r) return 0;\n            ensure(n);\n\
    \            return _fac[n] * _fac_inv[r] * _fac_inv[n - r];\n        }\n    \
    \    // binom(n, r) \u306E\u9006\u6570\n        // binom(n, r) = 0 \u306E\u5834\
    \u5408\u306F assert \u9055\u53CD\u3068\u306A\u308B\n        U binom_inv(const\
    \ int n, const int r) {\n            assert(r >= 0 and n >= r);\n            ensure(n);\n\
    \            return _fac_inv[n] * _fac[r] * _fac[n - r];\n        }\n        //\
    \ n \u7A2E\u985E\u304B\u3089\u91CD\u8907\u3092\u8A31\u3057\u3066 r \u500B\u9078\
    \u3076\u5834\u5408\u306E\u6570\n        // x_1+x_2+...+x_n=r\uFF08x_i \u306F\u975E\
    \u8CA0\u6574\u6570\uFF09\u3068\u306A\u308B x \u306E\u500B\u6570\u3067\u3082\u3042\
    \u308B\n        // multichoose(n, r) = binom(n + r - 1, r)\n        U multichoose(const\
    \ int n, const int r) {\n            if (n < 0 or r < 0) return 0;\n         \
    \   return r > 0 ? binom(n + r - 1, r) : U(1);\n        }\n        // n \u7A2E\
    \u985E\u304B\u3089\u91CD\u8907\u3092\u8A31\u3057\u3066 r \u500B\u9078\u3076\u5834\
    \u5408\u306E\u6570 multichoose(n, r) \u306E\u9006\u6570\n        // x_1+x_2+...+x_n=r\uFF08\
    x_i \u306F\u975E\u8CA0\u6574\u6570\uFF09\u3068\u306A\u308B x \u306E\u500B\u6570\
    \u306E\u9006\u6570\u3067\u3082\u3042\u308B\n        // multichoose(n, r) = binom(n\
    \ + r - 1, r)\n        // multichoose(n, r) = 0 \u306E\u5834\u5408\u306F assert\
    \ \u9055\u53CD\u3068\u306A\u308B\n        U multichoose_inv(const int n, const\
    \ int r) {\n            assert(n >= 0 and r >= 0);\n            return r > 0 ?\
    \ binom_inv(n + r - 1, r) : U(1);\n        }\n        template <typename ...Ds,\
    \ std::enable_if_t<std::conjunction_v<std::is_integral<Ds>...>, std::nullptr_t>\
    \ = nullptr>\n        U polynom(const int n, const Ds& ...ds) {\n            if\
    \ (n < 0) return 0;\n            ensure(n);\n            int sumd = 0;\n     \
    \       U res = _fac[n];\n            for (int d : { ds... }) {\n            \
    \    if (d < 0 or d > n) return 0;\n                sumd += d;\n             \
    \   res *= _fac_inv[d];\n            }\n            if (sumd > n) return 0;\n\
    \            res *= _fac_inv[n - sumd];\n            return res;\n        }\n\
    \        U perm(const int n, const int r) {\n            if (n < 0 or r < 0 or\
    \ n < r) return 0;\n            ensure(n);\n            return _fac[n] * _fac_inv[n\
    \ - r];\n        }\n        // perm(n, r) \u306E\u9006\u6570\n        // perm(n,\
    \ r) = 0 \u306E\u5834\u5408\u306F assert \u9055\u53CD\u3068\u306A\u308B\n    \
    \    U perm_inv(const int n, const int r) {\n            assert(r >= 0 and n >=\
    \ r);\n            ensure(n);\n            return _fac_inv[n] * _fac[n - r];\n\
    \        }\n    private:\n        static std::vector<T> _fac;\n        static\
    \ std::vector<U> _fac_inv;\n    };\n    template <typename T, typename U>\n  \
    \  std::vector<T> factorial<T, U>::_fac{ 1 };\n    template <typename T, typename\
    \ U>\n    std::vector<U> factorial<T, U>::_fac_inv{ 1 };\n} // namespace suisen\n\
    \n\n#line 5 \"library/sequence/binomial_coefficient_small_prime_mod.hpp\"\n\n\
    namespace suisen {\n    template <typename mint>\n    struct BinomialCoefficientSmallPrimeMod\
    \ {\n        mint operator()(long long n, long long r) const {\n            return\
    \ binom(n, r);\n        }\n        static mint binom(long long n, long long r)\
    \ {\n            factorial<mint> fac(mint::mod() - 1);\n\n            if (r <\
    \ 0 or n < r) return 0;\n            r = std::min(r, n - r);\n            // Lucas's\
    \ theorem\n            mint res = 1;\n            while (r) {\n              \
    \  int ni = n % mint::mod(), ri = r % mint::mod();\n                if (ni < ri)\
    \ return 0;\n                res *= fac.binom(ni, ri);\n                n = n\
    \ / mint::mod(), r = r / mint::mod();\n            }\n            return res;\n\
    \        }\n    };\n} // namespace suisen\n\n\n\n"
  code: "#ifndef SUISEN_BINOMIAL_COEFFICIENT_SMALL_P\n#define SUISEN_BINOMIAL_COEFFICIENT_SMALL_P\n\
    \n#include \"library/math/factorial.hpp\"\n\nnamespace suisen {\n    template\
    \ <typename mint>\n    struct BinomialCoefficientSmallPrimeMod {\n        mint\
    \ operator()(long long n, long long r) const {\n            return binom(n, r);\n\
    \        }\n        static mint binom(long long n, long long r) {\n          \
    \  factorial<mint> fac(mint::mod() - 1);\n\n            if (r < 0 or n < r) return\
    \ 0;\n            r = std::min(r, n - r);\n            // Lucas's theorem\n  \
    \          mint res = 1;\n            while (r) {\n                int ni = n\
    \ % mint::mod(), ri = r % mint::mod();\n                if (ni < ri) return 0;\n\
    \                res *= fac.binom(ni, ri);\n                n = n / mint::mod(),\
    \ r = r / mint::mod();\n            }\n            return res;\n        }\n  \
    \  };\n} // namespace suisen\n\n\n#endif // SUISEN_BINOMIAL_COEFFICIENT_SMALL_P\n"
  dependsOn:
  - library/math/factorial.hpp
  isVerificationFile: false
  path: library/sequence/binomial_coefficient_small_prime_mod.hpp
  requiredBy:
  - library/sequence/stirling_number2_small_prime_mod.hpp
  - library/sequence/stirling_number1_small_prime_mod.hpp
  timestamp: '2026-06-14 12:42:12+09:00'
  verificationStatus: LIBRARY_ALL_AC
  verifiedWith:
  - test/src/sequence/stirling_number2_small_prime_mod/stirling_number_of_the_second_kind_small_p_large_n.test.cpp
  - test/src/sequence/stirling_number1_small_prime_mod/stirling_number_of_the_first_kind_small_p_large_n.test.cpp
documentation_of: library/sequence/binomial_coefficient_small_prime_mod.hpp
layout: document
title: Binomial Coefficient Small Prime Mod
---
## Binomial Coefficient Small Prime Mod

以下に示す Lucas の定理を用いる。

> $p$ が素数のとき、非負整数 $n, r$ に対して次が成り立つ:
> $$\binom{n}{r} \equiv \prod _ {i = 0} ^ {k - 1} \binom{n _ i}{r _ i} \pmod{p}.$$
> ここで、$n$ を $p$ 進表記したときの $i$ 桁目を $n_i$ とした ($r$ についても同様)。

全ての $0\leq n\lt p,0\leq r\lt p$ に対する $\displaystyle \binom{n}{r} \bmod p$ を $O(p ^ 2)$ 時間だけ掛けて前計算しておくことで、クエリあたり $O(\log _ p n)$ で計算できる。
