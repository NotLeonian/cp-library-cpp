#ifndef SUISEN_REROOTING
#define SUISEN_REROOTING

#include <cassert>
#include <tuple>
#include <vector>
#include <variant>

namespace suisen {
    namespace internal::rerooting {
        using void_weight = std::monostate;

        template <typename VertexWeight, typename EdgeWeight>
        struct Rerooting {
            using vertex_weight = VertexWeight;
            using edge_weight = EdgeWeight;
    private:
            using is_vertex_weight_void = std::is_same<vertex_weight, void_weight>;
            using is_edge_weight_void = std::is_same<edge_weight, void_weight>;
            static constexpr bool is_vertex_weight_void_v = is_vertex_weight_void::value;
            static constexpr bool is_edge_weight_void_v = is_edge_weight_void::value;

            template <typename DP, typename AddSubtreeRoot>
            using is_add_subtree_root = std::conditional_t<
                std::negation_v<is_vertex_weight_void>,
                std::conditional_t<
                    std::negation_v<is_edge_weight_void>,
                    std::is_invocable_r<DP, AddSubtreeRoot, DP, vertex_weight, edge_weight>,
                    std::is_invocable_r<DP, AddSubtreeRoot, DP, vertex_weight>
                >,
                std::conditional_t<
                    std::negation_v<is_edge_weight_void>,
                    std::is_invocable_r<DP, AddSubtreeRoot, DP, edge_weight>,
                    std::is_invocable_r<DP, AddSubtreeRoot, DP>
                >
            >;
            template <typename DP, typename AddRoot>
            using is_add_root = std::conditional_t<
                std::negation_v<is_vertex_weight_void>,
                std::is_invocable_r<DP, AddRoot, DP, vertex_weight>,
                std::is_invocable_r<DP, AddRoot, DP>
            >;
    public:
            Rerooting() : _w{} {}
            explicit Rerooting(int n) : _w(n) {}
            explicit Rerooting(const std::vector<vertex_weight>& w) : _w(w) {}

            void reserve(int n) {
                _w.reserve(n);
            }

            void add_vertex(const vertex_weight& w) {
                _w.emplace_back(w);
            }
            void add_edge(int u, int v, const edge_weight& w = {}) {
                const int n = _w.size();
                assert(0 <= u and u < n);
                assert(0 <= v and v < n);
                _e.emplace_back(u, v, w);
            }
            void set_vertex_weights(const std::vector<vertex_weight>& w) {
                assert(w.size() == _w.size());
                _w = w;
            }

