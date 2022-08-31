#pragma once

#include <vector>
#include <numeric>


namespace joanaplewni {
    template<typename T>

    class FastUnionFind {

        std::vector<T> parent;
        std::vector<T> size;
    public:

        FastUnionFind(const T &num_elements) {
            parent.resize(num_elements);
            std::iota(parent.begin(), parent.end(), 0);
            size.resize(num_elements);
            std::iota(size.begin(), size.end(), 1);
        }

        T find(T el) {
            if (el == parent[el]) {
                return el;
            } else {
                return parent[el] = find(parent[el]);;
            }
        }

        void do_union(const T &el1, const T &el2) {
            assert(find(el1) != find(el2));
            T a = find(el1);
            T b = find(el2);
            if (size.at(a) < size.at(b)) {
                std::swap(a, b);
            }
            parent[b] = a;
            size[a] += size[b];
        }


    };

}
