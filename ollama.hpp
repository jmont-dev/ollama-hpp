#ifndef OLLAMA_HPP
#define OLLAMA_HPP

// Used for sending REST requests to the ollama server
#include "cpp-httplib/httplib.h"

//Used for formatting JSON with ollama
#include "json.hpp"

#include <string>
#include <iostream>

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

    std::string generate(const std::string& model,const std::string& prompt)
    {
        std::string response="";

        // Generate the JSON request
        json request;
        request["model"] = model;
        request["prompt"] = prompt;

        std::string request_string = request.dump();

        std::cout << request_string << std::endl;

        auto res = this->cli->Post("/api/generate",request_string, "application/json");

        if (res)
        {
            std::istringstream iss(res->body);

            std::string line;
            while(std::getline(iss,line))
            {            
                json chunk = json::parse(line);        
                response+=chunk["response"];
            }
        }

        return response;
    }

    bool is_running()
    {
        std::stringstream response;

        auto res = cli->Get("/");
        res->status;
        res->body;

        if (res)
        {
            response << res->body;
        }

        if (response.str()=="Ollama is running")
            return true;
        
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

    private:

    std::string server_url;
    httplib::Client *cli;

};

namespace ollama
{

}

#endif