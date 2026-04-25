import ollama;

#include <iostream>

int main()
{
    ollama::allow_exceptions(false);

    Ollama server("http://localhost:11434");

    std::cout << (server.is_running() ? "Ollama is running" : "Ollama is not running") << std::endl;
}
