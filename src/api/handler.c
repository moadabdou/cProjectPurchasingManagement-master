#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#include "api.h"
#include "../tools/errors.h"

#define ETUDIENT_QUERY "/etudient"

void handle_post(SOCKET client_socket, char *request) { //api handler 


    //getting the query of the api 
    char query[256] , prefix[] = "/api";
    int query_length = 0; 

    char *query_start_position = strstr(request , prefix);

    if (query_start_position == NULL ){
        printf("\n  from api handler :  error in getting query");
        SEND_ERROR_500;
        return;
    }

    query_start_position += + strlen(prefix);

    while(*(query_start_position + query_length) != ' ') query_length ++ ; 
    
    strncpy(query , query_start_position , query_length);
    query[query_length] = '\0';

    char *body = strstr(request, "\r\n\r\n");
    if (body != NULL) body += 4;  // Skip the \r\n\r\n to get to the actual body

    //getting the  api  function
    if (strncmp(query, ETUDIENT_QUERY , strlen(ETUDIENT_QUERY))== 0){
        handel_etudient_api(client_socket , query +  strlen(ETUDIENT_QUERY) , body); //as we found the query of  etudient we move the point  by litte so we cath only the following data
    }else  {
        SEND_ERROR_404_API;
    }
}