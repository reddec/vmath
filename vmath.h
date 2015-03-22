#ifndef VMATH
#define VMATH
#include <vector>
#include <stdexcept>
#include <algorithm>
namespace vec {
struct DefaultOperators {
  struct Sum {
    template <class T>
    inline T operator()(const T &a, const T &b) const noexcept {
      return a + b;
    }
  };

  struct Mul {
    template <class T>
    inline T operator()(const T &a, const T &b) const noexcept {
      return a * b;
    }
  };

  struct Div {
    template <class T>
    inline T operator()(const T &a, const T &b) const noexcept {
      return a / b;
    }
  };
};

template <class FuncType>
struct Calc : public FuncType {
  template <class T>
  inline T operator()(const T &a, const T &b) const noexcept {
    return FuncType::operator()(a, b);
  }

  template <class T>
  inline void operator()(std::vector<T> &result, const std::vector<T> &a,
                         const std::vector<T> &b) const {
    if (a.size() != b.size())
      throw std::invalid_argument("vectors must have same size");
    if (result.size() < a.size()) result.resize(a.size());
#pragma omp parallel for
    for (size_t i = 0; i < a.size(); ++i)
      result[i] = FuncType::operator()(a[i], b[i]);
  }

  template <class T, class... Vectors>
  inline void operator()(std::vector<T> &result, const std::vector<T> &a,
                         const std::vector<T> &b,
                         const Vectors &... vectors) const {
    operator()(result, a, b);
    operator()(result, result, vectors...);
  }

  template <class T>
  inline void operator()(std::vector<T> &result, const std::vector<T> &a,
                         const T &b) const {
#pragma omp parallel for
    for (size_t i = 0; i < a.size(); ++i)
      result[i] = FuncType::operator()(a[i], b);
  }
};

const static Calc<DefaultOperators::Sum> sum;
const static Calc<DefaultOperators::Mul> mul;
const static Calc<DefaultOperators::Div> div;

template <class T>
static inline T accumulate(const std::vector<T> &v, const T &init = T()) {
  T res = init;
#pragma omp parallel for reduction(+ : res)
  for (size_t i = 0; i < v.size(); ++i) res += v[i];
  return res;
}
}
#endif  // VMATH
