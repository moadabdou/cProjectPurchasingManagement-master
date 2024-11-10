#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#include "pages.h"  
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/colors.h"
#include "./../tools/html_render.h"

#include "../data_vars.h"
#include "./dashboard/data_handlers.h"

#include "./dashboard/admin.h"
#include "./dashboard/manager.h"
#include "./dashboard/employeer.h"


void dashboard_html(SOCKET client_socket,char *buffer, int user_id) {

    //getting the query of the api 
    char query[256]={'\0'} , prefix[] = "/dashboard";
    int query_length = 0; 

    char *query_start_position = strstr(buffer , prefix);

    if (query_start_position != NULL ){
        query_start_position += + strlen(prefix);

        while(*(query_start_position + query_length) != ' ') query_length ++ ; 
        
        strncpy(query , query_start_position , query_length);
        query[query_length] = '\0';
    }

    char *body = strstr(buffer, "\r\n\r\n");
    if (body != NULL) body += 4;  // Skip the \r\n\r\n to get to the actual body

    //getting user  data 
    File_prop json_file = read_file(EMPLOYEES_DATAFILE ,"r");

    if (json_file.content == NULL) {
        printf("cant open  employees  file ");
        SEND_ERROR_500;
        return ;
    } 

    cJSON *json_data = cJSON_Parse(json_file.content);
    free(json_file.content);

    if (!json_data || !cJSON_IsArray(json_data)) {
        printf("Error parsing JSON or not an array.\n");
        SEND_ERROR_500;
        return;
    }

    cJSON *user_data = searchById(json_data, user_id);
    if(user_data == NULL){
        SEND_ERROR_500;
        return;
    }
    //end  getting  user data


    //dashboard  handeling
    char  *side_content = "no content" , *active = "0";

    if (cJSON_GetObjectItem(user_data ,"role")->valueint == 0){ //normal  employeer

        employeer(query, body, user_id, client_socket, user_data);

    }else if(cJSON_GetObjectItem(user_data ,"role")->valueint == 1){

        manager(query, body, user_id, client_socket, user_data);
        
    }else{ //admin

        admin(query, body, user_id, client_socket, user_data);

    }
}


