#ifndef OLLAMA_HPP
#define OLLAMA_HPP

// Used for sending REST requests to the ollama server
#include "cpp-httplib/httplib.h"

//Used for formatting JSON with ollama
#include "json.hpp"

#include <string>
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
        }

        Ollama(): Ollama("http://localhost:11434") {}
        ~Ollama() {}

    // Generate a non-streaming reply as a string.
    std::string generate(const std::string& model,const std::string& prompt, bool return_as_json=false)
    {
        std::string response="";

        // Generate the JSON request
        json request;
        request["model"] = model;
        request["prompt"] = prompt;
        request["stream"] = false;

        std::string request_string = request.dump();

        //std::cout << request_string << std::endl;      

        if (auto res = this->cli->Post("/api/generate",request_string, "application/json"))
        {
                if (return_as_json) response+=res->body;
                else
                {
                    json chunk = json::parse(res->body);        
                    response+=chunk["response"];
                }
        }

        return response;
    }

    // Generate a streaming reply where a user-defined callback function is invoked when each token is received.
    void generate(const std::string& model,const std::string& prompt, std::function<void(const std::string&)> on_receive_token)//, bool receive_json=false)
    {
        std::string response="";

        // Generate the JSON request
        json request;
        request["model"] = model;
        request["prompt"] = prompt;
        request["stream"] = true;

        std::string request_string = request.dump();

        std::cout << request_string << std::endl;

        auto stream_callback = [on_receive_token](const char *data, size_t data_length) {std::string token(data, data_length); on_receive_token(token);};

        httplib::Headers headers = { {"Content-Type","application/json"}};

        //this->cli->Post("/api/generate", headers, request_string, "application/json",[&](const char* data, size_t data_length) {return true;})

        //if (auto res =  ) //stream_callback))
        //{
        //   if (res->status==200)
        //        throw new std::runtime_error("Received HTTP error: "+res.error());
        //}

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

    inline bool is_running()
    {
        return ollama.is_running();
    }

    inline std::string get_version()
    {
        return ollama.get_version();
    }


}


#endif