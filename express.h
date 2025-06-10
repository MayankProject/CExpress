

struct Router{
    int sockfd;
    struct task* tasks[100];
    int taskLen;
};

struct task {
    int sockfd;
    char* method;
    char* url;
    void* callback;
};

struct Http_parsed_object {
    char* method;
    char* path;
    char* body;
};

struct socket_duel {
    struct Router* host;
    int clientSocket;
};

struct Router* Listen(char* port);
void GET(struct Router* router, char* path, void *callback);
void POST(struct Router* router, char* path, void *callback);
