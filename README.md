# ollama-hpp
Modern, Header-only C++ bindings for the Ollama API.

Access the full power of local language models in C++ with just a few lines of code:

```C++
#include "ollama.hpp"

std::cout << ollama::generate("llama3:8b", "Why is the sky blue?") << std::endl
```

## About this software:

    Ollama is a high-quality REST server and API providing an interface to run
    language models locally via llama.cpp.

    Ollama was made by Jeffrey Morgan (@jmorganca) and the Ollama team and is 
    available under the MIT License. To support this project or for more details
    go to https://github.com/ollama or https://ollama.com/. 

    This library is a header-only C++ integration of the Ollama API providing access
    to most API features while integrating them with std library classes or popular
    header-only libraries within the community. The following external libraries are 
    used:

    httplib is a header-only C++ http/https library.
    This library was created by Yuji Hirose and is available under the MIT License.
    For more details visit: https://github.com/yhirose/cpp-httplib

    nlohmnann JSON is a feature-rich header-only C++ JSON implementation.
    This library was created by Niels Lohmann and is available under the MIT License.
    For more details visit: https://github.com/nlohmann/json

    Base64.h is a header-only C++ library for encoding and decoding Base64 values.
    This library was created by tomykaira and is available under the MIT License.
    For more details visit: 
    https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594

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


# Modern, Header-only C++ bindings for the Ollama API

