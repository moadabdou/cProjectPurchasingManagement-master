#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#include "../tools/tools.h"
#include "assets.h"
#include "../tools/errors.h"

void handle_images(SOCKET client_socket, char *request){
    //getting the query of the image
    char query[256] , prefix[] = "/images/";
    int query_length = 0; 

    char *query_start_position = strstr(request , prefix);

    if (query_start_position == NULL ){
        printf("\n  from images handler :  error in getting query");
        SEND_ERROR_500;
        return;
    }
    query_start_position += 1 ; //skip  first the /
    while(*(query_start_position + query_length) != ' ') query_length ++ ; 
    
    strncpy(query , query_start_position , query_length);
    query[query_length] = '\0';

    File_prop image = read_file(query , "rb");

    if (image.content == NULL) {
        printf("from  image handler : cant open  image or does not  exist ");
        SEND_ERROR_404;
        return;
    }

    char header[256];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\n\r\n", image.length);

    send(client_socket, header, strlen(header), 0);
    send(client_socket , image.content , image.length, 0);

    free(image.content);
}