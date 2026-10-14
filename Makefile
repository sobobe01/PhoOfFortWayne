CXX      ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
# Header-only libraries: nlohmann/json and cpp-httplib (brew install nlohmann-json cpp-httplib)
PREFIX   := $(shell brew --prefix 2>/dev/null || echo /usr/local)
CPPFLAGS += -I$(PREFIX)/include
LDLIBS   += -lcurl -lpthread

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(SOURCES:src/%.cpp=build/%.o)

pho: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: src/%.cpp src/*.hpp | build
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

build:
	mkdir -p build

run: pho
	./pho

agent: pho
	./pho --agent

web: pho
	./pho --web

report: pho
	./pho --report

test: pho
	@sh tests/run.sh

clean:
	rm -rf pho build

.PHONY: run agent web report test clean
