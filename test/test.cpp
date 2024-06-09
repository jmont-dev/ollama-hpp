#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ollama.hpp"

#include <iostream>
#include <string>

TEST_CASE("Pull, Copy, and Delete Models") {

    ollama::show_requests(true);

    // Pull a model by specifying a model name.
    CHECK( ollama::pull_model("llama3:8b") == true );

    // Copy a model by specifying a source model and destination model name.
    CHECK( ollama::copy_model("llama3:8b", "llama3_copy") ==true );

    // Delete a model by specifying a model name.
    CHECK( ollama::delete_model("llama3_copy") == true );

}

TEST_CASE("Basic Generation") {

    ollama::show_requests(true);

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

TEST_CASE("Embedding Generation") {

    ollama::show_requests(true);
    ollama::show_replies(true);

    // Use a seed and 0 temperature to generate deterministic outputs. num_predict determines the number of tokens generated.
    ollama::options options;
    options["seed"] = 1;
    options["temperature"] = 0;
    options["num_predict"] = 18;

    ollama::response response = ollama::generate_embeddings("mistral", "Why is the sky blue?");
    std::cout << response << std::endl;

    std::string expected_response = "";

    CHECK(response.as_simple_string() == expected_response);
}