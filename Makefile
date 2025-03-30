# Check if a default C++ compiler exists, otherwise use g++
CXX ?= g++
CXXFLAGS = -Wall -Wextra -Wpedantic

CREATE_BUILD_DIR = mkdir -p build; cp -n llama.jpg build;

all: examples function_calling structured_output test-cpp11 test-cpp14 test-cpp17 test-cpp20
build:
	mkdir -p build
ifeq ($(OS),Windows_NT)
		if not exist "build/llama.jpg" copy "llama.jpg" "build"
else
		cp -n llama.jpg build
endif
examples: build examples/main.cpp
	$(CXX) $(CXXFLAGS) examples/main.cpp -Iinclude -o build/examples -std=c++11 -pthread -latomic
function_calling: build examples/function_calling.cpp
	$(CXX) $(CXXFLAGS) examples/function_calling.cpp -Isingleheader -o build/function_calling -std=c++11 -pthread -latomic
structured_output: build examples/structured_output.cpp
	$(CXX) $(CXXFLAGS) examples/structured_output.cpp -Isingleheader -o build/structured_output -std=c++11 -pthread -latomic

test: test-cpp11
test-cpp11: build test/test.cpp
	$(CXX) $(CXXFLAGS) test/test.cpp -Iinclude -Itest -o build/test -std=c++11 -pthread -latomic
test-cpp14: build test/test.cpp
	$(CXX) $(CXXFLAGS) test/test.cpp -Iinclude -Itest -o build/test-cpp14 -std=c++14 -pthread -latomic
test-cpp17: build test/test.cpp
	$(CXX) $(CXXFLAGS) test/test.cpp -Iinclude -Itest -o build/test-cpp17 -std=c++17 -pthread -latomic	
test-cpp20: build test/test.cpp
	$(CXX) $(CXXFLAGS) test/test.cpp -Iinclude -Itest -o build/test-cpp20 -std=c++2a -pthread -latomic
clean:
	rm -rf build
