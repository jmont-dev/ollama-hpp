#include "ollama.hpp"

#include <iostream>
#include <string>
#include <functional>


void on_receive_token(const std::string& token)
{

    //std::istringstream iss(res->body);

    //std::string line;
    //while(std::getline(iss,line))
    //{            
        json chunk = json::parse(token);        
        std::string response = chunk["response"];
        std::cout << response << std::endl;
    //}

}

int main()
{
    std::cout << ollama::is_running() << std::endl;
    std::cout << ollama::get_version() << std::endl;
    std::cout << ollama::generate("mistral", "Why is the sky blue?") << std::endl;

    //std::cout << ollama.generate("mistral", "Why is the sky blue?", std::bind(on_receive_token)) << std::endl;

}