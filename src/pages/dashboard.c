#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "pages.h"  
#include "../tools/tools.h"
#include "../tools/errors.h"

#include "./../tools/html_render.h"

#define HTML_FILE "./layouts/dashboard.html"
#define SALES_LIST "./layouts/sales_list.html"
#define NOT_FOUND  "./layouts/404.html"
#define TASKS      "./layouts/tasks_list.html"
#define NEWSALE      "./layouts/newsale.html"
#define SALE_DETAILS "./layouts/sale_details.html"

#define EMPLOYEES_DATAFILE "./data/employees.json"
#define SALES_DATAFILE "./data/sales.json"




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
    printf("\n %s" , cJSON_Print(user_data));
    if (cJSON_GetObjectItem(user_data ,"role")->valueint == 0){ //normal  employeer
        
        char  *side_content, *active = "0";
        printf(">> query : %s \n" , query );
        if (strlen(query) == 0 || strlen(query) == 1){ // `` or /
            //getting sales made  by  this user
            json_file = read_file(SALES_DATAFILE, "r");

            if (json_file.content == NULL) {
                printf("cant open  sales  file ");
                SEND_ERROR_500;
                return ;
            } 

            cJSON *sales_data = cJSON_Parse(json_file.content);
            free(json_file.content);

            if (!sales_data || !cJSON_IsArray(sales_data)) {
                printf("Error parsing JSON or not an array.\n");
                SEND_ERROR_500;
                return;
            }

            const char *sale_html_item =  "<div class=\"card mb-4\" >"
                                    "  <div class=\"card-header\">"
                                    "    sale id %d"
                                    "  </div>"
                                    "  <div class=\"card-body\">"
                                    "    <p class=\"card-text\">%s</p>"
                                    "    <p class=\"card-text\">totale cost : <strong>%.lf$</strong></p>"
                                    "    <footer class=\"blockquote-footer\">"
                                    "      %s"
                                    "    </footer>"
                                    "    <a href=\"/dashboard/saleDetails/%d\" class=\"btn btn-primary\">more details</a>"
                                    "  </div>"
                                    "</div>";
            char  *sales_list = NULL;

            // Iterate over JSON array
            cJSON *item;
            cJSON_ArrayForEach(item, sales_data) {
                cJSON *id = cJSON_GetObjectItem(item, "employee_id");
                if (cJSON_IsNumber(id) && id->valueint == user_id) {
                    cJSON *date = cJSON_GetObjectItem(item, "date"); 
                    cJSON *notes = cJSON_GetObjectItem(item, "notes"); 
                    cJSON *total_cost = cJSON_GetObjectItem(item, "total_cost"); 
                    if (cJSON_IsString(date) && cJSON_IsString(notes) && cJSON_IsNumber(total_cost)) {
        
                        char template[512] = {'\0'};
                        
                        sprintf(template, sale_html_item ,   id->valueint,
                                                             notes->valuestring,
                                                             total_cost->valuedouble,
                                                             date->valuestring,
                                                             id->valueint);

                        append_to_string(&sales_list, template);

                    }
                }
            }

            if (sales_list == NULL){
                sales_list =  " no sales ";
            }

            //render  to  the  html component

            side_content = c_html_render(SALES_LIST, (Props []){{
                sales_list, 
                "sales_list",
                1
            }}, 1);

        }else if(strncmp(query, "/tasks" , strlen("/tasks"))== 0){
            active = "1";
            side_content = c_html_render(TASKS, NULL , 0);
        }else if(strncmp(query, "/newsale" , strlen("/newsale"))== 0){
            active = "2";
            side_content = c_html_render(NEWSALE , NULL , 0);
        }else if(strncmp(query, "/saleDetails" , strlen("/saleDetails"))== 0){
            active = "0";
            side_content = c_html_render(SALE_DETAILS , NULL , 0);
        }else{
            active = "a";
            side_content = c_html_render(NOT_FOUND, NULL , 0);
        }

        //end rendering sales



        //render dashboard html 
        Props props[] ={
            {
                cJSON_GetObjectItem(user_data, "name")->valuestring,//to do :  check  if  the user name exist first
                "User_name",
                1
            }, 
            {   
                side_content,
                "side_content",
                1
            },
            {
                active,
                "active_element",
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
        
        free(side_content);
        free(rendered_content);
    }else {
        char header[256*2];
        sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", strlen("hello  there you are an admin"));
        
        send(client_socket, header, strlen(header), 0);
        send(client_socket, "hello  there you are an admin" , strlen("hello  there you are an admin"), 0);
    }


}


