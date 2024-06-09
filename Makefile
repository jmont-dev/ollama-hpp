all: examples test
examples: examples/main.cpp
	mkdir -p build
	g++ examples/main.cpp -Iinclude -o build/examples -std=c++11 -pthread -latomic
test: test/test.cpp
	mkdir -p build
	g++ test/test.cpp -Iinclude -Itest -o build/test -std=c++11 -pthread -latomic	
clean:
	rm -rf build
