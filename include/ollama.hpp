#ifndef OLLAMA_HPP
#define OLLAMA_HPP

/*  MIT License

    Copyright (c) 2024 James Montgomery (jmont)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

// Used for sending REST requests to the ollama server
#include "httplib.h"

//Used for formatting JSON with ollama
#include "json.hpp"

//Used for Base64 formatting of images
#include "Base64.h"

#include <string>
#include <fstream>
#include <iostream>
#include <functional>
#include <exception>
#include <initializer_list>

using json = nlohmann::json;
using base64 = macaron::Base64;

// Namespace types and classes
namespace ollama
{    
    static bool use_exceptions = true;    // Change this to false to avoid throwing exceptions within the library.    
    static bool log_requests = false;      // Log raw requests to the Ollama server. Useful when debugging.       
    static bool log_replies = false;       // Log raw replies from the Ollama server. Useful when debugging.

    static void allow_exceptions(bool enable) {use_exceptions = enable;}
    static void show_requests(bool enable) {log_requests = enable;}
    static void show_replies(bool enable) {log_replies = enable;}

    class exception : public std::exception {
    private:
        std::string message;

    public:
        exception(const std::string& msg) : message(msg) {}
        const char* what() const noexcept override { return message.c_str(); }
    };

    class image {
        public:
            image(const std::string& filepath, bool loadFromFile=true)
            {
                std::ifstream file(filepath, std::ios::binary);
                if (!file) {
                    if (ollama::use_exceptions) throw ollama::exception("Unable to open image file from path.");
                    valid = false; return;
                }

                std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

                this->base64_sequence = macaron::Base64::Encode(file_contents);
                valid = true;

            }
            image(const std::string base64_sequence) 
            {
                this->base64_sequence = base64_sequence;
            }
            ~image(){};

            const std::string as_base64_string() const
            {
                return base64_sequence;
            }

            bool is_valid(){return valid;}

            operator std::string() const {
                    return base64_sequence;
    }

        private:
            std::string base64_sequence;
            bool valid;
    };

    class images: public std::vector<std::string> {

        public:
            images(): std::vector<std::string>(0)
            {

            }
            images(std::initializer_list<ollama::image> list) {
                for (ollama::image value : list) {
                    this->push_back(value);
                }
            }        
            ~images(){};
            std::vector<std::string> to_strings()
            {
                std::vector<std::string> strings;
                for (auto it = this->begin(); it != this->end(); ++it)
                    strings.push_back(*it);
                
                return strings;
            }

    };

    class message {
        public:
            message(const std::string& role, const std::string content, std::vector<ollama::image> images): role(role), content(content), images(images)
            {
                
            }
            ~message(){};

            std::string as_json_string() const
            {
                std::string json_string;

                return json_string;
            }
        private:

        std::string role, content;
        std::vector<ollama::image> images;

    };


    class request {

        public:
            // Create a request for a generation.
            request(const std::string& model,const std::string& prompt, bool stream=false, const json options=nullptr,const std::vector<std::string> images=std::vector<std::string>())
            {   
                json_request["model"] = model;
                json_request["prompt"] = prompt;
                json_request["options"] = options;
                json_request["stream"] = stream;
                json_request["images"] = images;
            }
            // Create a request for a chat completion.
            request(const std::string& model,const std::string& prompt,std::vector<message> messages, bool stream=false, const json options=nullptr)
            {
                json_request["model"] = model;
                //json_request["messages"] = messages;
                json_request["stream"] = stream;
            }
            ~request(){};

        json json_request;
    };

    class response {

        public:

            response(const std::string& json_string)
            {
                this->json_string = json_string;
                valid = parseLinesToJSON();
            }
            
            ~response(){};

            bool is_valid() const {return valid;};

            const std::string as_json_string() const
            {
                return json_string;
            }

            const std::vector<json> as_json() const
            {
                return json_entries;                
            }

            const std::string as_simple_string() const
            {
                std::string response;
                
                for (auto&& json_entry: json_entries)
                {
                    response+=json_entry["response"];
                }

                return response;                
            }

        private:

        bool parseLinesToJSON()
        {
            try
            {
                std::istringstream iss(this->json_string);
                std::string line;

                while (std::getline(iss, line))
                {
                    json message = json::parse(line);
                    simple_string+=message["response"];        
                    json_entries.push_back(message);
                }
            }
            catch (...) { json_entries.clear(); if (ollama::use_exceptions) throw new ollama::exception("Unable to parse JSON string:"+this->json_string); return false; }

            return true;
        }

        //Optimize by caching values if they have not changed
        std::string json_string;
        std::string simple_string;
        std::vector<json> json_entries;
        bool valid;

    };

}

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
    std::string generate(const std::string& model,const std::string& prompt, json options=nullptr, std::vector<std::string> images=std::vector<std::string>(), bool return_as_json=false)
    {

        std::string response="";

        // Generate the JSON request
        json request;
        request["model"] = model;
        request["prompt"] = prompt;
        if (options!=nullptr) request["options"] = options["options"];
        request["stream"] = false;
        if (!images.empty()) request["images"] = images;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;      

        if (auto res = this->cli->Post("/api/generate",request_string, "application/json"))
        {
            if (ollama::log_replies) std::cout << res->body << std::endl;

                if (return_as_json) response+=res->body;
                else
                {
                    json chunk = json::parse(res->body);        
                    response+=chunk["response"];
                }
        }
        else
        {
            if (ollama::use_exceptions) throw ollama::exception("No response returned from server "+this->server_url+". Error was: "+httplib::to_string( res.error() ));
            return "";
        }

        return response;
    }


    // Generate a streaming reply where a user-defined callback function is invoked when each token is received.
    bool generate(const std::string& model,const std::string& prompt, std::function<void(const std::string&, bool)> on_receive_token, json options=nullptr, bool receive_json=false)
    {
        std::string response="";

        // Generate the JSON request
        json request;
        request["model"] = model;
        request["prompt"] = prompt;
        if (options!=nullptr) request["options"] = options["options"];
        request["stream"] = true;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;

        auto stream_callback = [on_receive_token, receive_json](const char *data, size_t data_length)->bool{
            
            std::string message(data, data_length);
            json chunk = json::parse(message);        
            
            std::string response=chunk["response"];
            bool done = chunk["done"];

            // Either pass back the entire json string or the parsed token.
            on_receive_token( (receive_json ? message: response), done);

            return true;

        };

        if (auto res = this->cli->Post("/api/generate", request_string, "application/json", stream_callback)) { return true; }
        else { if (ollama::use_exceptions) throw ollama::exception( "No response returned from server "+this->server_url+". Error was: "+httplib::to_string( res.error() ) ); }

        return false;
    }

    // Generate a streaming reply where a user-defined callback function is invoked when each token is received.
    bool generate(const std::string& model,const std::string& prompt, std::function<void(const ollama::response&)> on_receive_token, json options=nullptr)
    {
        std::string response="";

        // Generate the JSON request
        json request;
        request["model"] = model;
        request["prompt"] = prompt;
        if (options!=nullptr) request["options"] = options["options"];
        request["stream"] = true;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;

        auto stream_callback = [on_receive_token](const char *data, size_t data_length)->bool{
            
            std::string message(data, data_length);
            ollama::response response(message);
            on_receive_token(response);

            return true;

        };

        if (auto res = this->cli->Post("/api/generate", request_string, "application/json", stream_callback)) { return true; }
        else { if (ollama::use_exceptions) throw ollama::exception( "No response from server returned at URL"+this->server_url+" Error: "+httplib::to_string( res.error() ) ); } 

        return false;
    }

    bool create(const std::string& modelName, const std::string& modelFile, bool loadFromFile=true)
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
                if (ollama::use_exceptions) throw ollama::exception("Failed to open file "+modelFile);
                return false;
            }

            // Read the entire file into a string using iterators
            std::string file_contents((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
            
            request["modelFile"] = file_contents;                                                
        }
        else request["modelFile"] = modelFile;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;  

        std::string response;

        if (auto res = this->cli->Post("/api/create",request_string, "application/json"))
        {
            if (ollama::log_replies) std::cout << res->body << std::endl;

            json chunk = json::parse(res->body);
            if (chunk["status"]=="success") return true;        
        }
        else
        {
            if (ollama::use_exceptions) throw ollama::exception("No response returned: "+httplib::to_string( res.error() ) );
        }

        return false;

    }

    bool load_model(const std::string& model)
    {
        json request;
        request["model"] = model;
        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;

        // Send a blank request with the model name to instruct ollama to load the model into memory.
        if (auto res = this->cli->Post("/api/generate", request_string, "application/json"))
        {
            if (ollama::log_replies) std::cout << res->body << std::endl;
            json response = json::parse(res->body);
            return response["done"];        
        }
        else
        { 
            if (ollama::use_exceptions) throw ollama::exception("No response returned from server when loading model: "+httplib::to_string( res.error() ) ); 
        }

        // If we didn't get a response from the server indicating the model was created, return false.        
        return false;                
    }

    bool is_running()
    {
        auto res = cli->Get("/");
        if (res) if (res->body=="Ollama is running") return true;
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
            throw ollama::exception("Error retrieving version: "+res->status);
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

    bool send_request(const ollama::request& request, std::function<void(const ollama::response&)> on_receive_response=nullptr)
    {

        return true;
    }

    std::string server_url;
    httplib::Client *cli;

};

// Functions associated with Ollama singleton
namespace ollama
{
    // Use directly from the namespace as a singleton
    static Ollama ollama;
    
    inline void setServerURL(const std::string& server_url)
    {
        ollama.setServerURL(server_url);
    }

    inline std::string generate(const std::string& model,const std::string& prompt, json options=nullptr, std::vector<std::string> images=std::vector<std::string>(), bool return_as_json=false)
    {
        return ollama.generate(model, prompt, options, images, return_as_json);
    }

    inline bool generate(const std::string& model,const std::string& prompt, std::function<void(const std::string&, bool)> on_receive_token, json options=nullptr, bool return_as_json=false)
    {
        return ollama.generate(model, prompt, on_receive_token, options, return_as_json);
    }

    inline bool generate(const std::string& model,const std::string& prompt, std::function<void(const ollama::response&)> on_receive_response, json options=nullptr)
    {
        return ollama.generate(model, prompt, on_receive_response, options);
    }

    inline bool create(const std::string& modelName, const std::string& modelFile, bool loadFromFile=true)
    {
        return ollama.create(modelName, modelFile, loadFromFile);
    }

    inline bool is_running()
    {
        return ollama.is_running();
    }

    inline bool load_model(const std::string& model)
    {
        return ollama.load_model(model);
    }

    inline std::string get_version()
    {
        return ollama.get_version();
    }

    inline void setReadTimeout(const int& seconds)
    {
        ollama.setReadTimeout(seconds);
    }

    inline void setWriteTimeout(const int& seconds)
    {
        ollama.setWriteTimeout(seconds);
    }

};


#endif