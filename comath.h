#ifndef COMATH
#define COMATH
#ifdef _OPENMP
#pragma message("OpenMP enabled")
#endif
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>

#ifdef _OPENCL
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#pragma message("OpenCL enabled")
typedef uint64_t CLSize;
typedef uint64_t CLOffsetV1;
typedef uint64_t CLOffsetV2;
typedef uint64_t CLPadding;
typedef decltype(
    cl::make_kernel<uint64_t, CLOffsetV1, CLOffsetV2, cl::Buffer, cl::Buffer,
                    cl::Buffer>(cl::Program("", true), "")) CLBinaryFunctorType;
typedef decltype(
    cl::make_kernel<CLSize, CLPadding, cl::Buffer, cl::Buffer>(
        cl::Program("", true), "")) CLAccumFunctorType;

static std::string command_builder(const std::string &typen,
                                   bool enable_fp64 = false) {
  std::string reduction =
      "typedef unsigned long uint64_t;\n"
      "void __kernel accumulate("
      "uint64_t size,"
      "uint64_t pad,";
  reduction += "global " + typen + " *data, " + "global " + typen + " *copy) ";
  reduction +=
      " {"
      "uint64_t index = get_global_id(0) * pad;"
      "uint64_t limit = (size < index + pad ? size : index + pad);"
      "uint64_t i;";
  reduction += typen + " accum = 0;";
  reduction +=
      "if(index < size) {"
      "for(i = index; i < limit; ++i) accum += data[i];"
      "copy[index / pad] = accum;"
      "}          }\n";
  std::string sumt =
      "void __kernel sum(unsigned long s, unsigned long offsetV1, unsigned "
      "long offsetV2, ";
  sumt += "global " + typen + " *v1, global " + typen + " *v2, global " +
          typen + " *r){";
  sumt +=
      "unsigned long index = get_global_id(0);"
      "unsigned long v1i = index + offsetV1;"
      "unsigned long v2i = index + offsetV2;"
      "if(v1i < s && v2i < s && index < s) r[index] = v1[v1i] + v2[v2i];}";
  if (enable_fp64)
    reduction = "#pragma OPENCL EXTENSION cl_khr_fp64: enable\n" + reduction;
  return reduction + sumt;
}

template <class T>
struct CLProcessor {
  static cl::Program program;
  static CLBinaryFunctorType sumFunctor;
  static CLAccumFunctorType accumFunctor;

  std::vector<T> sum(std::vector<T> &a, std::vector<T> &b) {
    std::vector<T> result(a.size());
    cl::Buffer buffer_v1(begin(a), end(a), true, false);
    cl::Buffer buffer_v2(begin(b), end(b), true, false);
    cl::Buffer buffer_r(CL_MEM_WRITE_ONLY, sizeof(T) * a.size());
    sumFunctor(cl::EnqueueArgs(cl::NDRange(a.size())), a.size(), 0, 0,
               buffer_v1, buffer_v2, buffer_r);
    cl::copy(buffer_r, std::begin(result), std::end(result));
    return result;
  }

  double accum(size_t size, cl::Buffer &data, cl::Buffer &copy) {
    size_t r_size;
    size_t padding = 2;
    bool flip = false;
    double result[1];
    while (size > 1) {
      r_size = ceil(size / (double)padding);
      if (!flip) {
        accumFunctor(cl::EnqueueArgs(cl::NDRange(r_size)), size, padding, data,
                     copy);
      } else {
        accumFunctor(cl::EnqueueArgs(cl::NDRange(r_size)), size, padding, copy,
                     data);
      }

      flip = !flip;
      size = r_size;
    }
    if (flip)
      cl::copy(copy, std::begin(result), std::end(result));
    else
      cl::copy(data, std::begin(result), std::end(result));
    return result[0];
  }

  std::vector<T> alter_johnson(const std::vector<T> &v, size_t target_len) {
    std::vector<T> result(target_len);
    std::vector<T> a = v * -1;  // Invert
    std::vector<T> b = v;
    std::vector<T> sslip(a.size());
    cl::Buffer buffer_v1(begin(a), end(a), true, false);
    cl::Buffer buffer_v2(begin(b), end(b), true, false);
    cl::Buffer buffer_r(CL_MEM_WRITE_ONLY, sizeof(T) * a.size());
    cl::Buffer buffer_cr(CL_MEM_WRITE_ONLY, sizeof(T) * a.size());

    for (size_t dt = 1; dt <= target_len; ++dt) {
      sumFunctor(cl::EnqueueArgs(cl::NDRange(a.size() - dt)), a.size(), 0, dt,
                 buffer_v1, buffer_v2, buffer_r);
      cl::copy(buffer_r, std::begin(sslip), std::end(sslip));

      result[dt - 1] = accum(a.size() - dt, buffer_r, buffer_cr);
    }
    return result;
  }
};

template <>
cl::Program CLProcessor<double>::program(command_builder("double", true), true);
template <>
cl::Program CLProcessor<int>::program(command_builder("int"), true);
template <>
cl::Program CLProcessor<long>::program(command_builder("long"), true);

