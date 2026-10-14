CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2

pho: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp

run: pho
	./pho

test: pho
	@for t in tests/*.in; do ./pho < $$t | diff -u $${t%.in}.expected - || exit 1; done
	@echo "tests passed"

clean:
	rm -f pho

.PHONY: run test clean
