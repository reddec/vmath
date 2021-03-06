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

  struct Rem {
    template <class T>
    inline T operator()(const T &a, const T &b) const noexcept {
      return a - b;
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
  /**
 * @brief operator () Do single binary operation
 * @param a Left value
 * @param b Right value
 * @return Result of binary operation
 */
  template <class T>
  inline auto operator()(const T &a, const T &b) const
      noexcept -> decltype(FuncType::operator()(a, b)) {
    return FuncType::operator()(a, b);
  }

  /**
   * @brief operator () Binary operation for each element in vectors.
   * @param result
   * @param a Left argument
   * @param b Right argument
   * @return True if computation done. Otherwise (vectors have not same size)
   * False
   */
  template <class VectorType>
  inline bool operator()(VectorType &result, const VectorType &a,
                         const VectorType &b) const noexcept {
    if (a.size() != b.size()) return false;
    if (result.size() < a.size()) result.resize(a.size());
#pragma omp parallel for
    for (size_t i = 0; i < a.size(); ++i)
      result[i] = FuncType::operator()(a[i], b[i]);
    return true;
  }

  /**
   * @brief operator () Binary operation for each element for each vector
   * @param result Result container
   * @param a Left argument
   * @param b Right argument
   * @param vectors Next arguments
   * @return True if computation done. Otherwise (vectors have not same size)
   * False
   */
  template <class VectorType, class... Vectors>
  inline bool operator()(VectorType &result, const VectorType &a,
                         const VectorType &b, const Vectors &... vectors) const
      noexcept {
    return operator()(result, a, b) &&operator()(result, result, vectors...);
  }

  template <class VectorType, class T>
  /**
   * @brief operator () Binary opertation for each element with constant
   * @param result Result container
   * @param a Left argument
   * @param b Right argument
   */
  inline void operator()(VectorType &result, const VectorType &a,
                         const T &b) const noexcept {
#pragma omp parallel for
    for (size_t i = 0; i < a.size(); ++i)
      result[i] = FuncType::operator()(a[i], b);
  }
};

const static Calc<DefaultOperators::Sum> sum;
const static Calc<DefaultOperators::Rem> rem;
const static Calc<DefaultOperators::Mul> mul;
const static Calc<DefaultOperators::Div> div;

template <class T, class VectorType>
static inline T accumulate(T &result, const VectorType &v,
                           const T &init = T()) noexcept {
  T res = init;
#pragma omp parallel for reduction(+ : res)
  for (size_t i = 0; i < v.size(); ++i) res += v[i];
  result = res;
  return result;
}
}
#endif  // VMATH
