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
    File_prop json_file;
    cJSON *client_data, *json_data;

    for (int i = 0;  i <  SESSIONS.max ;  i++){
        printf("\n session %d : %d " , i , SESSIONS.sessions[i]);
    }

    if (user_id == 0 || !check_in_sessions(SESSIONS, user_id)){
       SEND_ERROR_403;
       return; 
    }
    
    if(strncmp(query , NEW ,  strlen(NEW)) == 0){
        
        File_prop product_file = read_file(PRODUCTS_DATAFILE,"r");
        if(product_file.content==NULL){
            printf("unable to opern the files\n");
            SEND_ERROR_500;
            return;
        }

        cJSON *pro_json = cJSON_Parse(product_file.content);
        free(product_file.content);
        if(pro_json==NULL){
            printf("unable to parse the sales or the sales items\n");
            SEND_ERROR_500;
            return;
        }

        cJSON*newpro_json=cJSON_Parse(body);
        cJSON_AddNumberToObject(newpro_json,"id",generate_id());
        
        cJSON_AddItemToArray(pro_json,newpro_json);
        write_file(PRODUCTS_DATAFILE,cJSON_Print(pro_json),"w");

        char response[64];
        sprintf(response , "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}");
        send(client_socket, response, strlen(response), 0);

    }else{
        SEND_ERROR_404_API;
    }
}