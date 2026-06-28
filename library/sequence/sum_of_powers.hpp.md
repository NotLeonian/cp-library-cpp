---
data:
  _extendedDependsOn:
  - icon: ':heavy_check_mark:'
    path: library/math/factorial.hpp
    title: "\u968E\u4E57\u30C6\u30FC\u30D6\u30EB"
  - icon: ':heavy_check_mark:'
    path: library/math/pow_mods.hpp
    title: "\u51AA\u4E57\u30C6\u30FC\u30D6\u30EB"
  - icon: ':heavy_check_mark:'
    path: library/sequence/bernoulli_number.hpp
    title: Bernoulli Number
  _extendedRequiredBy: []
  _extendedVerifiedWith: []
  _isVerificationFailed: false
  _pathExtension: hpp
  _verificationStatusIcon: ':warning:'
  attributes:
    links: []
  bundledCode: "#line 1 \"library/sequence/sum_of_powers.hpp\"\n\n\n\n#line 1 \"library/math/pow_mods.hpp\"\
    \n\n\n\n#include <vector>\n\nnamespace suisen {\n    template <int base_as_int,\
    \ typename mint>\n    struct static_pow_mods {\n        static_pow_mods() = default;\n\
    \        static_pow_mods(int n) { ensure(n); }\n        const mint& operator[](int\
    \ i) const {\n            ensure(i);\n            return pows[i];\n        }\n\
    \        static void ensure(int n) {\n            int sz = pows.size();\n    \
    \        if (sz > n) return;\n            pows.resize(n + 1);\n            for\
    \ (int i = sz; i <= n; ++i) pows[i] = base * pows[i - 1];\n        }\n    private:\n\
    \        static inline std::vector<mint> pows { 1 };\n        static inline mint\
    \ base = base_as_int;\n        static constexpr int mod = mint::mod();\n    };\n\
    \n    template <typename mint>\n    struct pow_mods {\n        pow_mods() = default;\n\
    \        pow_mods(mint base, int n) : base(base) { ensure(n); }\n        const\
    \ mint& operator[](int i) const {\n            ensure(i);\n            return\
    \ pows[i];\n        }\n        void ensure(int n) const {\n            int sz\
    \ = pows.size();\n            if (sz > n) return;\n            pows.resize(n +\
    \ 1);\n            for (int i = sz; i <= n; ++i) pows[i] = base * pows[i - 1];\n\
    \        }\n    private:\n        mutable std::vector<mint> pows { 1 };\n    \
    \    mint base;\n        static constexpr int mod = mint::mod();\n    };\n}\n\n\
    \n#line 1 \"library/sequence/bernoulli_number.hpp\"\n\n\n\n#line 1 \"library/math/factorial.hpp\"\
    \n\n\n\n#include <cassert>\n#line 6 \"library/math/factorial.hpp\"\n\nnamespace\
    \ suisen {\n    // \u5F15\u6570\u3068\u3057\u3066\u4E0E\u3048\u308B\u5024\u306B\
    \u5BFE\u3057\u3066\u3001\u6CD5\u304C\u5341\u5206\u5927\u304D\u3044\u3053\u3068\
    \u3092\u4EEE\u5B9A\u3059\u308B\n    template <typename T, typename U = T>\n  \
    \  struct factorial {\n        factorial() = default;\n        factorial(int n)\
    \ { ensure(n); }\n\n        static void ensure(const int n) {\n            int\
    \ sz = _fac.size();\n            if (n + 1 <= sz) return;\n            int new_size\
    \ = std::max(n + 1, sz * 2);\n            _fac.resize(new_size), _fac_inv.resize(new_size);\n\
    \            for (int i = sz; i < new_size; ++i) _fac[i] = _fac[i - 1] * i;\n\
    \            _fac_inv[new_size - 1] = U(1) / _fac[new_size - 1];\n           \
    \ for (int i = new_size - 1; i > sz; --i) _fac_inv[i - 1] = _fac_inv[i] * i;\n\
    \        }\n\n        T fac(const int i) {\n            ensure(i);\n         \
    \   return _fac[i];\n        }\n        T operator()(int i) {\n            return\
    \ fac(i);\n        }\n        U fac_inv(const int i) {\n            ensure(i);\n\
    \            return _fac_inv[i];\n        }\n        // i \u306E\u9006\u6570\n\
    \        // i = 0 \u306E\u5834\u5408\u306F assert \u9055\u53CD\u3068\u306A\u308B\
    \n        U inv(const int i) {\n            assert(i > 0);\n            ensure(i);\n\
    \            return _fac_inv[i] * _fac[i - 1];\n        }\n        U binom(const\
    \ int n, const int r) {\n            if (n < 0 or r < 0 or n < r) return 0;\n\
    \            ensure(n);\n            return _fac[n] * _fac_inv[r] * _fac_inv[n\
    \ - r];\n        }\n        // binom(n, r) \u306E\u9006\u6570\n        // binom(n,\
    \ r) = 0 \u306E\u5834\u5408\u306F assert \u9055\u53CD\u3068\u306A\u308B\n    \
    \    U binom_inv(const int n, const int r) {\n            assert(r >= 0 and n\
    \ >= r);\n            ensure(n);\n            return _fac_inv[n] * _fac[r] * _fac[n\
    \ - r];\n        }\n        // n \u7A2E\u985E\u304B\u3089\u91CD\u8907\u3092\u8A31\
    \u3057\u3066 r \u500B\u9078\u3076\u5834\u5408\u306E\u6570\n        // x_1+x_2+...+x_n=r\uFF08\
    x_i \u306F\u975E\u8CA0\u6574\u6570\uFF09\u3068\u306A\u308B x \u306E\u500B\u6570\
    \u3067\u3082\u3042\u308B\n        // multichoose(n, r) = binom(n + r - 1, r)\n\
    \        U multichoose(const int n, const int r) {\n            if (n < 0 or r\
    \ < 0) return 0;\n            return r > 0 ? binom(n + r - 1, r) : U(1);\n   \
    \     }\n        // n \u7A2E\u985E\u304B\u3089\u91CD\u8907\u3092\u8A31\u3057\u3066\
    \ r \u500B\u9078\u3076\u5834\u5408\u306E\u6570 multichoose(n, r) \u306E\u9006\u6570\
    \n        // x_1+x_2+...+x_n=r\uFF08x_i \u306F\u975E\u8CA0\u6574\u6570\uFF09\u3068\
    \u306A\u308B x \u306E\u500B\u6570\u306E\u9006\u6570\u3067\u3082\u3042\u308B\n\
    \        // multichoose(n, r) = binom(n + r - 1, r)\n        // multichoose(n,\
    \ r) = 0 \u306E\u5834\u5408\u306F assert \u9055\u53CD\u3068\u306A\u308B\n    \
    \    U multichoose_inv(const int n, const int r) {\n            assert(n >= 0\
    \ and r >= 0);\n            return r > 0 ? binom_inv(n + r - 1, r) : U(1);\n \
    \       }\n        template <typename ...Ds, std::enable_if_t<std::conjunction_v<std::is_integral<Ds>...>,\
    \ std::nullptr_t> = nullptr>\n        U polynom(const int n, const Ds& ...ds)\
    \ {\n            if (n < 0) return 0;\n            ensure(n);\n            int\
    \ sumd = 0;\n            U res = _fac[n];\n            for (int d : { ds... })\
    \ {\n                if (d < 0 or d > n) return 0;\n                sumd += d;\n\
    \                res *= _fac_inv[d];\n            }\n            if (sumd > n)\
    \ return 0;\n            res *= _fac_inv[n - sumd];\n            return res;\n\
    \        }\n        U perm(const int n, const int r) {\n            if (n < 0\
    \ or r < 0 or n < r) return 0;\n            ensure(n);\n            return _fac[n]\
    \ * _fac_inv[n - r];\n        }\n        // perm(n, r) \u306E\u9006\u6570\n  \
    \      // perm(n, r) = 0 \u306E\u5834\u5408\u306F assert \u9055\u53CD\u3068\u306A\
    \u308B\n        U perm_inv(const int n, const int r) {\n            assert(r >=\
    \ 0 and n >= r);\n            ensure(n);\n            return _fac_inv[n] * _fac[n\
    \ - r];\n        }\n    private:\n        static std::vector<T> _fac;\n      \
    \  static std::vector<U> _fac_inv;\n    };\n    template <typename T, typename\
    \ U>\n    std::vector<T> factorial<T, U>::_fac{ 1 };\n    template <typename T,\
    \ typename U>\n    std::vector<U> factorial<T, U>::_fac_inv{ 1 };\n} // namespace\
    \ suisen\n\n\n#line 5 \"library/sequence/bernoulli_number.hpp\"\n\nnamespace suisen\
    \ {\n    template <typename FPSType>\n    std::vector<typename FPSType::value_type>\
    \ bernoulli_number(int n) {\n        using mint = typename FPSType::value_type;\n\
    \        factorial<mint> fac(n);\n        FPSType a(n + 1);\n        for (int\
    \ i = 0; i <= n; ++i) a[i] = fac.fac_inv(i + 1);\n        a.inv_inplace(n + 1),\
    \ a.resize(n + 1);\n        for (int i = 2; i <= n; ++i) a[i] *= fac(i);\n   \
    \     return a;\n    }\n} // namespace suisen\n\n\n#line 6 \"library/sequence/sum_of_powers.hpp\"\
    \n\nnamespace suisen {\n    // res[p] = 1^p + 2^p + ... + n^p for p=0, ..., max_exponent\
    \ (O(k log k), where k=max_exponent)\n    template <typename FPSType>\n    auto\
    \ sum_of_powers(int n, int max_exponent, const std::vector<typename FPSType::value_type>\
    \ &bernoulli_table) {\n        const int k = max_exponent;\n        assert(bernoulli_table.size()\
    \ >= size_t(k + 2));\n        using fps = FPSType;\n        using mint = typename\
    \ FPSType::value_type;\n\n        factorial<mint> fac(k + 1);\n        pow_mods<mint>\
    \ pow_n(n, k + 1);\n\n        fps f(k + 2);\n        for (int j = 0; j <= k +\
    \ 1; ++j) {\n            f[j] = pow_n[j] * fac.fac_inv(j);\n        }\n      \
    \  std::vector<mint> b(bernoulli_table.begin(), bernoulli_table.begin() + (k +\
    \ 2));\n        b[1] *= -1; // b[1] = +1/2\n        for (int j = 0; j <= k + 1;\
    \ ++j) {\n            b[j] *= fac.fac_inv(j);\n        }\n        f *= b;\n\n\
    \        std::vector<mint> res(k + 1);\n        for (int p = 0; p <= k; ++p) {\n\
    \            res[p] = fac.fac(p) * (f[p + 1] - b[p + 1]);\n        }\n       \
    \ return res;\n    }\n\n    // res[p] = 1^p + 2^p + ... + n^p for p=0, ..., max_exponent\
    \ (O(k log k), where k=max_exponent)\n    template <typename FPSType>\n    auto\
    \ sum_of_powers(int n, int max_exponent) {\n        return sum_of_powers<FPSType>(n,\
    \ max_exponent, bernoulli_number<FPSType>(max_exponent + 1));\n    }\n} // namespace\
    \ suisen\n\n\n"
  code: "#ifndef SUISEN_SUM_POWERS\n#define SUISEN_SUM_POWERS\n\n#include \"library/math/pow_mods.hpp\"\
    \n#include \"library/sequence/bernoulli_number.hpp\"\n\nnamespace suisen {\n \
    \   // res[p] = 1^p + 2^p + ... + n^p for p=0, ..., max_exponent (O(k log k),\
    \ where k=max_exponent)\n    template <typename FPSType>\n    auto sum_of_powers(int\
    \ n, int max_exponent, const std::vector<typename FPSType::value_type> &bernoulli_table)\
    \ {\n        const int k = max_exponent;\n        assert(bernoulli_table.size()\
    \ >= size_t(k + 2));\n        using fps = FPSType;\n        using mint = typename\
    \ FPSType::value_type;\n\n        factorial<mint> fac(k + 1);\n        pow_mods<mint>\
    \ pow_n(n, k + 1);\n\n        fps f(k + 2);\n        for (int j = 0; j <= k +\
    \ 1; ++j) {\n            f[j] = pow_n[j] * fac.fac_inv(j);\n        }\n      \
    \  std::vector<mint> b(bernoulli_table.begin(), bernoulli_table.begin() + (k +\
    \ 2));\n        b[1] *= -1; // b[1] = +1/2\n        for (int j = 0; j <= k + 1;\
    \ ++j) {\n            b[j] *= fac.fac_inv(j);\n        }\n        f *= b;\n\n\
    \        std::vector<mint> res(k + 1);\n        for (int p = 0; p <= k; ++p) {\n\
    \            res[p] = fac.fac(p) * (f[p + 1] - b[p + 1]);\n        }\n       \
    \ return res;\n    }\n\n    // res[p] = 1^p + 2^p + ... + n^p for p=0, ..., max_exponent\
    \ (O(k log k), where k=max_exponent)\n    template <typename FPSType>\n    auto\
    \ sum_of_powers(int n, int max_exponent) {\n        return sum_of_powers<FPSType>(n,\
    \ max_exponent, bernoulli_number<FPSType>(max_exponent + 1));\n    }\n} // namespace\
    \ suisen\n\n#endif // SUISEN_SUM_POWERS\n"
  dependsOn:
  - library/math/pow_mods.hpp
  - library/sequence/bernoulli_number.hpp
  - library/math/factorial.hpp
  isVerificationFile: false
  path: library/sequence/sum_of_powers.hpp
  requiredBy: []
  timestamp: '2026-06-28 22:26:07+09:00'
  verificationStatus: LIBRARY_NO_TESTS
  verifiedWith: []
documentation_of: library/sequence/sum_of_powers.hpp
layout: document
redirect_from:
- /library/library/sequence/sum_of_powers.hpp
- /library/library/sequence/sum_of_powers.hpp.html
title: library/sequence/sum_of_powers.hpp
---
