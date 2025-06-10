#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


struct Http_parsed_object PARSE_REQUEST(char buffer[1024]){
    char first_line[512];
    char* line_end = strstr(buffer, "\r\n");
    strncpy(first_line, buffer, line_end - buffer);
    struct Http_parsed_object res;
    char** resArr = malloc(sizeof(char*)*2);
    int prev = 0;
    int len = 0;
    for (int i = 0; i < strlen(first_line); i++) {
        if (first_line[i] == ' ') {
            resArr[len] = malloc(sizeof(char)*100);
            strncpy(resArr[len], first_line + prev, i - prev);
            resArr[len][i-prev] = '\0';
            prev = i+1;
            len++;
        }
    }
    res.method = malloc(strlen(resArr[0]) + 1);
    res.path = malloc(strlen(resArr[1]) + 1);
    strcpy(res.method, resArr[0]);
    strcpy(res.path, resArr[1]);
    if (strcmp(res.method, "POST") == 0){
        char* body = strstr(buffer, "\r\n\r\n");
        if(body){
            body += 4;
            res.body = body;
        }
        else{
            res.body = NULL;
        }
    }
    free(resArr[1]);
    free(resArr[0]);
    free(resArr);
    return res;
}


void* HANDLE_REQUEST(void* socket_duel_ptr){
    printf("HANDLING REQUEST\n");

    struct socket_duel socket_duel = *(struct socket_duel*) socket_duel_ptr;
    struct Router* router = socket_duel.host; 
    int clientSocket = socket_duel.clientSocket;

    // free(router);
    int const buffer_size = 1024;
    char buffer[buffer_size];

    int bytes_read = recv(clientSocket, &buffer, sizeof(buffer), 0);

    if (bytes_read <= 0){
        close(clientSocket);
        return NULL;
    }
    buffer[bytes_read] = '\0';
    struct Http_parsed_object* res = malloc(sizeof(struct Http_parsed_object)) ;
    *res = PARSE_REQUEST(buffer);
    int found = 0;
    for (int i = 0; i < router->taskLen && found == 0 ; i++ ) {
        struct task task = *router->tasks[i];
        if (strcmp(res->method, task.method) == 0 && strcmp(res->path, task.url) == 0){
            typedef char* (*callback)(void* request);
            callback func = (callback)task.callback; 
            char* response_body = func(res);
            char* response = malloc(1024);
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n%s", response_body);
            printf("%s\n", response);
            send(clientSocket, response, strlen(response), 0);
            close(clientSocket);
            free(response);
            found = 1;
        }
    }
    if (!found){
        send(clientSocket, "404 not found", strlen("404 not found"), 0);
        close(clientSocket);
    }
}

void* ACCEPT_REQUESTS(void* router){
    printf("Accepting requests...\n");
    int sockfd_int = ((struct Router*) router)->sockfd;
    while(1){
        struct sockaddr_storage thier_addr;
        socklen_t thier_addr_len = sizeof(thier_addr);
        int socket = accept(sockfd_int, (struct sockaddr*) &thier_addr, &thier_addr_len);

        struct socket_duel* newSocketDuel = malloc(sizeof(struct socket_duel));
        newSocketDuel->clientSocket = socket;
        newSocketDuel->host = router;

        if(socket < 0){
            printf("Error while accepting!");
        }
        printf("Accepted!\n");
        pthread_t thread;
        pthread_create(&thread, NULL, HANDLE_REQUEST, newSocketDuel);
        pthread_detach(thread);
    }
}

struct Router* Listen(char* port){
    struct addrinfo hint, *res, *p;

    memset(&hint, 0, sizeof(struct addrinfo));

    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    if(getaddrinfo(NULL, port, &hint, &res) != 0){
        printf("Error while getting addrinfo\n");
    }

    int sockfd;
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if( sockfd < 0){
            continue;
        };

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        const int bindResult = bind(sockfd, p->ai_addr, p->ai_addrlen);
        if(bindResult < 0){
            printf("Error while binding socket!\n");
            continue;
        }
    }
    listen(sockfd, 10);
    pthread_t thread;
    struct Router* router = malloc(sizeof(struct Router));
    router->taskLen = 0;
    router->sockfd = sockfd;
    pthread_create(&thread, NULL, ACCEPT_REQUESTS, router);
    return router;
}

void GET(struct Router* router, char* path, void *callback){
    struct task* task = malloc(sizeof(struct task));
    task->sockfd = router->sockfd;
    task->method = "GET";
    task->url = path;
    task->callback = callback;
    router->tasks[router->taskLen] = task;
    router->taskLen++;
}
void POST(struct Router* router, char* path, void *callback){
    struct task* task = malloc(sizeof(struct task));
    task->sockfd = router->sockfd;
    task->method = "POST";
    task->url = path;
    task->callback = callback;
    router->tasks[router->taskLen] = task;
    router->taskLen++;
}
