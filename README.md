# Request Organizer

Open source HTTP/s request organizer and collector. 
You can save your API REST/SOAP/XMLRPC/GRAPHQL requests per project and 
keep it clean an separated between logic flows.


## Features

- Organize requests in a hierarchical folder structure
- Visualize Request and Responses like in Burp Suite
- Compatible with Burp Suite 
- Import requests using cURL and XML
- Add and view screenshots on each request.

## TODO

- HTTP Fuzzer (Intruder like)
- Toggle raw view and prettify view on XML and JSON body contents
- Preview responses in embedded webview/custom chromium
- Export requests as cURL, ffuf, wfuzz, python requests, python aiohttp, C libcurl, etc

## Dependencies

- QT6
- Clang/GNU C
- Cmake

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Requirements

- CMake 3.16 or higher
- Qt6 (Core and Widgets components)
- C++17 compatible compiler

## Contribute

I don't need feauture contribution, but feel free to PR and open an issue
expalining why your feature is a good idea.
Keep it simple, don't add bloatware libraries, try to stick to the stdlib
qt and any well maintained framework/library that is cross-platform

## Bug report

Open an issue, I won't fix it right away but will look into it when
got some spare time.
