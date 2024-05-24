#include "ollama.hpp"
#include "json.hpp"

#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <chrono>
#include <atomic>

std::string output;

std::atomic<bool> done{false};

void on_receive_token(const std::string& token, bool done)
{
    std::cout << token << std::flush;
    if (done) std::cout << std::endl;
    output +=token;
}

void on_receive_response(const ollama::response& response)
{   
    std::cout << response.as_simple_string() << std::flush;

    if (response.as_json()[0]["done"]==true) { done=true;  std::cout << std::endl;}
}

// Install ollama, llama3, and llava first to run this demo
// ollama pull llama3 llava 
int main()
{

    ollama::show_requests(true);
    ollama::show_replies(true);


    // Pull a model by specifying a model name.
    if ( ollama::pull_model("phi") ) std::cout << "Model was pulled" << std::endl;

    // Copy a model by specifying a source model and destination model name.
    if ( ollama::copy_model("llama3:8b", "llama3_copy") ) std::cout << "Model was copied" << std::endl;

    // Delete a model by specifying a model name.
    if ( ollama::delete_model("llama3:8b") ) std::cout << "Model was deleted" << std::endl;

    // Request model info from the Ollama server.
    json model_info = ollama::show_model_info("llama3");
    std::cout << "Model family is " << model_info["details"]["family"] << std::endl;

    // Create a blob on the ollama server using the following digest
    try { ollama::create_blob("sha256:29fdb92e57cf0827ded04ae6461b5931d01fa595843f55d36f5b275a52087dd2"); std::cout << "Blob was created on Ollama server." << std::endl; }
    catch( ollama::exception e) { std::cout << "Error when creating blob: " << e.what() << std::endl;} 


    // Check if a blob with the following digest exists.
    if ( ollama::blob_exists("sha256:29fdb92e57cf0827ded04ae6461b5931d01fa595843f55d36f5b275a52087dd2") ) std::cout << "Blob exists on Ollama server." << std::endl; 

    sleep(10);

    // List the models available locally in the ollama server
    std::vector<std::string> models = ollama::list_models();    
    std::cout << "These models are locally available: " << std::endl;
    for (std::string model: models) std::cout << model << std::endl;

    ollama::image image("llama.jpg", true);   
    //std::vector<ollama::image> images = {image};

    ollama::images images={image};

    std::cout << ollama::generate("llava", "What is this an image of?", nullptr,images) << std::endl;

    // Enable debug logging for raw requests and replies sent to and from the Ollama server.
    ollama::show_requests(false);
    ollama::show_replies(false);

    // Functions will throw ollama::exception if invalid parameters are used or an invalid response is received.
    try { 
        ollama::generate("Non-existent-model", "Requesting this model will throw an error"); 
    } 
    catch(ollama::exception e) { std::cout << e.what() << std::endl; }

    //Alternatively, throwing exceptions can be disabled. In this case, either emptry values or false will be returned in the event of an error.
    //ollama::allow_exceptions(false);

    // Optional. By default, the server URL is set to http://localhost:11434. Use this function if you need to point to a different URL.
    ollama::setServerURL("http://localhost:11434");    

    // Optional. Set the read and write timeouts in seconds for receiving from and sending data to ollama.
    // If you have a large model with a long response time you may need to increase these.
    ollama::setReadTimeout(120);
    ollama::setWriteTimeout(120);

    // Check to see whether the ollama server is running.
    std::cout << (ollama::is_running() ? "Ollama is running" : "Ollama is not running") << std::endl;
    
    // Get the version of the ollama server
    std::cout << ollama::get_version() << std::endl;

    // Optionally send a request to ollama to load a model into memory.
    // This will occur automatically during generation but this allows you to preload a model before using it.
    bool model_loaded = ollama::load_model("llama3");

    // Perform a simple generation to a string by specifying a model and a prompt. The response will be returned as one string without streaming the reply.
    std::cout << ollama::generate("llama3", "Why is the sky blue?") << std::endl;
    
    // Set options to include with use of the model. These should be specified in JSON.
    json options;
    options["options"]["top_k"] = 20;
    
    // You can also pass in the options as a string containing json.
    std::string string_options="{options: {top_k:20} }";

    // Perform a simple generation which includes model options.
    std::cout << ollama::generate("llama3", "Why is the sky green?", options) << std::endl;

    // Define a callback function to receive a streaming reply from the server. This will allow you to perform an action on receiving each token.
    // Socket communication is blocking so this will still block the main thread.
    std::function<void(const std::string&, bool)> callback = on_receive_token;    
    ollama::generate("llama3", "Why is the sky red?", callback);

    std::function<void(const ollama::response&)> response_callback = on_receive_response;  
    ollama::generate("llama3", "Why is the sky orange?", response_callback);

    // You can launch the generation in a thread with a callback to use it asynchronously.
    std::thread new_thread( [response_callback]{ ollama::generate("llama3", "Why is the sky gray?", response_callback); } );

    // Prevent the main thread from exiting while we wait for an asynchronous response.
    while (!done) { std::this_thread::sleep_for(std::chrono::microseconds(100) ); }
    new_thread.join();

    // If you don't want to use the static singleton defined in the namespace, you can create an Ollama instance itself.
    // This is helpful if you have multiple Ollama servers or need custom control over the object.
    Ollama my_ollama_server("http://localhost:11434");

    // You can use all of the same functions from this instanced version of the class.
    std::cout << my_ollama_server.generate("llama3", "Why is the sky blue?") << std::endl;

    // Enable debug logging for raw requests and replies sent to and from the Ollama server.
    ollama::show_requests(true);
    ollama::show_replies(false);
    
}