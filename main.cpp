#include "vmath.h"
#include <iostream>
#include <string>
#include <vector>
#include <sys/time.h>
#include <tclap/CmdLine.h>
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
    std::cerr << name << " tooks " << elapsedTime << " ms" << std::endl;
  }
};

int main(int argc, char *argv[]) {
  TCLAP::CmdLine cmd("Vector math", ' ', "1.0");
  TCLAP::SwitchArg alter_djonson("a", "alter-djonson",
                                 "Calculate Alter-Djonson", false);
  TCLAP::SwitchArg normalize("n", "normalize", "Normalize (/max)", false);
  TCLAP::ValueArg<int> columnNum("c", "column", "Column number", false, 0,
                                 "int");
  TCLAP::SwitchArg reduce("s", "sum", "Sum of numbers", false);
  TCLAP::SwitchArg mx("x", "max", "Max of numbers", false);
  TCLAP::SwitchArg mn("m", "min", "Min of numbers", false);
  TCLAP::SwitchArg av("v", "avg", "Average of numbers", false);
  TCLAP::SwitchArg total("t", "total", "Sum/Min/Max/Avg", false);
  std::vector<double> v;
  std::string tmp, line;
  std::stringstream ss;
  uint64_t count = 0;
  double val, sum = 0, min = INFINITY, max = -INFINITY;
  cmd.add(alter_djonson);
  cmd.add(columnNum);
  cmd.add(normalize);
  cmd.add(reduce);
  cmd.add(mx);
  cmd.add(mn);
  cmd.add(av);
  cmd.add(total);
  cmd.parse(argc, argv);

  std::cerr << "Reading standart input" << std::endl;
  while (!std::cin.eof()) {
    std::getline(std::cin, line);
    ss.str(line);
    for (int i = 0; i < columnNum.getValue() - 1 && !ss.eof(); ++i) {
      ss >> tmp;
    }
    if (ss.eof()) break;
    ss >> val;
    sum += val;
    if (val < min) min = val;
    if (val > max) max = val;
    if (!reduce.isSet() && !mn.isSet() && !mx.isSet() && !total.isSet() &&
        !av.isSet())
      v.push_back(val);
    ++count;
  }
  std::cerr << "Read: " << count << " points" << std::endl;
  if (reduce.isSet()) {
    std::cout << sum << std::endl;
    return 0;
  }
  if (mx.isSet()) {
    std::cout << max << std::endl;
    return 0;
  }
  if (mn.isSet()) {
    std::cout << min << std::endl;
    return 0;
  }
  if (av.isSet()) {
    std::cout << sum / count << std::endl;
    return 0;
  }
  if (total.isSet()) {
    std::cout << sum << '\t' << min << '\t' << max << '\t' << sum / count
              << std::endl;
    return 0;
  }
  if (normalize.isSet()) {
    TimePrinter printer("Normalize");
    auto maxe = *std::max_element(std::begin(v), std::end(v));
    if (maxe != 0) {
      vec::div(v, v, maxe);
    } else
      std::cerr << "MAX is 0" << std::endl;
  }
  if (alter_djonson.isSet()) {
    TimePrinter printer("Alter-Djonson");
    v = vec::alter_johnson(v);
  }
  for (auto &val : v) {
    std::cout << val << std::endl;
  }
  return 0;
}
