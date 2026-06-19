#ifndef SUISEN_POLY_COMPOSE
#define SUISEN_POLY_COMPOSE

#include <cmath>
#include <vector>
#include <atcoder/convolution>

namespace suisen {
    template <typename mint>
    std::vector<mint> compose(const std::vector<mint>& f, std::vector<mint> g, const int n) {
        std::vector<mint> res(n);
        if (n == 0) return res;
        if (f.empty()) return res;

        if (std::find_if(g.begin(), g.end(), [](mint x) { return x != 0; }) == g.end()) return res[0] = f[0], res;

        // taylor shift f(x + [x^0]g)
        const std::vector<mint> fa = [&]{
            const mint a = std::exchange(g[0], 0);
            const int size_f = f.size();
            
            std::vector<mint> fac(size_f), fac_inv(size_f);
            fac[0] = 1;
            for (int i = 1; i <= size_f - 1; ++i) fac[i] = fac[i - 1] * i;
            fac_inv[size_f - 1] = fac[size_f - 1].inv();
            for (int i = size_f - 1; i >= 1; --i) fac_inv[i - 1] = fac_inv[i] * i;

            std::vector<mint> ec(size_f), fa(size_f);
            mint p = 1;
            for (int i = 0; i < size_f; ++i, p *= a) {
                ec[i] = p * fac_inv[i];
                fa[size_f - 1 - i] = (i < int(f.size()) ? f[i] : 0) * fac[i];
            }
            fa = atcoder::convolution(fa, ec), fa.resize(size_f);
            std::reverse(fa.begin(), fa.end());
            for (int i = 0; i < size_f; ++i) {
                fa[i] *= fac_inv[i];
            }
            if (size_f > n) fa.resize(n);
            return fa;
        }();

        const int sqn = ::sqrt(f.size()) + 1;

        const int z = [n]{
            int z = 1;
            while (z < 2 * n - 1) z <<= 1;
            return z;
        }();
        const mint inv_z = mint(z).inv();

        g.erase(g.begin());
        g.resize(z);
        atcoder::internal::butterfly(g);

        auto mult_g = [&](std::vector<mint> a) {
            a.resize(z);
            atcoder::internal::butterfly(a);
            for (int j = 0; j < z; ++j) a[j] *= g[j] * inv_z;
            atcoder::internal::butterfly_inv(a);
            a.resize(n);
            return a;
        };

        std::vector<std::vector<mint>> pow_g(sqn, std::vector<mint>(n));
        pow_g[0][0] = 1;
        for (int i = 1; i < sqn; ++i) {
            pow_g[i] = mult_g(pow_g[i - 1]);
        }

        std::vector<mint> gl = mult_g(pow_g[sqn - 1]);
        gl.resize(z);
        atcoder::internal::butterfly(gl);

        std::vector<mint> pow_gl(z);
        pow_gl[0] = 1;

        for (int i = 0; i < sqn; ++i) {
            const int off_i = i * sqn;
            const int size_i = n - off_i;
            if (size_i < 0) break;
            std::vector<mint> fg(size_i);
            for (int j = 0; j < sqn; ++j) {
                const int ij = i * sqn + j;
                if (ij >= int(fa.size())) break;

                const mint c = fa[ij];
                const std::vector<mint>& gj = pow_g[j];
                for (int k = 0; k < size_i - j; ++k) {
                    fg[j + k] += c * gj[k];
                }
            }
            fg.resize(z);
            atcoder::internal::butterfly(pow_gl);
            atcoder::internal::butterfly(fg);
            for (int k = 0; k < z; ++k) {
                fg[k] *= pow_gl[k] * inv_z;
                pow_gl[k] *= gl[k] * inv_z;
            }
            atcoder::internal::butterfly_inv(pow_gl);
            atcoder::internal::butterfly_inv(fg);
            for (int k = 0; k < size_i; ++k) {
                res[off_i + k] += fg[k];
            }
            std::fill(pow_gl.begin() + n, pow_gl.end(), 0);
        }
        return res;
    }
} // namespace suisen


#endif // SUISEN_POLY_COMPOSE
