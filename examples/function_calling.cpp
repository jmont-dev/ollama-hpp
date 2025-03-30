#include "ollama.hpp"

#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <chrono>
#include <atomic>

using json = nlohmann::json;

int main()
{

    ollama::show_requests(false);
    ollama::show_replies(false);
    ollama::allow_exceptions(true);

    const std::string model = "llama3.1:8b";

    std::cout << "Pulling model " << model << std::endl; 
    ollama::pull_model(model);
    std::cout << "Finished pulling model." << std::endl;

    ollama::request request(ollama::message_type::chat);

    request["model"] = model;
    ollama::messages messages = { ollama::message("user","What is the weather in Madrid?") };
    request["messages"] = messages.to_json();
    request["stream"] = false;

    // We can encode a list of tools in JSON under the "tools" key.
    request["tools"] = json::parse( R"([
    {
      "type": "function",
      "function": {
        "name": "get_current_weather",
        "description": "Get the current weather for a location",
        "parameters": {
          "type": "object",
          "properties": {
            "location": {
              "type": "string",
              "description": "The location to get the weather for, e.g. San Francisco, CA"
            },
            "format": {
              "type": "string",
              "description": "The format to return the weather in, e.g. 'celsius' or 'fahrenheit'",
              "enum": ["celsius", "fahrenheit"]
            }
          },
          "required": ["location", "format"]
        }
      }
    }
])");

    ollama::response response = ollama::chat(request);

    std::cout << response.as_json() << std::endl;
}