#include "benchmark/benchmark.h"
#include "ik/memory.h"

int main(int argc, char** argv) {
    ik_memory_init();
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ik_memory_deinit();
}
