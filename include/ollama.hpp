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
            message(const std::string& role, const std::string content, std::vector<ollama::image> images): role(role), content(content), images(images) {}
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


    class request: public json {

        public:

            enum type{ generation, embedding };

            // Create a request for a generation.
            request(const std::string& model,const std::string& prompt, const json& options=nullptr, bool stream=false, const std::vector<std::string>& images=std::vector<std::string>()): json()
            {   
                (*this)["model"] = model;
                (*this)["prompt"] = prompt;
                (*this)["stream"] = stream;

                if (options!=nullptr) (*this)["options"] = options["options"];
                if (!images.empty()) (*this)["images"] = images;

                type = type::generation;
            }
            // Create a request for a chat completion.
            request(const std::string& model,const std::string& prompt,std::vector<message> messages, const json& options=nullptr, bool stream=false): json()
            {
                (*this)["model"] = model;
                //(*this)["messages"] = messages;
                (*this)["stream"] = stream;
            }
            request(): json() {}
            ~request(){};

            static ollama::request embedding(const std::string& name, const std::string& prompt, const json& options=nullptr, const std::string& keep_alive_duration="5m")
            {
                ollama::request request;

                request["name"] = name;
                request["prompt"] = prompt;
                if (options!=nullptr) request["options"] = options["options"];
                request["keep_alive"] = keep_alive_duration;

                return request;
            }

        request::type type;
    };

    class response {

        public:

            enum response_type{ generation, embedding };

            response(const std::string& json_string)
            {
                this->json_string = json_string;
                try 
                {
                    json_data = json::parse(json_string); 
                    
                    if (type==generation) 
                        { if ( json_data.contains("response") ) simple_string=json_data["response"].get<std::string>(); }
                    else
                    if (type==embedding) 
                       { if ( json_data.contains("embedding") ) simple_string=json_data["embedding"].get<std::string>(); }                    
                    
                    if ( json_data.contains("error") ) error_string =json_data["error"].get<std::string>();
                }
                catch(...) { if (ollama::use_exceptions) throw new ollama::exception("Unable to parse JSON string:"+this->json_string); valid = false; }
            }
            
            response() {json_string = ""; valid = false;}
            ~response(){};

            bool is_valid() const {return valid;};

            const std::string& as_json_string() const
            {
                return json_string;
            }

            const json& as_json() const
            {
                return json_data;                
            }

            const std::string& as_simple_string() const
            {
                return simple_string;               
            }

            const bool has_error() const
            {
                if ( json_data.contains("error") ) return true;
                return false;                
            }

            const std::string& get_error() const
            {
                return error_string;
            }

            friend std::ostream& operator<<(std::ostream& os, const ollama::response& response) { os << response.as_simple_string(); return os; }

        private:

        std::string json_string;
        std::string simple_string;
        std::string error_string;

        json json_data;        
        response_type type;
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
    ollama::response generate(const std::string& model,const std::string& prompt, json options=nullptr, const std::vector<std::string>& images=std::vector<std::string>())
    {

        ollama::response response;
        ollama::request request(model, prompt, options, false, images);

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;      

        if (auto res = this->cli->Post("/api/generate",request_string, "application/json"))
        {
            if (ollama::log_replies) std::cout << res->body << std::endl;

            response = ollama::response(res->body);
            if ( response.has_error() ) { if (ollama::use_exceptions) throw ollama::exception("Ollama response returned error: "+response.get_error() ); }
           
        }
        else
        {
            if (ollama::use_exceptions) throw ollama::exception("No response returned from server "+this->server_url+". Error was: "+httplib::to_string( res.error() ));
        }

        return response;
    }

    // Generate a streaming reply where a user-defined callback function is invoked when each token is received.
    bool generate(const std::string& model,const std::string& prompt, std::function<void(const ollama::response&)> on_receive_token, json options=nullptr, const std::vector<std::string>& images=std::vector<std::string>())
    {

        ollama::request request(model, prompt, options, true, images);

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;

        auto stream_callback = [on_receive_token](const char *data, size_t data_length)->bool{
            
            std::string message(data, data_length);
            if (ollama::log_replies) std::cout << message << std::endl;
            ollama::response response(message);
            on_receive_token(response);

            return true;
        };

        if (auto res = this->cli->Post("/api/generate", request_string, "application/json", stream_callback)) { return true; }
        else { if (ollama::use_exceptions) throw ollama::exception( "No response from server returned at URL"+this->server_url+" Error: "+httplib::to_string( res.error() ) ); } 

        return false;
    }

    bool create_model(const std::string& modelName, const std::string& modelFile, bool loadFromFile=true)
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



    json list_model_json()
    {
        json models;
        if (auto res = cli->Get("/api/tags"))
        {
            if (ollama::log_replies) std::cout << res->body << std::endl;
            models = json::parse(res->body);
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when querying model list: "+httplib::to_string( res.error() ) );}        

        return models;
    }

    std::vector<std::string> list_models()
    {
        std::vector<std::string> models;

        json json_response = list_model_json();
        
        for (auto& model: json_response["models"])
        {
            models.push_back(model["name"]);
        }

        return models;
    }

    bool blob_exists(const std::string& digest)
    {
        if (auto res = cli->Head("/api/blobs/"+digest))
        {
            if (res->status==httplib::StatusCode::OK_200) return true;
            if (res->status==httplib::StatusCode::NotFound_404) return false;            
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when checking if blob exists: "+httplib::to_string( res.error() ) );}        

        return false;
    }

    bool create_blob(const std::string& digest)
    {
        if (auto res = cli->Post("/api/blobs/"+digest))
        {
            if (res->status==httplib::StatusCode::Created_201) return true;
            if (res->status==httplib::StatusCode::BadRequest_400) { if (ollama::use_exceptions) throw ollama::exception("Received bad request (Code 400) from Ollama server when creating blob."); }            
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when creating blob: "+httplib::to_string( res.error() ) );}        

        return false;

    }

    json show_model_info(const std::string& model)
    {
        json request, response;
        request["name"] = model;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;

        if (auto res = cli->Post("/api/show", request_string, "application/json"))
        {
            if (ollama::log_replies) std::cout << "Reply was " << res->body << std::endl;
            try
            { 
                response = json::parse(res->body); 
            }
            catch(...)
            { if (ollama::use_exceptions) throw ollama::exception("Received bad response from Ollama server when querying model info."); }           
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when querying model info: "+httplib::to_string( res.error() ) );}        

        return response;
    }

    bool copy_model(const std::string& source_model, const std::string& dest_model)
    {
        json request;
        request["source"] = source_model;
        request["destination"] = dest_model;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;
        
        if (auto res = cli->Post("/api/copy", request_string, "application/json"))
        {
            if (res->status==httplib::StatusCode::OK_200) return true;
            if (res->status==httplib::StatusCode::NotFound_404) { if (ollama::use_exceptions) throw ollama::exception("Source model not found when copying model (Code 404)."); }            
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when copying model: "+httplib::to_string( res.error() ) );}        

        return false;
    }

    bool delete_model(const std::string& model)
    {
        json request;
        request["name"] = model;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;
        
        if (auto res = cli->Delete("/api/delete", request_string, "application/json"))
        {
            if (res->status==httplib::StatusCode::OK_200) return true;
            if (res->status==httplib::StatusCode::NotFound_404) { if (ollama::use_exceptions) throw ollama::exception("Model not found when trying to delete (Code 404)."); }            
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when deleting model: "+httplib::to_string( res.error() ) );}        

        return false;
    }

    bool pull_model(const std::string& model, bool allow_insecure = false)
    {
        json request, response;
        request["name"] = model;
        request["insecure"] = allow_insecure;
        request["stream"] = false;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;
        
        if (auto res = cli->Post("/api/pull", request_string, "application/json"))
        {
            if (res->status==httplib::StatusCode::OK_200) return true;
            if (res->status==httplib::StatusCode::NotFound_404) { if (ollama::use_exceptions) throw ollama::exception("Model not found when trying to pull (Code 404)."); return false; }

            response = json::parse(res->body);
            if ( response.contains("error") ) { if (ollama::use_exceptions) throw ollama::exception( "Error returned from ollama when pulling model: "+response["error"].get<std::string>() ); return false; }          
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when pulling model: "+httplib::to_string( res.error() ) );}        

        return false;
    }

    bool push_model(const std::string& model, bool allow_insecure = false)
    {
        json request, response;
        request["name"] = model;
        request["insecure"] = allow_insecure;
        request["stream"] = false;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;
        
        if (auto res = cli->Post("/api/push", request_string, "application/json"))
        {
            if (res->status==httplib::StatusCode::OK_200) return true;
            if (res->status==httplib::StatusCode::NotFound_404) { if (ollama::use_exceptions) throw ollama::exception("Model not found when trying to push (Code 404)."); return false; }

            response = json::parse(res->body);
            if ( response.contains("error") ) { if (ollama::use_exceptions) throw ollama::exception( "Error returned from ollama when pushing model: "+response["error"].get<std::string>() ); return false; }          
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when pushing model: "+httplib::to_string( res.error() ) );}        

        return false;
    }

    ollama::response generate_embeddings(const std::string& model, const std::string& prompt, json options=nullptr, const std::string& keep_alive_duration="5m")
    {
        ollama::request request = ollama::request::embedding(model, prompt, options, keep_alive_duration);
        ollama::response response;

        std::string request_string = request.dump();
        if (ollama::log_requests) std::cout << request_string << std::endl;
        
        if (auto res = cli->Post("/api/embeddings", request_string, "application/json"))
        {
            if (ollama::log_replies) std::cout << res->body << std::endl;


            if (res->status==httplib::StatusCode::OK_200) {response = ollama::response(res->body); return response; };
            if (res->status==httplib::StatusCode::NotFound_404) { if (ollama::use_exceptions) throw ollama::exception("Model not found when trying to push (Code 404)."); }

            if ( response.has_error() ) { if (ollama::use_exceptions) throw ollama::exception( "Error returned from ollama when generating embeddings: "+response.get_error() ); }          
        }
        else { if (ollama::use_exceptions) throw ollama::exception("No response returned from server when pushing model: "+httplib::to_string( res.error() ) );}        

        return response;
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

    inline std::vector<std::string> list_models()
    {
        return ollama.list_models();
    }

    inline json list_model_json()
    {
        return ollama.list_model_json();
    }

    inline bool blob_exists(const std::string& digest)
    {
        return ollama.blob_exists(digest);
    }

    inline bool create_blob(const std::string& digest)
    {
        return ollama.create_blob(digest);
    }

    inline json show_model_info(const std::string& model)
    {
        return ollama.show_model_info(model);
    }

    inline bool copy_model(const std::string& source_model, const std::string& dest_model)
    {
        return ollama.copy_model(source_model, dest_model);
    }

    inline bool delete_model(const std::string& model)
    {
        return ollama.delete_model(model);
    }

    inline bool pull_model(const std::string& model, bool allow_insecure = false)
    {
        return ollama.pull_model(model, allow_insecure);
    }

    inline bool push_model(const std::string& model, bool allow_insecure = false)
    {
        return ollama.push_model(model, allow_insecure);
    }

    inline bool generate_embeddings(const std::string& model, const std::string& prompt, json options=nullptr, const std::string& keep_alive_duration="5m")
    {
        return ollama.generate_embeddings(model, prompt, options, keep_alive_duration);
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