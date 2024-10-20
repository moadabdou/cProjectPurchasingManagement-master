#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#include "../tools/tools.h"
#include "api.h"
#include "../tools/errors.h"

#define EMPLOYEE_QUERY "/employees"

void handle_post(SOCKET client_socket, char *request,  Sessions SESSIONS) { //api handler 


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
    if (strncmp(query, EMPLOYEE_QUERY , strlen(EMPLOYEE_QUERY))== 0){
        handel_employee_api(client_socket , query +  strlen(EMPLOYEE_QUERY) , body, SESSIONS); //as we found the query of  etudient we move the point  by litte so we cath only the following data
    }else  {
        SEND_ERROR_404_API;
    }
}