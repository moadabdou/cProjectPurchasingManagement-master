#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "pages.h"  
#include "../tools/tools.h"
#include "../tools/errors.h"

#define HTML_FILE "./layouts/index.c"

void index_html(SOCKET client_socket) {
    File_prop html = read_file(HTML_FILE, "rb");  
    if (html.content == NULL) {
        SEND_ERROR_500;
        return;
    }

    char header[256*2];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", html.length);
    
    send(client_socket, header, strlen(header), 0);
    send(client_socket, html.content , html.length, 0);
    
    free(html.content);
}