            /**
             * op               : (T, T) -> T                               // commutative monoid
             * e                : () -> T                                   // identity
             * add_subtree_root : (T, vertex_weight, edge_weight) -> T        // add subroot, edge to parent
             * add_root         : (T, vertex_weight) -> T                    // add root
            */
            template <typename Op, typename E, typename AddSubtreeRoot, typename AddRoot,
                typename DP = std::decay_t<std::invoke_result_t<E>>,
                std::enable_if_t<std::conjunction_v<
                    std::is_invocable_r<DP, Op, DP, DP>,
                    std::is_invocable_r<DP, E>,
                    is_add_subtree_root<DP, AddSubtreeRoot>,
                    is_add_root<DP, AddRoot>
                >, std::nullptr_t> = nullptr
            >
            std::vector<DP> run_dp(const Op& op, const E& e, const AddSubtreeRoot& add_subtree_root, const AddRoot& add_root) const {
                auto add_subtree_root_ = [&add_subtree_root](const DP &val, const vertex_weight& vw, const edge_weight& ew) {
                    if constexpr (std::negation_v<is_vertex_weight_void>) {
                        if constexpr (std::negation_v<is_edge_weight_void>) {
                            return add_subtree_root(val, vw, ew);
                        } else {
                            return add_subtree_root(val, vw);
                        }
                    } else {
                        if constexpr (std::negation_v<is_edge_weight_void>) {
                            return add_subtree_root(val, ew);
                        } else {
                            return add_subtree_root(val);
                        }
                    }
                };
                auto add_root_ = [&add_root](const DP &val, const vertex_weight& vw) {
                    if constexpr (std::negation_v<is_vertex_weight_void>) {
                        return add_root(val, vw);
                    } else {
                        return add_root(val);
                    }
                };

                const int n = _w.size();

                GraphCSR g(n, _e);

                std::vector<DP> dp(n, e());

                [dfs = [&, this](auto dfs, int u, int p) -> void {
                    for (const auto& [v, w] : g[u]) if (v != p) {
                        dfs(dfs, v, u);
                        dp[u] = op(dp[u], add_subtree_root_(dp[v], _w[v], w));
                    }
                }] { dfs(dfs, 0, -1); }();
                dp[0] = add_root_(dp[0], _w[0]);

                [dfs = [&, this](auto dfs, int u, int p, const DP& sum_p) -> void {
                    auto get_sum = [&](int v) {
                        return v == p ? sum_p : dp[v];
                    };

                    const int size = g[u].size();
                    std::vector<DP> sum_r(size + 1, e());
                    for (int i = size - 1; i >= 0; --i) {
                        const auto& [v, w] = g[u][i];
                        sum_r[i] = op(sum_r[i + 1], add_subtree_root_(get_sum(v), _w[v], w));
                    }

                    DP sum_l = e();
                    for (int i = 0; i < size; ++i) {
                        const auto& [v, w] = g[u][i];
                        DP nxt_sum_l = op(sum_l, add_subtree_root_(get_sum(v), _w[v], w));
                        if (v != p) {
                            DP sum_lr = op(sum_l, sum_r[i + 1]);
                            DP sum_v = op(dp[v], add_subtree_root_(sum_lr, _w[u], w));
                            dp[v] = add_root_(sum_v, _w[v]);
                            dfs(dfs, v, u, sum_lr);
                        }
                        sum_l = std::move(nxt_sum_l);
                    }
                }, &e] { dfs(dfs, 0, -1, e()); }();

                return dp;
            }

        private:
            std::vector<vertex_weight> _w;
            std::vector<std::tuple<int, int, edge_weight>> _e;

            struct GraphCSR {
                GraphCSR(int n, const std::vector<std::tuple<int, int, edge_weight>>& edges) : _n(n), _m(edges.size()), _edges(2 * _m), _start(_n + 1) {
                    for (const auto& [u, v, w] : edges) {
                        ++_start[u];
                        ++_start[v];
                    }
                    for (int i = 1; i <= _n; ++i) {
                        _start[i] += _start[i - 1];
                    }
                    for (const auto& [u, v, w] : edges) {
                        _edges[--_start[u]] = { v, w };
                        _edges[--_start[v]] = { u, w };
                    }
                }
            private:
                using edge_type = std::pair<int, edge_weight>;
                using iterator = typename std::vector<edge_type>::const_iterator;

                struct AdjacentListView {
                    AdjacentListView(const iterator& l, const iterator& r) : _l(l), _r(r) {}

                    int size() const { return _r - _l; }

                    const edge_type& operator[](int i) const { return *(_l + i); }

                    iterator begin() const { return _l; }
                    iterator end() const { return _r; }
                private:
                    iterator _l, _r;
                };
            public:
                AdjacentListView operator[](int u) const {
                    return AdjacentListView(_edges.begin() + _start[u], _edges.begin() + _start[u + 1]);
                }
            private:
                int _n, _m;
                std::vector<std::pair<int, edge_weight>> _edges;
                std::vector<int> _start;
            };
        };
    }
    using Rerooting = internal::rerooting::Rerooting<internal::rerooting::void_weight, internal::rerooting::void_weight>;
    template <typename VertexWeight>
    using RerootingVertexWeighted = internal::rerooting::Rerooting<VertexWeight, internal::rerooting::void_weight>;
    template <typename EdgeWeight>
    using RerootingEdgeWeighted = internal::rerooting::Rerooting<internal::rerooting::void_weight, EdgeWeight>;
    template <typename VertexWeight, typename EdgeWeighted>
    using RerootingWeighted = internal::rerooting::Rerooting<VertexWeight, EdgeWeighted>;
} // namsepace suisen

#endif // SUISEN_REROOTING
