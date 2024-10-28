#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/cJSON.h"


#define DATAFILE "./data/sales.json"
#define NEW  "/new"

void handel_sales_api(SOCKET client_socket, char *query , char *body, Sessions SESSIONS, int user_id){
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
        char current_date[15] ;
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        // Print date in YYYY-MM-DD format
        sprintf( current_date ,"%d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
        cJSON_AddStringToObject( client_data  ,"date" , current_date);
        cJSON_AddNumberToObject(client_data, "employee_id", user_id);

        int  sale_id =  generate_id();
        cJSON_AddNumberToObject(client_data, "id" , sale_id);

        cJSON* sales_item =  cJSON_CreateObject();
        cJSON_AddNumberToObject(sales_item, "sale_id", sale_id);

        cJSON_AddItemToObject(sales_item, "products" , cJSON_Duplicate(cJSON_GetObjectItem(client_data, "products"), 1));

        cJSON_DeleteItemFromObject(client_data , "products");

        printf("\n >> sale  data \n %s \n sale_items : %s", cJSON_Print(client_data), cJSON_Print(sales_item));

        write_file(DATAFILE , cJSON_Print(json_data), "w");
        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}";
        send(client_socket, response, strlen(response), 0);

    }else{
        SEND_ERROR_404_API;
    }
}