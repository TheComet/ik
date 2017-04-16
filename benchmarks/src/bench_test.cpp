#include "benchmark/benchmark.h"
#include <string>

using namespace ::benchmark;

static void test(State& state)
{
    while (state.KeepRunning())
        std::string empty_string;
}
BENCHMARK(test);

