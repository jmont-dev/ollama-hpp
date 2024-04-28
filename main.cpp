#include "ollama.hpp"

#include <iostream>
#include <string>

int main()
{
    Ollama ollama;
    std::cout << ollama.generate("mistral", "Why is the sky blue?") << std::endl;
}