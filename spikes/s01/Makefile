# SUBDESTRUCKT S0.1 — build macOS / Linux (clang++ ou g++). Windows : build_msvc.bat
# -ffp-contract=off : pas de FMA implicite (déterminisme bit-exact ; clang arm64 contracte par défaut).
CXX ?= c++
ARCHFLAGS ?=
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -ffp-contract=off $(ARCHFLAGS) -Iinclude -I../s0/include
LDFLAGS ?= -pthread
BIN = build
HDR = $(wildcard include/s01/*.hpp) ../s0/include/s0/analysis.hpp

all: $(BIN)/s01_tests $(BIN)/s01_cli $(BIN)/s01_bench

$(BIN)/%: tests/%.cpp $(HDR)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)
$(BIN)/s01_cli: src/s01_cli.cpp $(HDR)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)
$(BIN)/s01_bench: bench/s01_bench.cpp $(HDR)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test: $(BIN)/s01_tests
	./$(BIN)/s01_tests

# Benchmark Apple M1 (PENDING USER MACHINE) : make bench-m1
bench-m1:
	$(MAKE) clean
	$(MAKE) $(BIN)/s01_bench CXX=clang++ ARCHFLAGS=-mcpu=apple-m1
	./$(BIN)/s01_bench data/bench_m1.csv

clean:
	rm -rf $(BIN)
