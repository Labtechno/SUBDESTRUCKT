CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -ffp-contract=off -Iinclude
BIN = build

all: $(BIN)/s0_tests $(BIN)/s0_cli

$(BIN)/s0_tests: tests/s0_tests.cpp include/s0/*.hpp
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN)/s0_cli: src/s0_cli.cpp include/s0/*.hpp
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(BIN)/s0_tests
	./$(BIN)/s0_tests

clean:
	rm -rf $(BIN)
