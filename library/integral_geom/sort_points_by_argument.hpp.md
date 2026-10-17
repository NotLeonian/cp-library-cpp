---
data:
  _extendedDependsOn: []
  _extendedRequiredBy: []
  _extendedVerifiedWith:
  - icon: ':heavy_check_mark:'
    path: test/src/integral_geom/count_lattice_point/yuki1999.test.cpp
    title: test/src/integral_geom/count_lattice_point/yuki1999.test.cpp
  - icon: ':heavy_check_mark:'
    path: test/src/integral_geom/sort_points_by_argument/sort_points_by_argument.test.cpp
    title: test/src/integral_geom/sort_points_by_argument/sort_points_by_argument.test.cpp
  _isVerificationFailed: false
  _pathExtension: hpp
  _verificationStatusIcon: ':heavy_check_mark:'
  attributes:
    links: []
  bundledCode: "#line 1 \"library/integral_geom/sort_points_by_argument.hpp\"\n\n\n\
    \n#include <algorithm>\n#include <vector>\n\nnamespace suisen::integral_geometry\
    \ {\n    /**\n     * 1. (x < 0, y = 0) -> pi\n     * 2. (x = 0, y = 0) -> 0\n\
    \     * 3. points with same argument -> arbitrary order\n     */\n    template\
    \ <typename PointType, typename MultipliedType = long long>\n    bool compare_by_atan2(const\
    \ PointType &p, const PointType &q) {\n        const auto &[x1, y1] = p;\n   \
    \     const auto &[x2, y2] = q;\n        if ((y1 < 0) xor (y2 < 0)) return y1\
    \ < y2;\n        if ((x1 < 0) xor (x2 < 0)) return (y1 >= 0) xor (x1 < x2);\n\
    \        if (x2 == 0 and y2 == 0) return false;\n        if (x1 == 0 and y1 ==\
    \ 0) return true;\n        return (MultipliedType(y1) * x2 < MultipliedType(y2)\
    \ * x1);\n    }\n    template <typename PointType, typename MultipliedType = long\
    \ long>\n    void sort_points_by_argument(std::vector<PointType> &points) {\n\
    \        std::sort(points.begin(), points.end(), compare_by_atan2<PointType, MultipliedType>);\n\
    \    }\n} // namespace suisen::integral_geometry\n\n\n\n"
  code: "#ifndef SUISEN_SORT_POINTS_BY_ARGUMENT\n#define SUISEN_SORT_POINTS_BY_ARGUMENT\n\
    \n#include <algorithm>\n#include <vector>\n\nnamespace suisen::integral_geometry\
    \ {\n    /**\n     * 1. (x < 0, y = 0) -> pi\n     * 2. (x = 0, y = 0) -> 0\n\
    \     * 3. points with same argument -> arbitrary order\n     */\n    template\
    \ <typename PointType, typename MultipliedType = long long>\n    bool compare_by_atan2(const\
    \ PointType &p, const PointType &q) {\n        const auto &[x1, y1] = p;\n   \
    \     const auto &[x2, y2] = q;\n        if ((y1 < 0) xor (y2 < 0)) return y1\
    \ < y2;\n        if ((x1 < 0) xor (x2 < 0)) return (y1 >= 0) xor (x1 < x2);\n\
    \        if (x2 == 0 and y2 == 0) return false;\n        if (x1 == 0 and y1 ==\
    \ 0) return true;\n        return (MultipliedType(y1) * x2 < MultipliedType(y2)\
    \ * x1);\n    }\n    template <typename PointType, typename MultipliedType = long\
    \ long>\n    void sort_points_by_argument(std::vector<PointType> &points) {\n\
    \        std::sort(points.begin(), points.end(), compare_by_atan2<PointType, MultipliedType>);\n\
    \    }\n} // namespace suisen::integral_geometry\n\n\n#endif // SUISEN_SORT_POINTS_BY_ARGUMENT\n"
  dependsOn: []
  isVerificationFile: false
  path: library/integral_geom/sort_points_by_argument.hpp
  requiredBy: []
  timestamp: '2026-06-01 17:03:55+09:00'
  verificationStatus: LIBRARY_ALL_AC
  verifiedWith:
  - test/src/integral_geom/count_lattice_point/yuki1999.test.cpp
  - test/src/integral_geom/sort_points_by_argument/sort_points_by_argument.test.cpp
documentation_of: library/integral_geom/sort_points_by_argument.hpp
layout: document
title: "\u504F\u89D2\u30BD\u30FC\u30C8 (\u6574\u6570\u5EA7\u6A19)"
---
## 偏角ソート (整数座標)
