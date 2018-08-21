#include "benchmark/benchmark.h"
#include "ik/ik.h"

int main(int argc, char** argv) {
    ik_init();
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ik_deinit();
}
