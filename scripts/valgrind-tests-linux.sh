#!/bin/sh
cd build/tests || echo "Error: Please run this script from the project's root directory as ./scripts/valgrind-tests-linux.sh"

echo "Started valgrind..."
valgrind --num-callers=50 \
	--leak-resolution=high \
	--leak-check=full \
	--track-origins=yes \
	--time-stamp=yes \
	--suppressions=../../scripts/valgrind-suppressions.supp \
	./iktests 2>&1 | tee ../../tests.grind
cd .. && cd ..
