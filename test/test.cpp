#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ollama.hpp"

#include <iostream>
#include <string>

TEST_CASE("Basic Generation Test") {

    ollama::show_requests(true);
    ollama::show_replies(false);
    ollama::allow_exceptions(true);

    // Use a seed and 0 temperature to generate deterministic outputs. num_predict determines the number of tokens generated.
    ollama::options options;
    options["seed"] = 1;
    options["temperature"] = 0;
    options["num_predict"] = 18;

    ollama::response response = ollama::generate("llama3", "Why is the sky blue?", options);
    //std::cout << response << std::endl;

    std::string expected_response = "What a great question!\n\nThe sky appears blue because of a phenomenon called Rayleigh scattering,";

    CHECK(response.as_simple_string() == expected_response);
}
