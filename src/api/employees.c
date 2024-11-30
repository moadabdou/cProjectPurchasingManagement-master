#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/cJSON.h"
#include "../data_vars.h"


#define AUTH  "/auth"
#define NEW  "/new"
#define SUSPEND "/suspend" 
#define LOGOUT "/logout"

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

void handel_employee_api(SOCKET client_socket, char *query , char *body, Sessions SESSIONS, int user_id){
    
    cJSON *client_data, *json_data;

    for (int i = 0;  i <  SESSIONS.max ;  i++){
        printf("\n session %d : %d " , i , SESSIONS.sessions[i]);
    }

    if(strncmp(query , NEW ,  strlen(NEW)) == 0){
        
        client_data = cJSON_Parse(body);
        json_data = load_json_from_file(EMPLOYEES_DATAFILE);
        if (json_data ==  NULL || client_data == NULL){
            SEND_ERROR_500;
            return ;
        }

        
        cJSON_AddNumberToObject( client_data  ,"role" , 0);
        cJSON_AddNumberToObject( client_data  ,"state" , 0);
        if (check_email_validity(json_data, client_data)){
            update_data_in_indexed_array(client_data , json_data , EMPLOYEES_DATAFILE);
        }else {
            SEND_ERROR_400;
            return;
        }

        printf("\n received json new  employees: \n %s" , cJSON_Print(client_data));

        write_file(EMPLOYEES_DATAFILE , cJSON_Print(json_data), "w");


        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}";
        send(client_socket, response, strlen(response), 0);

    }else if(strncmp(query , AUTH ,  strlen(AUTH)) == 0){

        client_data = cJSON_Parse(body);
        json_data = load_json_from_file(EMPLOYEES_DATAFILE);
        if (json_data ==  NULL || client_data == NULL){
            SEND_ERROR_500;
            return ;
        }

        printf("\n checking auth ");
        cJSON *response_validity = check_email_password_validity(json_data, client_data); //if valide return  all  infos
        if (response_validity == NULL){
            printf("\ninvalid password or email");
            SEND_ERROR_400;
            return;
        }
        cJSON *state = cJSON_GetObjectItem(response_validity , "state");
        if (state == NULL || state->valueint == 0){
            printf("\n users  account  suspended");
            SEND_ERROR_403;
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

    }else if(strncmp(query , SUSPEND ,  strlen(SUSPEND)) == 0){  
        client_data = cJSON_Parse(body);
        json_data = load_json_from_file(EMPLOYEES_DATAFILE);
        if (json_data ==  NULL || client_data == NULL){
            SEND_ERROR_500;
            return ;
        }

        cJSON *target_id =  cJSON_GetObjectItem(client_data , "id");
        cJSON *option =  cJSON_GetObjectItem(client_data , "option");
        if (target_id != NULL && option != NULL){
            for (int i= 0;  i <  cJSON_GetArraySize(json_data) ; i++ ){
                cJSON *employeer = cJSON_GetArrayItem(json_data, i);
                cJSON *emp_id =  cJSON_GetObjectItem( employeer, "id");
                if (emp_id->valueint == target_id->valueint){
                    cJSON *state = cJSON_GetObjectItem(employeer, "state");
                    cJSON_SetNumberValue(state , option->valueint);
                }
            }
        }else {
            SEND_ERROR_400;
            return;
        }

        write_file(EMPLOYEES_DATAFILE , cJSON_Print(json_data), "w");

        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}";
        send(client_socket, response, strlen(response), 0);
    }else if(strncmp(query , LOGOUT ,  strlen(LOGOUT)) == 0){

        if (user_id == 0){
            SEND_ERROR_500;
            return;
        }

        close_session(SESSIONS , user_id);
        
        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}";
        send(client_socket, response, strlen(response), 0);
    }else{
        SEND_ERROR_404_API;
    }
}