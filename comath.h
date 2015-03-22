#ifndef COMATH
#define COMATH
#include "vmath.h"
#include <vector>
#include <iostream>
namespace vec {

template <class VectorType, class RealType = double>
static inline RealType strike_slip(const VectorType &a, const size_t dt) {
  RealType accum = RealType();
#pragma omp parallel for reduction(+ : accum)
  for (size_t i = 0; i < a.size() - dt; ++i) {
    accum += fabs(a[i] - a[i + dt]);
  }
  return accum / (RealType)(a.size() - dt);
}

template <class VectorType>
static inline VectorType alter_johnson(const VectorType &a, double part = 0.6) {
  VectorType result(ceil(a.size() * part) - 1);
#pragma omp parallel for
  for (size_t i = 1; i <= result.size(); ++i) {
    result[i - 1] = strike_slip(a, i);
  }
  return result;
}

template <class T, class VectorType>
static inline void print(const VectorType &v, std::ostream &out = std::cerr) {
  for (auto &vi : v) out << vi << ' ';
  out << std::endl;
}
}
#endif  // COMATH
