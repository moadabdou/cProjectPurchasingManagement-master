#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#include "pages.h"  
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/colors.h"
#include "./../tools/html_render.h"


typedef struct dashboard_content{
    char *side_content;
    char *sidebar;
} dashboardContent;

#include "../data_vars.h"
#include "./dashboard/data_handlers.h"

#include "./dashboard/admin.h"
#include "./dashboard/manager.h"
#include "./dashboard/employeer.h"


#define HTML_FILE "./layouts/dashboard.html"

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
    cJSON* json_data  = load_json_from_file(EMPLOYEES_DATAFILE);
    if( json_data == NULL ){
        SEND_ERROR_500
        return;
    };

    cJSON *user_data = searchById(json_data, user_id);
    if(user_data == NULL){
        SEND_ERROR_500;
        return;
    }
    //end  getting  user data


    //dashboard  handeling
    dashboardContent content = {NULL, NULL};

    if (cJSON_GetObjectItem(user_data ,"role")->valueint == 0){ //normal  employeer

        content = employeer(query, body, user_id, client_socket, user_data);

    }else if(cJSON_GetObjectItem(user_data ,"role")->valueint == 1){

        content = manager(query, body, user_id, client_socket, user_data);
        
    }else{ //admin

        content = admin(query, body, user_id, client_socket, user_data);

    }

    if (content.side_content == NULL || content.sidebar == NULL) return ; // an  error  occured and it should be already treated

    //render dashboard html 
    Props props[] ={
        {
            cJSON_GetObjectItem(user_data, "name")->valuestring,//to do :  check  if  the user name exist first
            "User_name",
            1
        }, 
        {   
            content.side_content == NULL ? "NO CONTENT" : content.side_content,
            "side_content",
            1
        },
        {
            content.sidebar,
            "sidebar",
            1
        }
    };

    int props_length =  sizeof(props)/sizeof(Props);

    char  *rendered_content  =  c_html_render(HTML_FILE, props , props_length);

    if (rendered_content == NULL){
        SEND_ERROR_500;
        return;
    }
    char header[256*2];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", strlen(rendered_content));
    
    send(client_socket, header, strlen(header), 0);
    send(client_socket, rendered_content , strlen(rendered_content), 0);
    
    if (content.side_content != NULL){   
        free(content.side_content);
    }
    if (content.sidebar != NULL){   
        free(content.sidebar);
    }
    free(rendered_content);

}


