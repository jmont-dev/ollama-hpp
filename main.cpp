#include "ollama.hpp"

#include <iostream>
#include <string>
#include <functional>
#include <thread>

std::string output;

void on_receive_token(const std::string& token, bool done)
{
    std::cout << token << std::flush;
    if (done) std::cout << std::endl;
    //output +=token;
}

int main()
{
    std::cout << ollama::is_running() << std::endl;
    std::cout << ollama::get_version() << std::endl;
    //std::cout << ollama::generate("llama3", "Why is the sky blue?") << std::endl;
    
    std::function<void(const std::string&, bool)> callback = on_receive_token;    
    ollama::generate("llama3", "Why is the sky blue?", callback);

    //std::thread new_thread( [callback]{ ollama::generate("llama3", "Why is the sky blue?", callback); } );

    while(1)
    {
        usleep(100);
    }
    
    //std::cout << ollama.generate("mistral", "Why is the sky blue?", std::bind(on_receive_token)) << std::endl;

}