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
#define SALESITEMS_DATAFILE "./data/sales_items.json"
#define PRODUCTS_DATAFILE "./data/products.json"




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
                                    "    <p class=\"card-text\">totale cost : <strong>%.2lf$</strong></p>"
                                    "    <footer class=\"blockquote-footer\">"
                                    "      %s"
                                    "    </footer>"
                                    "    <a href=\"/dashboard/saleDetails/%d\" class=\"btn btn-primary\">more details</a>"
                                    "    <button class=\"btn btn-warning removeSale\" data-saleid=\"%d\" >cancel</button>"
                                    "  </div>"
                                    "</div>";
            char  *sales_list = NULL;

            // Iterate over JSON array
            cJSON *item;
            for(int i= cJSON_GetArraySize(sales_data) - 1;  i > -1 ;  i--){
                item = cJSON_GetArrayItem(sales_data, i);
                cJSON *id = cJSON_GetObjectItem(item, "employee_id");
                if (cJSON_IsNumber(id) && id->valueint == user_id) {
                    cJSON *sale_id = cJSON_GetObjectItem(item, "id"); 
                    cJSON *date = cJSON_GetObjectItem(item, "date"); 
                    cJSON *notes = cJSON_GetObjectItem(item, "notes"); 
                    cJSON *total_cost = cJSON_GetObjectItem(item, "total_cost"); 
                    if (cJSON_IsString(date) && cJSON_IsString(notes) && cJSON_IsNumber(total_cost)) {
        
                        char template[1024] = {'\0'};
                        
                        sprintf(template, sale_html_item ,   sale_id->valueint,
                                                             notes->valuestring,
                                                             total_cost->valuedouble,
                                                             date->valuestring,
                                                             sale_id->valueint,
                                                             sale_id->valueint);

                        append_to_string(&sales_list, template);

                    }
                }
            }

            //render  to  the  html component

            side_content = c_html_render(SALES_LIST, (Props []){{
                sales_list == NULL ? " no sales " : sales_list, 
                "sales_list",
                1
            }}, 1);

            if (sales_list == NULL){
                free(sales_list);
            }


        }else if(strncmp(query, "/tasks" , strlen("/tasks"))== 0){
            active = "1";
            side_content = c_html_render(TASKS, NULL , 0);
        }else if(strncmp(query, "/newsale" , strlen("/newsale"))== 0){
            active = "2";

            //preparing product list 
            File_prop products   = read_file(PRODUCTS_DATAFILE ,"r");
            if(products.content == NULL){
                    printf("\n cant open one of the files :  %s",PRODUCTS_DATAFILE);
                    SEND_ERROR_500;
                    return;
                }  
            //parsing data  
            cJSON *products_json = cJSON_Parse(products.content);
            free(products.content);

            char *productname_list_html = NULL;

            cJSON* product = NULL;
            char template[128] = {'\0'};
            cJSON_ArrayForEach(product, products_json) {
                
                sprintf(template,"<option value=\"%d\">%s</option>" , 
                                cJSON_GetObjectItem(product, "id")->valueint,
                                cJSON_GetObjectItem(product, "name")->valuestring);
                append_to_string(&productname_list_html, template);

            }

            Props props[] = {{
                productname_list_html == NULL  ? " ": productname_list_html ,
                "available_products",
                1
            }};

            side_content = c_html_render(NEWSALE , props , 1);

            if (productname_list_html != NULL){
                free(productname_list_html);
            }


        }else if(strncmp(query, "/saleDetails" , strlen("/saleDetails"))== 0){
            active = "0";
            //getting  sale id  from  the url 
            char *url_sale_id = query + strlen("/saleDetails/") ;  
            printf("\n sale_id  %s", url_sale_id);
            
            char *endptr;  
            int sale_id =  strtol(url_sale_id , &endptr ,  10);

            if(*endptr == '\0'){
                //getting  data  files 
                File_prop sales   = read_file(SALES_DATAFILE, "r"),
                        sales_item = read_file(SALESITEMS_DATAFILE, "r"),
                        products   = read_file(PRODUCTS_DATAFILE ,"r");
                if(sales.content == NULL || sales_item.content == NULL ||  products.content == NULL){
                    printf("\n cant open one of the files :  %s %s %s", SALES_DATAFILE , SALESITEMS_DATAFILE, PRODUCTS_DATAFILE);
                    SEND_ERROR_500;
                    return;
                }  
                //parsing data  
                cJSON *sales_json = cJSON_Parse(sales.content);
                cJSON *sales_item_json = cJSON_Parse(sales_item.content);
                cJSON *products_json = cJSON_Parse(products.content);
                free(sales.content);
                free(sales_item.content);
                free(products.content);

                if (sales_json == NULL || sales_item_json == NULL || products_json == NULL){
                    printf("\n cant parse data one of the files :  %s %s %s", SALE_DETAILS , SALESITEMS_DATAFILE, PRODUCTS_DATAFILE);
                    SEND_ERROR_500;
                    return;
                }

                //getting sale information
                cJSON *sale_data =  searchById(sales_json, sale_id);
                char str_cost[10];
                sprintf(str_cost, "%.2f", cJSON_GetObjectItem(sale_data, "total_cost")->valuedouble);
                char str_id[15]; 
                sprintf(str_id, "%d", cJSON_GetObjectItem(sale_data, "id")->valueint);


                //getting  products of this sale  
                char *products_list_html = NULL;

                cJSON *sale_products =  searchById_cutomized(sales_item_json, sale_id, "sale_id");
                if (sale_products != NULL){
                    const char* product_item_html =   "<tr>\n"
                                                "    <td>%d</td>\n"
                                                "    <td>%s</td>\n"
                                                "    <td>%s</td>\n"
                                                "    <td>%.2lf</td>\n"
                                                "    <td>%d</td>\n"
                                                "    <td>$%.2lf</td>\n"
                                                "    <td>$%.2lf</td>\n"
                                                "</tr>\n";
                    char template[512] = {'\0'};

                    cJSON *products_list =  cJSON_GetObjectItem( sale_products , "products");
                    
                    cJSON* product = NULL;
                    cJSON_ArrayForEach(product, products_list) {
                        int product_id = cJSON_GetObjectItem(product, "product_id")->valueint;
                        //getting the rest  of the data  
                        cJSON* product_details = searchById(products_json, product_id);

                        sprintf(template, product_item_html, 
                                product_id,
                                cJSON_GetObjectItem(product_details, "name")->valuestring,
                                cJSON_GetObjectItem(product_details, "category")->valuestring,
                                cJSON_GetObjectItem(product_details, "rating")->valuedouble,
                                cJSON_GetObjectItem(product, "quantity")->valueint,
                                cJSON_GetObjectItem(product_details, "price")->valuedouble,
                                cJSON_GetObjectItem(product_details, "price")->valuedouble * 
                                cJSON_GetObjectItem(product, "quantity")->valueint
                            );
                        append_to_string(&products_list_html, template);

                    }

                }
                //supposing that  the data  is clean and dont need to  be treated
                Props props[] = {{
                    str_id,
                    "sale_id",
                    1
                },{
                    cJSON_GetObjectItem(sale_data, "date")->valuestring,
                    "sale_date",
                    1 
                },{
                   cJSON_GetObjectItem(sale_data, "notes")->valuestring,
                    "sale_notes",
                    1  
                },{
                    str_cost,
                    "total_cost",
                    1 
                },{
                    products_list_html ==NULL ? "no products" : products_list_html,
                    "products_list",
                    1
                }};
                int props_length  =  sizeof(props)/sizeof(Props);
                side_content = c_html_render(SALE_DETAILS , props , props_length);

                if (products_list_html != NULL){
                    free(products_list_html);
                }

            }else {
                side_content = c_html_render(SALE_DETAILS , NULL , 0);
            }
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


