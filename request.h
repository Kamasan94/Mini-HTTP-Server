#define ROOTDIR "./src"

//STRUCTURES
//This defines the struct for the first line of the request
struct request {
    char method[7];
    char path[2048];
    char protocol[10];
    char headers[16000];
};

//Check if the last row is \r\n\r\n in header parsing
int check_last_row(char* p){
    if(*p == '\r' && *(p+1) == '\n' && *(p+2) == '\r' && *(p+3) == '\n') return 1;
    else return 0;
}

//Takes the buffer at the beginning of the request headers and copies the header in the struct
void parse_header(struct request* req, char* buf){
    char *i = buf;
    int count = 0;
    while(check_last_row(i) != 1){
        count += 1;
        i += 1;
    }
    strncpy(req->headers, buf, count);
}

//Print the request
void print_request(struct request* req){
    printf("%s %s %s\n%s\n", req->method, req->path, req->protocol, req->headers);
}

//Returns the correct path to the file
char* parse_path(char* path){
    if(strcmp(path, "/") == 0){
        char*p = malloc(sizeof("./src/index.html"));
        strcpy(p, "./src/index.html");
        return p;
    }
    else {
        char*p = malloc(sizeof(path) + sizeof(ROOTDIR));
        strcpy(p, ROOTDIR);
        strcat(p, path);
        return p;
    }
}

//It takes the buffer with the whole request and puts the desired data in the request structure
struct request* parse_request(struct request* req, char* buf){ 
    memset(req, 0, sizeof(struct request));
    char* p = buf;
    int offset = 0, count = 0;
    while(*p != '\n') {
        while(*p != ' ' && *p != '\t' && *p != '\r'){
            p += 1;
            count += 1;
        }
        strncpy(req->method, buf, count);
        offset += count + 1;
        count = 0;
        p += 1;
        while(*p != ' ' && *p != '\t' && *p != '\r'){
            p += 1;
            count += 1;
        }
        strncpy(req->path, buf+offset, count);
        offset += count + 1;
        count = 0;
        p += 1;
        while(*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n'){
            p += 1;
            count += 1;
        }
        strncpy(req->protocol, buf+offset, count);
        p += 1;
    }

    parse_header(req, p + 1);

    return req;
}