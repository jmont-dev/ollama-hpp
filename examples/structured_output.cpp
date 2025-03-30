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

    const std::string model = "phi4-mini";

    std::cout << "Pulling model " << model << std::endl; 
    ollama::pull_model(model);
    std::cout << "Finished pulling model." << std::endl;

    ollama::request request(ollama::message_type::chat);

    request["model"] = model;
    ollama::messages messages = { ollama::message("user","Tell me about the country of Canada. Respond in JSON.") };
    request["messages"] = messages.to_json();
    request["stream"] = false;

    // We can define the desired output structure in JSON under the ['format'] key.
    request["format"] = json::parse( R"({
    "type": "object",
    "properties": {
      "population": {
        "type": "number",
        "description": "The population of the country, in millions of people."
      },
      "capital": {
        "type": "string"
      },
      "languages": {
        "type": "array",
        "description": "The monst common languages spoken in the country.",
        "items": {
          "type": "string"
        }
      }
    },
    "required": [
      "population",
      "capital", 
      "languages"
    ]
  })");

    ollama::response response = ollama::chat(request);

    std::cout << response.as_json_string() << std::endl;
}