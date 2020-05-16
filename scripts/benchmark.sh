#!/usr/bin/env bash

./scripts/build.sh && \
    ./build/bin/benchmark_pure_simd \
	--benchmark_repetitions=10 \
	--benchmark_report_aggregates_only=true \
	$*

