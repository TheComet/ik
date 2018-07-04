#include "benchmark/benchmark.h"
#include "ik/ik.h"

int main(int argc, char** argv) {
    IKAPI.init();
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    IKAPI.deinit();
}
