# ollama-hpp
Modern, Header-only C++11/14/20 bindings for the [Ollama](https://ollama.ai) API.

Access the full power of local language models in C++ with just a few lines of code:

```C++
#include "ollama.hpp"

std::cout << ollama::generate("llama3:8b", "Why is the sky blue?") << std::endl;
```

## Quick Start
Download the header in singleheader/ollama.hpp and include it with your project to get started. No additional files or linking are needed. C++11 is the minimum language specification and C++14/20 are also supported.

For example:
`g++ your_source_file.cpp -Iollama-hpp/singleheader -std=c++11`

Ensure you have an active instance of the ollama server and any models you plan to run. For more details, see https://ollama.com. You can check if your ollama server is running using: `sudo systemctl status ollama`

## Building examples
To build the ollama-hpp examples and test cases, use:
`make`

To run the examples and test cases, use:

`build/test` <br>
`build/examples`

## Full API

The test cases do a good job of providing discrete examples for each of the API features supported. I recommend reviewing these first in `test/test.cpp` to understand what the library and Ollama API provide.

- [Full API](#full-api)
  - [Server Status](#server-status)
  - [Version](#version)


### Ollama Class and Singleton
The `Ollama` class defines the logic required to interface and make calls to an ollama server.

```C++
Ollama my_server("http://localhost:11434");
std::cout << my_server.generate("llama3:8b", "Why is the sky blue?") << std::endl;
```

For convenience, a static singleton of the Ollama class is included in the `ollama` namspace which defaults to http://localhost:11434. Using the static singleton is preferred and will be easier for most people. This allows you to make calls to a default server immediately simply by including the file. All calls to the ollama server are also valid for the singleton:

```C++
// No object creation required; the static singleton is usable as soon as the file is included, and points to http://localhost:11434.
ollama::generate("llama3:8b", "Why is the sky blue?") << std::endl;
```

### Server Defaults
The `Ollama` object contains a series of intelligent defaults used to communicate with an ollama server. You will not typically have to change these, but can do so if required:

```C++
// Optional. By default, the server URL is set to http://localhost:11434. Use this function if you need to point to a different URL.
ollama::setServerURL("http://localhost:11434");    

// Optional. Set the read and write timeouts in seconds for receiving from and sending data to ollama.
// If you have a large model with a long response time you may need to increase these.
ollama::setReadTimeout(120);
ollama::setWriteTimeout(120);
```

### Server Status
Verify that the Ollama server is running with `ollama::is_running()`

```C++
bool running = ollama::is_running();
```

### Version
Return a std::string containing the version of the ollama server using `ollama::get_version()`

```C++
std::string version = ollama::get_version();
```

## Single-header vs Separate Headers
For convenience, ollama-hpp includes a single-header version of the library in `singleheader/ollama.hpp` which bundles the core ollama.hpp code with single-header versions of nlohmann json, httplib, and base64.h. Each of these libraries is available under the MIT license and their respective licenses are included.
The single-header include can be regenerated from these standalone files by running `./make_single_header.sh`

If you prefer to include the headers for these libraries separately, you can do so by including the standard header located in `include/ollama.hpp`. 

## About this software:

Ollama is a high-quality REST server and API providing an interface to run language models locally via llama.cpp.

Ollama was made by Jeffrey Morgan (@jmorganca) and the Ollama team and is available under the MIT License. To support this project or for more details go to https://github.com/ollama or https://ollama.ai 

This library is a header-only C++ integration of the Ollama API providing access to most API features while integrating them with std library classes or popular header-only libraries within the community. The following external libraries are used:

* **nlohmnann JSON** is a feature-rich header-only C++ JSON implementation. This library was created by Niels Lohmann and is available under the MIT License. For more details visit: https://github.com/nlohmann/json

* **httplib** is a header-only C++ http/https library. This library was created by Yuji Hirose and is available under the MIT License. For more details visit: https://github.com/yhirose/cpp-httplib

* **Base64.h** is a header-only C++ library for encoding and decoding Base64 values. This library was created by tomykaira and is available under the MIT License. For more details visit: https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594

## License
MIT License

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

