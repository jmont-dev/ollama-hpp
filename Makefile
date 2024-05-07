examples: examples/main.cpp
	mkdir -p build
	g++ examples/main.cpp -Iinclude -o build/ollama-test -std=c++11 -pthread -latomic
clean:
	rm -rf build
