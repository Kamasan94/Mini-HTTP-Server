#include <unistd.h>

struct response {
    char protocol[10];
    u_int16_t status_code;
    char status_response[512];
    char headers[16000];
    char body[1000000]; //TODO TEMPORARY
};

//Response builder, it takse, protocol, status code, status response e the bodys
struct response* create_response(char* protocol, u_int16_t status_code, char* status_response, char* body){
    struct response* resp = malloc(sizeof(struct response));
    strcpy(resp->protocol,protocol);
    resp->status_code = status_code;
    strcpy(resp->status_response, status_response);
    strcpy(resp->body, body);
    return resp;
}

//This prepares the response to be sent as a string
int serialize_resp(struct response* resp, char* buf, int buf_size){
    snprintf(buf, buf_size,
        "%s %d %s\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s\n"
        "\r\n\r\n",
        resp->protocol, 
        resp->status_code, 
        resp->status_response, 
        strlen(resp->body),
        resp->body
    );
}
