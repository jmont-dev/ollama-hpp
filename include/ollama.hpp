#ifndef OLLAMA_HPP
#define OLLAMA_HPP

// Used for sending REST requests to the ollama server
#include "cpp-httplib/httplib.h"

//Used for formatting JSON with ollama
#include "json.hpp"

//Used for Base64 formatting of images
#include "Base64.h"

#include <string>
#include <fstream>
#include <iostream>
#include <functional>

using json = nlohmann::json;

class Ollama
{
    public:

        Ollama(const std::string& url)
        {
            this->server_url = url;
            this->cli = new httplib::Client(url);
            this->setReadTimeout(120);
        }

        Ollama(): Ollama("http://localhost:11434") {}
        ~Ollama() {}

    // Generate a non-streaming reply as a string.
    std::string generate(const std::string& model,const std::string& prompt, bool return_as_json=false)
    {


        //std::vector<std::string> imageFiles;

        std::string response="";

        // Generate the JSON request
        json request;
        request["model"] = model;
        request["prompt"] = prompt;
        request["stream"] = false;

        std::string request_string = request.dump();
        std::cout << request_string << std::endl;      

        if (auto res = this->cli->Post("/api/generate",request_string, "application/json"))
        {
            std::cout << res->body << std::endl;

                if (return_as_json) response+=res->body;
                else
                {
                    json chunk = json::parse(res->body);        
                    response+=chunk["response"];
                }
        }
        else
        {
            std::cout << "No response returned: " << res.error() << std::endl;
        }

        return response;
    }


    // Generate a streaming reply where a user-defined callback function is invoked when each token is received.
    void generate(const std::string& model,const std::string& prompt, std::function<void(const std::string&, bool)> on_receive_token, bool receive_json=false)
    {
        std::string response="";

        // Generate the JSON request
        json request;
        request["model"] = model;
        request["prompt"] = prompt;
        request["stream"] = true;

        std::string request_string = request.dump();
        std::cout << request_string << std::endl;

        auto stream_callback = [on_receive_token, receive_json](const char *data, size_t data_length)->bool{
            
            std::string message(data, data_length);
            json chunk = json::parse(message);        
            
            std::string response=chunk["response"];
            bool done = chunk["done"];

            // Either pass back the entire json string or the parsed token.
            on_receive_token( (receive_json ? message: response), done);

            return !done;

        };

        if (auto res = this->cli->Post("/api/generate", request_string, "application/json", stream_callback)) {}
        else {std::cout << "No response returned: " << res.error() << std::endl;}

    }

    void create(const std::string& modelName, const std::string& modelFile, bool loadFromFile=true)
    {

        // Generate the JSON request
        json request;
        request["name"] = modelName;

        if (loadFromFile)
        {
            // Open the file
            std::ifstream file(modelFile, std::ios::binary);

            // Check if the file is open
            if (!file.is_open()) {
                std::cerr << "Failed to open file.\n";
                return;
            }

            // Read the entire file into a string using iterators
            std::string file_contents((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
            
            request["modelFile"] = file_contents;                                                
        }
        else request["modelFile"] = modelFile;

        std::string request_string = request.dump();
        std::cout << request_string << std::endl;  

        std::string response;

        if (auto res = this->cli->Post("/api/create",request_string, "application/json"))
        {
            std::cout << res->body << std::endl;

            json chunk = json::parse(res->body);        
            response+=chunk["response"];
        }
        else
        {
            std::cout << "No response returned: " << res.error() << std::endl;
        }

    }


    bool is_running()
    {
        std::stringstream response;

        auto res = cli->Get("/");

        if (res) response << res->body;

        if (response.str()=="Ollama is running") return true;
        
        return false;
    }

    std::string get_version()
    {
        std::string version;
        httplib::Client cli("http://localhost:11434");

        auto res = this->cli->Get("/api/version");

        if (res)
        {
            json response = json::parse(res->body);        
            version = response["version"];
        }
        else
        {
            throw std::runtime_error("Error retrieving version: "+res->status);
        }

        return version;

    }

    void setServerURL(const std::string& server_url)
    {
        this->server_url = server_url;
        delete(this->cli);        
        this->cli = new httplib::Client(server_url);
    }

    void setReadTimeout(const int& seconds)
    {
        this->cli->set_read_timeout(seconds);
    }

    void setWriteTimeout(const int& seconds)
    {
        this->cli->set_write_timeout(seconds);
    }

    private:

    std::string server_url;
    httplib::Client *cli;

};

namespace ollama
{
    // Use directly from the namespace as a singleton
    static Ollama ollama;
    
    inline void setServerURL(const std::string& server_url)
    {
        ollama.setServerURL(server_url);
    }

    inline std::string generate(const std::string& model,const std::string& prompt, bool return_as_json=false)
    {
        return ollama.generate(model, prompt, return_as_json);
    }

    inline void generate(const std::string& model,const std::string& prompt, std::function<void(const std::string&, bool)> on_receive_token, bool return_as_json=false)
    {
        return ollama.generate(model, prompt, on_receive_token, return_as_json);
    }

    inline void create(const std::string& modelName, const std::string& modelFile, bool loadFromFile=true)
    {
        return ollama.create(modelName, modelFile, loadFromFile);
    }

    inline bool is_running()
    {
        return ollama.is_running();
    }

    inline std::string get_version()
    {
        return ollama.get_version();
    }

    inline void setReadTimeout(const int& seconds)
    {
        return ollama.setReadTimeout(seconds);
    }

    inline void setWriteTimeout(const int& seconds)
    {
        return ollama.setWriteTimeout(seconds);
    }

}


#endif