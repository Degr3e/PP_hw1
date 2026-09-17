#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <future>

namespace yalovsky {
  class Clicker {
  public:
    Clicker():
      start_(std::chrono::high_resolution_clock::now())
    {}

    double millisec() const
    {
      using std::chrono::high_resolution_clock;
      using std::chrono::duration_cast;
      using std::chrono::milliseconds;
      auto t = high_resolution_clock::now();
      return duration_cast< milliseconds >(t - start_).count();
    }

  private:
      std::chrono::time_point< std::chrono::high_resolution_clock > start_;
  };
}

using data_t = std::vector< unsigned long long >;
using value_t = data_t::value_type;
using const_iterator = data_t::const_iterator;

value_t summator(const_iterator b, const_iterator e)
{
  value_t s{0};
  for(; b != e; ++b) {
    s += *b;
  }
  return s;
}

int main(int argc, char** argv)
{
  constexpr size_t size{1'000'000'000};

  int threads{1};
  if (argc >= 2) {
    threads = std::stoi(argv[1]);
    if (threads <= 0) {
      std::cerr << "Incorrect count of threads\n";
      return 1;
    }
  }

  size_t valsOnThread = size / threads;
  double init{0}, total{0};
  value_t sum{0};

  try {
    yalovsky::Clicker cl;
    data_t vals(size, 1);
    std::vector< std::future< value_t > > tasks;
    tasks.reserve(threads);
    init = cl.millisec();

    for(size_t i = 0; i < threads - 1; ++i) {
      auto cbegin = vals.cbegin() + valsOnThread * i;
      auto cend = vals.cbegin() + valsOnThread * (i + 1);
      tasks.emplace_back(std::async(std::launch::async, summator, cbegin, cend));
    }
    auto cb = vals.cbegin() + valsOnThread * (threads - 1);
    auto ce = vals.cend();
    tasks.emplace_back(std::async(std::launch::async, summator, cb, ce));

    for(size_t i = 0; i < threads; ++i) {
      sum += tasks[i].get();
    }

    total = cl.millisec();
  } catch(const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  std::cout << "Init: " << init << '\n';
  std::cout << "Total: " << total << '\n';
  std::cout << "Duration of calc: " << total - init << '\n';
  std::cout << "Sum: " << sum << '\n';
  std::cout << "Threads: " << threads << '\n';
}