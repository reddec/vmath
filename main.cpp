#include <iostream>
#include <vector>
#include <sys/time.h>
#include <algorithm>
#include <cmath>
#include "comath.h"
using namespace std;

struct TimePrinter {
  timeval t1, t2;
  std::string name;

  TimePrinter(const std::string &name_) : name(name_) {
    gettimeofday(&t1, NULL);
  }

  ~TimePrinter() {
    gettimeofday(&t2, NULL);
    double elapsedTime;
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;     // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;  // us to ms
    std::cout << name << " tooks " << elapsedTime << " ms" << std::endl;
  }
};

template <class T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &data) {
  out << '{';
  for (auto &v : data) out << ' ' << v;
  out << " }";
  return out;
}

int main() {
  size_t count = 1;
  std::vector<double> v(10000);
  std::vector<double> v2(10000);
  std::vector<double> v3;
  double accum, strikeSlip;

  for (size_t i = 0; i < v.size(); ++i) {
    v[i] = i % 2 == 0 ? -1 : 1;
    v2[i] = v[i];
  }
  {
    v3 = v + v2;
    TimePrinter p("Add (omp)");
    for (size_t i = 0; i < count; ++i) v3 = v + v2;
  }
  {
    TimePrinter p("Sum (omp)");
    for (size_t i = 0; i < count; ++i) accum = sum(v, fabs);
  }
  {
    TimePrinter p("Strike-slip (omp)");
    for (size_t i = 0; i < count; ++i) strikeSlip = strike_slip(v, 10);
  }
  {
    TimePrinter p("Alter-Djohnson (omp)");
    for (size_t i = 0; i < count; ++i) v3 = alter_johnson(v);
  }
  std::cout << "------------------------------" << std::endl;
  // std::cout << v << std::endl;
  std::cout << "OMP: " << accum << std::endl;
  std::cout << "SS: " << strikeSlip << std::endl;
  return 0;
}
