# Check if a default C++ compiler exists, otherwise use g++
CXX ?= g++

all: examples test-cpp11 test-cpp14 test-cpp20
examples: examples/main.cpp
	mkdir -p build
	$(CXX) examples/main.cpp -Iinclude -o build/examples -std=c++11 -pthread -latomic
test: test-cpp11
test-cpp11: test/test.cpp
	mkdir -p build
	$(CXX) test/test.cpp -Iinclude -Itest -o build/test -std=c++11 -pthread -latomic
test-cpp14: test/test.cpp
	mkdir -p build
	$(CXX) test/test.cpp -Iinclude -Itest -o build/test-cpp14 -std=c++14 -pthread -latomic
test-cpp20: test/test.cpp
	mkdir -p build
	$(CXX) test/test.cpp -Iinclude -Itest -o build/test-cpp20 -std=c++2a -pthread -latomic
clean:
	rm -rf build