template <class T>
CLBinaryFunctorType CLProcessor<T>::sumFunctor =
    cl::make_kernel<uint64_t, CLOffsetV1, CLOffsetV2, cl::Buffer, cl::Buffer,
                    cl::Buffer>(CLProcessor<T>::program, "sum");
template <class T>
CLAccumFunctorType CLProcessor<T>::accumFunctor =
    cl::make_kernel<CLSize, CLPadding, cl::Buffer, cl::Buffer>(
        CLProcessor<T>::program, "accumulate");
#endif
template <class T>
inline std::vector<T> operator+(const std::vector<T> &a,
                                const std::vector<T> &b) {
  if (a.size() != b.size())
    throw std::invalid_argument("vectors must have same size");
  std::vector<T> result(a.size());
#pragma omp parallel for
  for (size_t i = 0; i < a.size(); ++i) result[i] = a[i] + b[i];

  return result;
}
#ifdef _OPENCL

inline std::vector<double> operator+(std::vector<double> &a,
                                     std::vector<double> &b) {
  if (a.size() != b.size())
    throw std::invalid_argument("vectors must have same size");
  CLProcessor<double> processor;
  return processor.sum(a, b);
}

inline std::vector<int> operator+(std::vector<int> &a, std::vector<int> &b) {
  if (a.size() != b.size())
    throw std::invalid_argument("vectors must have same size");
  CLProcessor<int> processor;
  return processor.sum(a, b);
}
#endif

template <class T>
inline std::vector<T> operator*(const std::vector<T> &a,
                                const std::vector<T> &b) {
  if (a.size() != b.size())
    throw std::invalid_argument("vectors must have same size");
  std::vector<T> result(a.size());
#pragma omp parallel for
  for (size_t i = 0; i < a.size(); ++i) result[i] = a[i] * b[i];
  return result;
}

template <class T, class V>
inline auto operator*(const std::vector<T> &a, const V &b)
    -> std::vector<decltype(a[0] * b)> {
  std::vector<T> result(a.size());
#pragma omp parallel for
  for (size_t i = 0; i < a.size(); ++i) result[i] = a[i] * b;
  return result;
}

template <class T>
inline std::vector<T> operator/(const std::vector<T> &a,
                                const std::vector<T> &b) {
  if (a.size() != b.size())
    throw std::invalid_argument("vectors must have same size");
  std::vector<T> result(a.size());
#pragma omp parallel for
  for (size_t i = 0; i < a.size(); ++i) result[i] = a[i] / b[i];
  return result;
}

template <class T, class V>
inline auto operator/(const std::vector<T> &a, const V &b)
    -> std::vector<decltype(a[0] / b)> {
  std::vector<T> result(a.size());
#pragma omp parallel for
  for (size_t i = 0; i < a.size(); ++i) result[i] = a[i] / b;
  return result;
}

template <class T>
inline T sum(const std::vector<T> &a, const T &first = T()) {
  T accum = first;
#ifdef _OPENMP
#pragma omp parallel for reduction(+ : accum)
  for (size_t i = 0; i < a.size(); ++i) accum += a[i];
#else
  accum = std::accumulate(std::begin(a), std::end(a), first);
#endif
  return accum;
}

template <class T, class Functor>
inline T sum(const std::vector<T> &a, Functor functor, const T &first = T()) {
  T accum = first;
#ifdef _OPENMP
#pragma omp parallel for reduction(+ : accum)
  for (size_t i = 0; i < a.size(); ++i) accum += functor(a[i]);
#else
  accum = std::accumulate(std::begin(a), std::end(a), first, functor);
#endif
  return accum;
}

template <class T>
inline T sum(const std::vector<T> &a, T (*functor)(T v), const T &first = T()) {
  T accum = first;
#ifdef _OPENMP
#pragma omp parallel for reduction(+ : accum)
  for (size_t i = 0; i < a.size(); ++i) accum += functor(a[i]);
#else
  accum = std::accumulate(std::begin(a), std::end(a), first, functor);
#endif
  return accum;
}

template <class T, class RealType = T>
inline T strike_slip(const std::vector<T> &a, const size_t dt) {
  T accum = T();
#pragma omp parallel for reduction(+ : accum)
  for (size_t i = 0; i < a.size() - dt; ++i) {
    accum += fabs(a[i] - a[i + dt]);
  }
  return (RealType)accum / (a.size() - dt);
}

template <class T>
inline std::vector<T> alter_johnson(const std::vector<T> &a,
                                    double part = 0.6) {
  std::vector<T> result(ceil(a.size() * part) - 1);
#pragma omp parallel for
  for (size_t i = 1; i <= result.size(); ++i) {
    result[i - 1] = strike_slip(a, i);
  }
  return result;
}

#ifdef _OPENCL
template <>
inline std::vector<double> alter_johnson(const std::vector<double> &a,
                                         double part) {
  CLProcessor<double> clp;
  return clp.alter_johnson(a, ceil(a.size() * part));
}
#endif

#endif  // COMATH
