
# CExpress

> A minimal HTTP server framework written in C — and *definitely not* inspired by Express.js. Pinky swear.


`CExpress` is a simple, lightweight HTTP server framework written in pure C. It lets you define routes using `GET()` and `POST()` handlers, parses basic HTTP requests, and responds with plain text — all while managing multiple clients with threads.

This is **not** Express.js, but if you squint hard enough, you might see the resemblance.

## Features

* Multi-threaded
* That's it I guess

##  Example Usage

```c
#include "express.h"

char* HOME(void* req) {
    return "Welcome to CExpress!";
}

char* Greet(void* req) {
    struct Http_parsed_object* request = (struct Http_parsed_object*) req;
    char* response = malloc(100);
    sprintf(response, "Hello, %s!", request->body);
    return response;
}

int main() {
    struct Router* router = Listen("8080");
    GET(router, "/", HOME);
    POST(router, "/greet", Greet);

    while (1) {
        sleep(1);
    }
}
```

Make a `POST` request with a body like `Mayank`, and get `Hello, Mayank!` in return. I bet you didn't expect this.

## Build & Run

```bash
gcc main.c -o server -lpthread
./server
```

Now go to `http://localhost:8080`.

##  Known Limitations

* No HTTP headers parsing
* No JSON support (Yo! it's C)
* Memory management is... let's say *relaxed*

## Why?

Because sometimes you want to suffer understand how the web works at a low level. Also because writing a web server in C makes you feel like you control the Matrix.
