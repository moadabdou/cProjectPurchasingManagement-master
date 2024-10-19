//errors
#define SEND_ERROR_404_API  char *error_message = "HTTP/1.1 404 udefined operation \r\nContent-Length: 0\r\n\r\n";\
                        send(client_socket, error_message, strlen(error_message), 0);\

#define SEND_ERROR_404  const char * response = "<div style='text-align:center'><h1>404 not found</h1>  wanna go <a href='/'>home<a/> ?</div>";\
                        char not_found[256] = {0};\
                        sprintf(not_found , "HTTP/1.1 404 not found \r\nContent-type: text/html \r\nContent-length: %d\r\n\r\n %s" , strlen(response) , response);\
                        send(client_socket, not_found, strlen(not_found), 0);

#define SEND_ERROR_500  char *error_message = "HTTP/1.1 500 \r\nContent-Length: 0\r\n\r\n";\
                        send(client_socket, error_message, strlen(error_message), 0);

