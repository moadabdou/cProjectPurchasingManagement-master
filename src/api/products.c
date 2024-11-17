#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/cJSON.h"
#include "../pages/pages.h"
#include "../data_vars.h"

#define NEW  "/new"


void handel_products_api(SOCKET client_socket, char *query , char *body, Sessions SESSIONS, int user_id){

    cJSON *client_data, *json_data;

    for (int i = 0;  i <  SESSIONS.max ;  i++){
        printf("\n session %d : %d " , i , SESSIONS.sessions[i]);
    }

    if (user_id == 0 || !check_in_sessions(SESSIONS, user_id)){
       SEND_ERROR_403;
       return; 
    }
    
    if(strncmp(query , NEW ,  strlen(NEW)) == 0){
        
        client_data = cJSON_Parse(body);
        json_data = load_json_from_file(PRODUCTS_DATAFILE);
        if (json_data ==  NULL || client_data == NULL){
            SEND_ERROR_500;
            return ;
        }


        cJSON_AddNumberToObject(client_data,"id",generate_id());
        
        cJSON_AddItemToArray(json_data,client_data);
        write_file(PRODUCTS_DATAFILE,cJSON_Print(json_data),"w");

        char response[64];
        sprintf(response , "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}");
        send(client_socket, response, strlen(response), 0);

    }else{
        SEND_ERROR_404_API;
    }
}