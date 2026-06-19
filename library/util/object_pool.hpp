#ifndef SUISEN_OBJECT_POOL
#define SUISEN_OBJECT_POOL

#include <deque>
#include <vector>

namespace suisen {
    template <typename T, bool auto_extend = false>
    struct ObjectPool {
        using value_type = T;
        using value_pointer_type = T*;

        template <typename U>
        using container_type = std::conditional_t<auto_extend, std::deque<U>, std::vector<U>>;

        container_type<value_type> pool;
        container_type<value_pointer_type> stock;
        decltype(stock.begin()) it;

        ObjectPool() : ObjectPool(0) {}
        ObjectPool(int size) : pool(size), stock(size) {
            clear();
        }

        int capacity() const { return pool.size(); }
        int size() const { return it - stock.begin(); }

        value_pointer_type alloc() {
            if constexpr (auto_extend) ensure();
            return *it++;
        }

        void free(value_pointer_type t) {
            *--it = t;
        }

        void clear() {
            int size = pool.size();
            it = stock.begin();
            for (int i = 0; i < size; i++) stock[i] = &pool[i];
        }

        void ensure() {
            if (it != stock.end()) return;
            int size = stock.size();
            for (int i = size; i <= size * 2; ++i) {
                stock.push_back(&pool.emplace_back());
            }
            it = stock.begin() + size;
        }
    };
} // namespace suisen

#endif // SUISEN_OBJECT_POOL
