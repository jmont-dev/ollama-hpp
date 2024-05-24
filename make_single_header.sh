#!/bin/bash

# Remove the includes to other libraries in ollama.hpp
sed '27,34d' "include/ollama.hpp" > "no_includes_ollama.hpp"

# Combine the headers into one file
{ cat "include/json.hpp"; echo ""; cat "include/httplib.h"; echo ""; cat "include/Base64.h"; echo ""; cat "no_includes_ollama.hpp"; } > "singleheader/ollama.hpp"

# Clean up
rm "no_includes_ollama.hpp"

echo "Header-only library created in singleheader/ollama.hpp"
