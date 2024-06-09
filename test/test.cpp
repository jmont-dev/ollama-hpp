#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ollama.hpp"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <string>

// Use a seed and 0 temperature to generate deterministic outputs. num_predict determines the number of tokens generated.
// Note that this is static. We will use these options for other generations.
static ollama::options options;

TEST_SUITE("Ollama Tests") {

    TEST_CASE("Initialize Options") {

        ollama::show_requests(false);
        ollama::show_replies(false);
        ollama::allow_exceptions(true);

        options["seed"] = 1;
        options["temperature"] = 0;
        options["num_predict"] = 18;

        CHECK(true);
    }

    TEST_CASE("Check if Ollama is Running") {

        CHECK( ollama::is_running() );
    }

    TEST_CASE("Get Version") {

        CHECK( ollama::get_version()!="" );
    }

    TEST_CASE("Set Server Parameters") {

        // Optional. By default, the server URL is set to http://localhost:11434. Use this function if you need to point to a different URL.
        ollama::setServerURL("http://localhost:11434");    

        // Optional. Set the read and write timeouts in seconds for receiving from and sending data to ollama.
        // If you have a large model with a long response time you may need to increase these.
        ollama::setReadTimeout(120);
        ollama::setWriteTimeout(120);

        CHECK(true);
    }

    TEST_CASE("Load Model") {

        CHECK( ollama::load_model("llama3:8b") );
    }

    TEST_CASE("Pull, Copy, and Delete Models") {

        // Pull a model by specifying a model name.
        CHECK( ollama::pull_model("llama3:8b") == true );

        // Copy a model by specifying a source model and destination model name.
        CHECK( ollama::copy_model("llama3:8b", "llama3_copy") ==true );

        // Delete a model by specifying a model name.
        CHECK( ollama::delete_model("llama3_copy") == true );
    }

    TEST_CASE("Model Info") {

        // Request model info from the Ollama server.
        nlohmann::json model_info = ollama::show_model_info("llama3:8b");
        //std::cout << "Model family is " << model_info["details"]["family"] << std::endl;

        CHECK(model_info["details"]["family"] == "llama");
    }

    TEST_CASE("List Local Models") {

        // List the models available locally in the ollama server
        std::vector<std::string> models = ollama::list_models();    

        bool contains_model = (std::find(models.begin(), models.end(), "llama3:8b") != models.end() );

        CHECK( contains_model );
    }

    TEST_CASE("Exception Handling") {

        bool exception_handled = false;

        // If exceptions are enabled, functions will throw ollama::exception if invalid parameters are used or an invalid response is received.
        try { 
            ollama::generate("Non-existent-model", "Requesting this model will throw an error"); 
        } 
        catch(ollama::exception e) { exception_handled = true; }

        CHECK( exception_handled );
    }

    TEST_CASE("Basic Generation") {

        ollama::response response = ollama::generate("llama3:8b", "Why is the sky blue?", options);
        //std::cout << response << std::endl;

        std::string expected_response = "What a great question!\n\nThe sky appears blue because of a phenomenon called Rayleigh scattering,";

        CHECK(response.as_simple_string() == expected_response);
    }


    std::atomic<bool> done{false};
    std::string streamed_response;


    void on_receive_response(const ollama::response& response)
    {   
        streamed_response+=response.as_simple_string();

        if (response.as_json()["done"]==true) done=true;
    }

    TEST_CASE("Streaming Generation") {

        std::function<void(const ollama::response&)> response_callback = on_receive_response;  
        ollama::generate("llama3:8b", "Why is the sky blue?", response_callback, options);

        std::string expected_response = "What a great question!\n\nThe sky appears blue because of a phenomenon called Rayleigh scattering,";

        CHECK( streamed_response == expected_response );
    }

    TEST_CASE("Non-Singleton Generation") {

        Ollama my_ollama_server("http://localhost:11434");

        // You can use all of the same functions from this instanced version of the class.
        ollama::response response = my_ollama_server.generate("llama3:8b", "Why is the sky blue?", options);
        //std::cout << response << std::endl;

        std::string expected_response = "What a great question!\n\nThe sky appears blue because of a phenomenon called Rayleigh scattering,";

        CHECK(response.as_simple_string() == expected_response);
    }

    TEST_CASE("Single-Message Chat") {

        ollama::message message("user", "Why is the sky blue?");

        ollama::response response = ollama::chat("llama3:8b", message, options);

        std::string expected_response = "What a great question!\n\nThe sky appears blue because of a phenomenon called Rayleigh scattering,";

        CHECK(response.as_simple_string()!="");
    }

    TEST_CASE("Multi-Message Chat") {

        ollama::message message1("user", "What are nimbus clouds?");
        ollama::message message2("assistant", "Nimbus clouds are dense, moisture-filled clouds that produce rain.");
        ollama::message message3("user", "What are some other kinds of clouds?");

        ollama::messages messages = {message1, message2, message3};

        ollama::response response = ollama::chat("llama3:8b", messages, options);

        std::string expected_response = "";

        CHECK(response.as_simple_string()!="");
    }

    TEST_CASE("Generation with Image") {

        ollama::show_requests(false);

        options["num_predict"] = 12;

        ollama::image image = ollama::image::from_file("llama.jpg");

        //ollama::images images={image};

        ollama::response response = ollama::generate("llava", "What do you see in this image?", options, image);
        std::string expected_response = " The image features a large, fluffy white llama";

        CHECK(response.as_simple_string() == expected_response);
    }

    TEST_CASE("Generation with Multiple Images") {

        ollama::show_requests(false);

        options["num_predict"] = 14;

        ollama::image image = ollama::image::from_file("llama.jpg");
        ollama::image base64_image = ollama::image::from_base64_string("iVBORw0KGgoAAAANSUhEUgAAAAoAAAAKCAYAAACNMs+9AAAAFUlEQVR42mNkYPhfz0AEYBxVSF+FAP5FDvcfRYWgAAAAAElFTkSuQmCC");

        ollama::images images={image, base64_image};

        ollama::response response = ollama::generate("llava", "What do you see in this image?", options, images);
        std::string expected_response = " The image features a large, fluffy white and gray llama";

        CHECK(response.as_simple_string() == expected_response);
    }

    TEST_CASE("Chat with Image") {

        ollama::show_requests(false);

        options["num_predict"] = 12;

        ollama::image image = ollama::image::from_file("llama.jpg");

        // We can optionally include images with each message. Vision-enabled models will be able to utilize these.
        ollama::message message_with_image("user", "What do you see in this image?", image);
        ollama::response response = ollama::chat("llava", message_with_image, options);

        std::string expected_response = " The image features a large, fluffy white llama";

        CHECK(response.as_simple_string()!="");
    }

    TEST_CASE("Embedding Generation") {

        options["num_predict"] = 18;

        ollama::response response = ollama::generate_embeddings("llama3:8b", "Why is the sky blue?");
        //std::cout << response << std::endl;

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
}