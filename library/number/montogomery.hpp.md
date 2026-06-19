---
data:
  _extendedDependsOn:
  - icon: ':heavy_check_mark:'
    path: library/number/montgomery.hpp
    title: Montgomery
  _extendedRequiredBy: []
  _extendedVerifiedWith: []
  _isVerificationFailed: false
  _pathExtension: hpp
  _verificationStatusIcon: ':warning:'
  attributes:
    links: []
  bundledCode: "#line 1 \"library/number/montogomery.hpp\"\n\n\n\n#line 1 \"library/number/montgomery.hpp\"\
    \n\n\n\n#include <cassert>\n#include <cstdint>\n#include <limits>\n\nnamespace\
    \ suisen {\n    namespace internal::montgomery {\n        template <typename Int,\
    \ typename DInt>\n        struct Montgomery {\n        private:\n            static\
    \ constexpr uint32_t bits = std::numeric_limits<Int>::digits;\n            static\
    \ constexpr Int mask = ~Int(0);\n            // R = 2**32 or 2**64\n\n       \
    \     // 1. N is an odd number\n            // 2. N < R\n            // 3. gcd(N,\
    \ R) = 1\n            // 4. R * R2 - N * N2 = 1\n            // 5. 0 < R2 < N\n\
    \            // 6. 0 < N2 < R\n            Int N, N2, R2;\n\n            // RR\
    \ = R * R (mod N)\n            Int RR;\n        public:\n            constexpr\
    \ Montgomery() = default;\n            explicit constexpr Montgomery(Int N) :\
    \ N(N), N2(calcN2(N)), R2(calcR2(N, N2)), RR(calcRR(N)) {\n                assert(N\
    \ & 1);\n            }\n\n            // @returns t * R (mod N)\n            constexpr\
    \ Int make(Int t) const {\n                return reduce(static_cast<DInt>(t)\
    \ * RR);\n            }\n            // @returns T * R^(-1) (mod N)\n        \
    \    constexpr Int reduce(DInt T) const {\n                // 0 <= T < RN\n\n\
    \                // Note:\n                //  1. m = T * N2 (mod R)\n       \
    \         //  2. 0 <= m < R\n                DInt m = modR(static_cast<DInt>(modR(T))\
    \ * N2);\n\n                // Note:\n                //  T + m * N = T + T *\
    \ N * N2 = T + T * (R * R2 - 1) = 0 (mod R)\n                //  => (T + m * N)\
    \ / R is an integer.\n                //  => t * R = T + m * N = T (mod N)\n \
    \               //  => t = T R^(-1) (mod N)\n                DInt t = divR(T +\
    \ m * N);\n\n                // Note:\n                //  1. 0 <= T < RN\n  \
    \              //  2. 0 <= mN < RN (because 0 <= m < R)\n                //  =>\
    \ 0 <= T + mN < 2RN\n                //  => 0 <= t < 2N\n                return\
    \ t >= N ? t - N : t;\n            }\n\n            constexpr Int add(Int A, Int\
    \ B) const {\n                return (A += B) >= N ? A - N : A;\n            }\n\
    \            constexpr Int sub(Int A, Int B) const {\n                return (A\
    \ -= B) < 0 ? A + N : A;\n            }\n            constexpr Int mul(Int A,\
    \ Int B) const {\n                return reduce(static_cast<DInt>(A) * B);\n \
    \           }\n            constexpr Int div(Int A, Int B) const {\n         \
    \       return reduce(static_cast<DInt>(A) * inv(B));\n            }\n       \
    \     constexpr Int inv(Int A) const; // TODO: Implement\n\n            constexpr\
    \ Int pow(Int A, long long b) const {\n                Int P = make(1);\n    \
    \            for (; b; b >>= 1) {\n                    if (b & 1) P = mul(P, A);\n\
    \                    A = mul(A, A);\n                }\n                return\
    \ P;\n            }\n\n        private:\n            static constexpr Int divR(DInt\
    \ t) { return t >> bits; }\n            static constexpr Int modR(DInt t) { return\
    \ t & mask; }\n\n            static constexpr Int calcN2(Int N) {\n          \
    \      // - N * N2 = 1 (mod R)\n                // N2 = -N^{-1} (mod R)\n\n  \
    \              // calculates N^{-1} (mod R) by Newton's method\n             \
    \   DInt invN = N; // = N^{-1} (mod 2^2)\n                for (uint32_t cur_bits\
    \ = 2; cur_bits < bits; cur_bits *= 2) {\n                    // loop invariant:\
    \ invN = N^{-1} (mod 2^cur_bits)\n\n                    // x = a^{-1} mod m =>\
    \ x(2-ax) = a^{-1} mod m^2 because:\n                    //  ax = 1 (mod m)\n\
    \                    //  => (ax-1)^2 = 0 (mod m^2)\n                    //  =>\
    \ 2ax - a^2x^2 = 1 (mod m^2)\n                    //  => a(x(2-ax)) = 1 (mod m^2)\n\
    \                    invN = modR(invN * modR(2 - N * invN));\n               \
    \ }\n                assert(modR(N * invN) == 1);\n\n                return modR(-invN);\n\
    \            }\n            static constexpr Int calcR2(Int N, Int N2) {\n   \
    \             // R * R2 - N * N2 = 1\n                // => R2 = (1 + N * N2)\
    \ / R\n                return divR(1 + static_cast<DInt>(N) * N2);\n         \
    \   }\n            static constexpr Int calcRR(Int N) {\n                return\
    \ -DInt(N) % N;\n            }\n        };\n    } // namespace internal::montgomery\n\
    \    using Montgomery32 = internal::montgomery::Montgomery<uint32_t, uint64_t>;\n\
    \    using Montgomery64 = internal::montgomery::Montgomery<uint64_t, __uint128_t>;\n\
    } // namespace suisen\n\n\n\n#line 5 \"library/number/montogomery.hpp\"\n\n\n"
  code: '#ifndef SUISEN_MONTOGOMERY

    #define SUISEN_MONTOGOMERY


    #include "library/number/montgomery.hpp"


    #endif // SUISEN_MONTOGOMERY

    '
  dependsOn:
  - library/number/montgomery.hpp
  isVerificationFile: false
  path: library/number/montogomery.hpp
  requiredBy: []
  timestamp: '2026-06-19 20:35:33+09:00'
  verificationStatus: LIBRARY_NO_TESTS
  verifiedWith: []
documentation_of: library/number/montogomery.hpp
layout: document
redirect_from:
- /library/library/number/montogomery.hpp
- /library/library/number/montogomery.hpp.html
title: library/number/montogomery.hpp
---
