#include "express.h"
#include "express.c"
#include <stdio.h>

char* HOME(void* r_request){
    return "HOME PAGE";
}


char* ABOUT(void* r_request){
    return "ABOUT PAGE";
}

// Takes only name in the whole body ( plain text )
char* Greet(void* r_request){
    struct Http_parsed_object* request = (struct Http_parsed_object*) r_request;
    printf("body: %s\n", request->body);
    char* response = malloc(100);
    sprintf(response, "Hello, %s!", request->body);
    return response;
}

int main(){
    struct Router* router = Listen("8080");
    struct Router* router2 = Listen("8000");

    GET(router, "/", HOME);
    GET(router2, "/about", ABOUT);
    POST(router2, "/greet", Greet);

    while(1){
        sleep(1);
    }
}
