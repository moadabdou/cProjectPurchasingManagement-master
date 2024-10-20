#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/cJSON.h"


#define DATAFILE "./data/employees.json"
#define GET  "/get"
#define AUTH  "/auth"
#define NEW  "/new"
#define DELET "/delete" //delete  already  used 

char  check_email_validity(cJSON *json_data , cJSON *client_json){
    
    cJSON *client_email = cJSON_GetObjectItem(client_json , "email");

    if (client_email == NULL ) {
        return 0;
    }

    for (int i= 0; i <  cJSON_GetArraySize(json_data); i++){
            cJSON *item =  cJSON_GetArrayItem(json_data , i);
            if  (item !=  NULL){
                cJSON *item_email = cJSON_GetObjectItem(item , "email");
                if (item_email != NULL){
                    if (strcmp(item_email->valuestring , client_email->valuestring) == 0){
                        return 0;
                    }
                }
            }
        }

    return 1;
}

cJSON *check_email_password_validity(cJSON *json_data , cJSON *client_json){
    cJSON *client_email = cJSON_GetObjectItem(client_json , "email");
    cJSON *client_psw = cJSON_GetObjectItem(client_json , "password");
    if (client_email == NULL || client_psw == NULL) {
        return NULL;
    }

    for (int i= 0; i <  cJSON_GetArraySize(json_data); i++){
        cJSON *item =  cJSON_GetArrayItem(json_data , i);
        if  (item !=  NULL){
            cJSON *item_email = cJSON_GetObjectItem(item , "email");
            cJSON *item_psw = cJSON_GetObjectItem(item , "password");
            if (item_email != NULL && item_psw != NULL){
                if ((strcmp(item_email->valuestring , client_email->valuestring) == 0)
                    && (strcmp(item_psw->valuestring , client_psw->valuestring) == 0)){
                    return item;
                }
            }
        }
    }

    return NULL;
}

void handel_employee_api(SOCKET client_socket, char *query , char *body, Sessions SESSIONS){
    File_prop json_file;
    cJSON *client_data, *json_data;

    for (int i = 0;  i <  SESSIONS.max ;  i++){
        printf("\n session %d : %d " , i , SESSIONS.sessions[i]);
    }

    if (strncmp(query , GET ,  strlen(GET)) == 0){

        json_file = read_file(DATAFILE , "rb");

        if (json_file.content == NULL) {
           printf("cant open  files ");
           SEND_ERROR_500;
           return;
        }

        char header[256];
        sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n", json_file.length);

        send(client_socket, header, strlen(header), 0);
        send(client_socket , json_file.content , json_file.length, 0);

        free(json_file.content);

    }else if(strncmp(query , NEW ,  strlen(NEW)) == 0){
        json_file = read_file(DATAFILE,"r");

        if (json_file.content == NULL || body == NULL) {
           printf("cant open  files or cant  get body data");
           SEND_ERROR_500;
           return ;
        }

        client_data = cJSON_Parse(body);
        json_data = cJSON_Parse(json_file.content);
        free(json_file.content);

        if (!json_data || !cJSON_IsArray(json_data) || !client_data || !cJSON_IsObject(client_data) ) {
            printf("Error parsing JSON or not an array.\n");
            SEND_ERROR_500;
            return;
        }
        
        cJSON_AddNumberToObject( client_data  ,"role" , 0);

        if (check_email_validity(json_data, client_data)){
            update_data_in_indexed_array(client_data , json_data , DATAFILE);
        }else {
            SEND_ERROR_400;
            return;
        }

        printf("\n received json new  employees: \n %s" , cJSON_Print(client_data));

        write_file(DATAFILE , cJSON_Print(json_data), "w");


        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}";
        send(client_socket, response, strlen(response), 0);

    }else if(strncmp(query , AUTH ,  strlen(AUTH)) == 0){

        json_file = read_file(DATAFILE,"r");

        if (json_file.content == NULL || body == NULL) {
           printf("cant open  files or cant  get body data");
           SEND_ERROR_500;
           return ;
        }

        client_data = cJSON_Parse(body);
        json_data = cJSON_Parse(json_file.content);
        free(json_file.content);

        if (!json_data || !cJSON_IsArray(json_data) || !client_data || !cJSON_IsObject(client_data) ) {
            printf("Error parsing JSON or not an array.\n");
            SEND_ERROR_500;
            return;
        }

        printf("\n checking auth ");
        cJSON *response_validity = check_email_password_validity(json_data, client_data); //if valide return  all  infos
        if (response_validity == NULL){
            printf("\ninvalid password or email");
            SEND_ERROR_400;
            return;
        }
        cJSON *id =  cJSON_GetObjectItem(response_validity , "id");
        if (id == NULL || open_session(SESSIONS,id->valueint) == 0){
            printf("\n Can't open session ");
            SEND_ERROR_500;
            return ;
        }

        char response[256];
        sprintf(response , "HTTP/1.1 200 OK\r\nContent-type: application/json\r\nContent-Length: %ld\r\n\r\n" , strlen(cJSON_Print(response_validity)));
        send(client_socket, response, strlen(response), 0);
        send(client_socket, cJSON_Print(response_validity), strlen(cJSON_Print(response_validity)), 0);

    }else if(strncmp(query , DELET ,  strlen(DELET)) == 0){  
        json_file = read_file(DATAFILE, "r");
        
        if (json_file.content == NULL || body  == NULL) {
           printf("\n ERROR : cant open  files or body is  NULL\n");
           SEND_ERROR_500;
           return;
        }

        client_data = cJSON_Parse(body);
        json_data = cJSON_Parse(json_file.content);
        free(json_file.content);

        if (!json_data || !cJSON_IsArray(json_data) || !client_data || !cJSON_IsArray(json_data) ) {
            printf("Error parsing JSON or not an array.\n");
            SEND_ERROR_500;
            return;
        }

        cJSON *id =  cJSON_GetObjectItem(client_data , "id");
        if (id != NULL){
            delete_data_in_indexed_array_id(id->valueint , json_data , DATAFILE);
        }

        write_file(DATAFILE , cJSON_Print(json_data), "w");

        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}";
        send(client_socket, response, strlen(response), 0);
    }else {
        SEND_ERROR_404_API;
    }
}