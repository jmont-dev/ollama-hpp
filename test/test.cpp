#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ollama.hpp"

#include <algorithm>
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

TEST_CASE("Model Info") {

    ollama::show_requests(true);

    // Request model info from the Ollama server.
    nlohmann::json model_info = ollama::show_model_info("llama3:8b");
    std::cout << "Model family is " << model_info["details"]["family"] << std::endl;

    CHECK(model_info["details"]["family"] == "llama");
}

TEST_CASE("List Local Models") {

    ollama::show_requests(true);

    // List the models available locally in the ollama server
    std::vector<std::string> models = ollama::list_models();    

    bool contains_model = (std::find(models.begin(), models.end(), "llama3:8b") != models.end() );

    CHECK( contains_model );
}

TEST_CASE("Basic Generation") {

    ollama::show_requests(true);

    // Use a seed and 0 temperature to generate deterministic outputs. num_predict determines the number of tokens generated.
    ollama::options options;
    options["seed"] = 1;
    options["temperature"] = 0;
    options["num_predict"] = 18;

    ollama::response response = ollama::generate("llama3:8b", "Why is the sky blue?", options);
    //std::cout << response << std::endl;

    std::string expected_response = "What a great question!\n\nThe sky appears blue because of a phenomenon called Rayleigh scattering,";

    CHECK(response.as_simple_string() == expected_response);
}

TEST_CASE("Embedding Generation") {

    ollama::show_requests(true);

    // Use a seed and 0 temperature to generate deterministic outputs. num_predict determines the number of tokens generated.
    ollama::options options;
    options["seed"] = 1;
    options["temperature"] = 0;
    options["num_predict"] = 18;

    ollama::response response = ollama::generate_embeddings("llama3:8b", "Why is the sky blue?");
    std::cout << response << std::endl;

    CHECK(response.as_json().contains("embedding") == true);
}

/*
TEST_CASE("Create and Check Blobs") {

    ollama::show_requests(true);

    // Create a blob on the ollama server using the following digest
    try { ollama::create_blob("sha256:29fdb92e57cf0827ded04ae6461b5931d01fa595843f55d36f5b275a52087dd2"); std::cout << "Blob was created on Ollama server." << std::endl; }
    catch( ollama::exception e) { std::cout << "Error when creating blob: " << e.what() << std::endl;} 

    // Check if a blob with the following digest exists.
    CHECK(ollama::blob_exists("sha256:29fdb92e57cf0827ded04ae6461b5931d01fa595843f55d36f5b275a52087dd2") == true);
}
*/