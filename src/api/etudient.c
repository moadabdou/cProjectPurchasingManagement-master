#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/cJSON.h"


#define DATAFILE "./data/etudients.json"
#define GET  "/get"
#define SET  "/set"
#define DELET "/delete" //delete  already  used 


void handel_etudient_api(SOCKET client_socket, char *query , char *body){
    File_prop json_file;
    cJSON *client_data, *json_data;
    if (strncmp(query , GET ,  strlen(GET)) == 0){

        json_file = read_file(DATAFILE , "rb");

        if (json_file.content == NULL) {
           printf("cant open  files ");
           SEND_ERROR_500;
        }

        char header[256];
        sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n", json_file.length);

        send(client_socket, header, strlen(header), 0);
        send(client_socket , json_file.content , json_file.length, 0);

        free(json_file.content);

    }else if(strncmp(query , SET ,  strlen(SET)) == 0){
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
        
        update_data_in_indexed_array(client_data , json_data , DATAFILE);

        write_file(DATAFILE , cJSON_Print(json_data), "w");


        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}";
        send(client_socket, response, strlen(response), 0);

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