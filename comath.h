#ifndef COMATH
#define COMATH
#include "vmath.h"
#include <vector>
#include <iostream>
namespace vec {

template <class T, class RealType = T>
static inline RealType strike_slip(const std::vector<T> &a, const size_t dt) {
  T accum = T();
#pragma omp parallel for reduction(+ : accum)
  for (size_t i = 0; i < a.size() - dt; ++i) {
    accum += fabs(a[i] - a[i + dt]);
  }
  return (RealType)accum / (a.size() - dt);
}

template <class T>
static inline std::vector<T> alter_johnson(const std::vector<T> &a,
                                           double part = 0.6) {
  std::vector<T> result(ceil(a.size() * part) - 1);
#pragma omp parallel for
  for (size_t i = 1; i <= result.size(); ++i) {
    result[i - 1] = strike_slip(a, i);
  }
  return result;
}

template <class T>
static inline void print(const std::vector<T> &v,
                         std::ostream &out = std::cerr) {
  for (auto &vi : v) out << vi << ' ';
  out << std::endl;
}
}
#endif  // COMATH
