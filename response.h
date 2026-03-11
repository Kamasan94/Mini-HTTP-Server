#ifndef __RESPONSE_H__
#define __RESPONSE_H__
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct response {
    char protocol[10];
    u_int16_t status_code;
    char status_response[512];
    char content_type[256];
    char headers[16000];
    char body[1000000]; //TODO TEMPORARY
};
typedef struct response response;

char *get_content_type(const char *file_extension) {
    if (strcmp(file_extension, "html") == 0) {
        return "text/html";
    } else if (strcmp(file_extension, "css") == 0) {
        return "text/css";
    } else if (strcmp(file_extension, "js") == 0) {
        return "application/javascript";
    } else if (strcmp(file_extension, "png") == 0) {
        return "image/png";
    } else if (strcmp(file_extension, "jpg") == 0 || strcmp(file_extension, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcmp(file_extension, "gif") == 0) {
        return "image/gif";
    } else if (strcmp(file_extension, "svg") == 0) {
        return "image/svg+xml";
    } else if (strcmp(file_extension, "json") == 0) {
        return "application/json";
    } else if (strcmp(file_extension, "pdf") == 0) {
        return "application/pdf";
    } else {
        return "application/octet-stream"; // Default binary type
    }
}

//Response builder, it takes: protocol, status code, status response e the bodys
struct response* create_response(const char* protocol, const u_int16_t status_code, const char* status_response, const char* body, const char* file_extension){
    struct response* resp = malloc(sizeof(struct response));
    strcpy(resp->protocol,protocol);
    resp->status_code = status_code;
    strcpy(resp->status_response, status_response);
    strcpy(resp->body, body);
    strcpy(resp->content_type, get_content_type(file_extension));
    return resp;
};


//This prepares the response to be sent as a string
int serialize_resp(struct response* resp, char* buf, int buf_size){
    snprintf(buf, buf_size,
        "%s %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n"
        "%s\n"
        "\r\n\r\n",
        resp->protocol, 
        resp->status_code, 
        resp->status_response, 
        resp->content_type,
        strlen(resp->body),
        resp->body
    );
}

#endif /* __RESPONSE_H__ */