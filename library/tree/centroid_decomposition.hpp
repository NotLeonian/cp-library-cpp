#ifndef SUISEN_CENTROID_DECOMPOSITION
#define SUISEN_CENTROID_DECOMPOSITION

#include <deque>
#include <limits>
#include <tuple>
#include <vector>
#include "library/graph/csr_graph.hpp"

namespace suisen {
    namespace internal {
        template <typename WeightType = void>
        struct CentroidDecomposition : Graph<WeightType> {
            friend struct CentroidDecompositionUnweighted;
            template <typename WeightType_, std::enable_if_t<not std::is_same_v<WeightType_, void>, std::nullptr_t>>
            friend struct CentroidDecompositionWeighted;

            using graph_type = Graph<WeightType>;
            using weight_type = WeightType;

            CentroidDecomposition(const graph_type& g) : graph_type(g), n(this->size()), cpar(n, -1), cdep(n, std::numeric_limits<int>::max()), component_size(n) {
                build();
            }

            int dct_parent(int i) const { return cpar[i]; }
            int dct_depth(int i) const { return cdep[i]; }
            int dct_size(int i) const { return component_size[i]; }

        private:
            int n;
            std::vector<int> cpar;
            std::vector<int> cdep;
            std::vector<int> component_size;

            void build() {
                std::vector<int> eid(n, 0);

                cpar[0] = -1, component_size[0] = n;
                std::deque<std::tuple<int, int>> dq{ { 0, 0 } };

                while (dq.size()) {
                    const auto [r, dep] = dq.front();
                    const int size = component_size[r], prev_ctr = cpar[r];
                    dq.pop_front();

                    int c = -1;
                    eid[r] = 0, component_size[r] = 1, cpar[r] = -1;
                    for (int cur = r;;) {
                        for (const int edge_num = int((*this)[cur].size());;) {
                            if (eid[cur] == edge_num) {
                                if (component_size[cur] * 2 > size) {
                                    c = cur;
                                } else {
                                    const int nxt = cpar[cur];
                                    component_size[nxt] += component_size[cur];
                                    cur = nxt;
                                }
                                break;
                            }
                            const int nxt = (*this)[cur][eid[cur]++];
                            if (cdep[nxt] >= dep and nxt != cpar[cur]) {
                                eid[nxt] = 0, component_size[nxt] = 1, cpar[nxt] = cur;
                                cur = nxt;
                                break;
                            }
                        }
                        if (c >= 0) break;
                    }
                    for (int v : (*this)[c]) if (cdep[v] >= dep) {
                        if (cpar[c] == v) cpar[v] = c, component_size[v] = size - component_size[c];
                        dq.emplace_back(v, dep + 1);
                    }
                    cpar[c] = prev_ctr, cdep[c] = dep, component_size[c] = size;
                }
            }
        };

        struct CentroidDecompositionUnweighted : internal::CentroidDecomposition<void> {
            using base_type = internal::CentroidDecomposition<void>;
            using base_type::base_type;

            std::vector<std::vector<std::pair<int, int>>> collect(int root, int root_val = 0) const {
                std::vector<std::vector<std::pair<int, int>>> res{ { { root, root_val } } };
                for (int sub_root : (*this)[root]) if (this->cdep[sub_root] > this->cdep[root]) {
                    res.emplace_back();
                    std::deque<std::tuple<int, int, int>> dq{ { sub_root, root, root_val + 1 } };
                    while (dq.size()) {
                        auto [u, p, w] = dq.front();
                        dq.pop_front();
                        res.back().emplace_back(u, w);
                        for (int v : (*this)[u]) if (v != p and this->cdep[v] > this->cdep[root]) {
                            dq.emplace_back(v, u, w + 1);
                        }
                    }
                    std::copy(res.back().begin(), res.back().end(), std::back_inserter(res.front()));
                }
                return res;
            }
        };

        template <typename WeightType, std::enable_if_t<not std::is_same_v<WeightType, void>, std::nullptr_t> = nullptr>
        struct CentroidDecompositionWeighted : internal::CentroidDecomposition<WeightType> {
            using base_type = internal::CentroidDecomposition<WeightType>;
            using base_type::base_type;

            using weight_type = typename base_type::weight_type;

            template <typename Op, std::enable_if_t<std::is_invocable_r_v<weight_type, Op, weight_type, weight_type>, std::nullptr_t> = nullptr>
            std::vector<std::vector<std::pair<int, weight_type>>> collect(int root, Op op, weight_type root_val) const {
                std::vector<std::vector<std::pair<int, weight_type>>> res{ { { root, root_val } } };
                for (auto [sub_root, ew] : (*this)[root]) if (this->cdep[sub_root] > this->cdep[root]) {
                    res.emplace_back();
                    std::deque<std::tuple<int, int, weight_type>> dq{ { sub_root, root, op(root_val, ew) } };
                    while (dq.size()) {
                        auto [u, p, w] = dq.front();
                        dq.pop_front();
                        res.back().emplace_back(u, w);
                        for (auto [v, ew] : (*this)[u]) if (v != p and this->cdep[v] > this->cdep[root]) {
                            dq.emplace_back(v, u, op(w, ew));
                        }
                    }
                    std::copy(res.back().begin(), res.back().end(), std::back_inserter(res.front()));
                }
                return res;
            }
        };
    }

    using CentroidDecompositionUnweighted = internal::CentroidDecompositionUnweighted;
    template <typename WeightType, std::enable_if_t<not std::is_same_v<WeightType, void>, std::nullptr_t> = nullptr>
    using CentroidDecompositionWeighted = internal::CentroidDecompositionWeighted<WeightType>;

} // namespace suisen

#endif // SUISEN_CENTROID_DECOMPOSITION
