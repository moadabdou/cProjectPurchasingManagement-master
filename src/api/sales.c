#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/cJSON.h"
#include "../data_vars.h"

#define NEW  "/new"
#define REMOVE "/remove"



void handel_sales_api(SOCKET client_socket, char *query , char *body, Sessions SESSIONS, int user_id){


    for (int i = 0;  i <  SESSIONS.max ;  i++){
        printf("\n session %d : %d " , i , SESSIONS.sessions[i]);
    }

    if (user_id == 0 || !check_in_sessions(SESSIONS, user_id)){
       SEND_ERROR_403;
       return; 
    }
    
    if(strncmp(query , NEW ,  strlen(NEW)) == 0){

        //parsing data  
        cJSON *employeers  = load_json_from_file(EMPLOYEES_DATAFILE);
        cJSON *products_json  = load_json_from_file(PRODUCTS_DATAFILE);
        cJSON *sales_item_json  = load_json_from_file(SALESITEMS_DATAFILE);
        cJSON *sales_json  = load_json_from_file(SALES_DATAFILE);
        cJSON *client_data = cJSON_Parse(body);

        if (employeers ==  NULL   || client_data == NULL     || 
            products_json == NULL || sales_item_json == NULL || sales_json == NULL ) {
            SEND_ERROR_500;
            return ;
        }

        char current_date[15] ;
        struct tm *t = get_current_date();

        // Print date in YYYY-MM-DD format
        sprintf( current_date ,"%d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
        cJSON_AddStringToObject( client_data  ,"date" , current_date);
        cJSON_AddNumberToObject(client_data, "employee_id", user_id);

        cJSON_AddNumberToObject(client_data, "shop_id", 
                    cJSON_GetObjectItem(
                        searchById(employeers, user_id), "shop_id" //this  employeer exist because we checked the session
                    )->valueint);

        //additional  informations
        int  sale_id =  generate_id();
        cJSON_AddNumberToObject(client_data, "id" , sale_id);

        cJSON* sale_item =  cJSON_CreateObject();
        cJSON_AddNumberToObject(sale_item, "sale_id", sale_id);
        cJSON_AddItemToObject(sale_item, "products" , cJSON_Duplicate(cJSON_GetObjectItem(client_data, "products"), 1));
        cJSON_DeleteItemFromObject(client_data , "products");

        //calculating total cost
        cJSON *product = NULL;
        float total_cost = 0;

        // Loop through each product in the "products" array of the sale_item JSON object
        cJSON_ArrayForEach(product, cJSON_GetObjectItem(sale_item, "products")) {
            if (product == NULL) continue;

            // Get the product ID and check for errors
            cJSON *product_id_item = cJSON_GetObjectItem(product, "product_id");
            if (!cJSON_IsNumber(product_id_item)) {
                SEND_ERROR_400;
                return;
            }
            int product_id = product_id_item->valueint;

            // Find the product in products_json by ID
            cJSON *product_info = searchById(products_json, product_id);
            if (product_info == NULL) {
                SEND_ERROR_400;
                return;
            }

            // Retrieve the price and check for errors
            cJSON *price_item = cJSON_GetObjectItem(product_info, "price");
            if (!cJSON_IsNumber(price_item)) {
                SEND_ERROR_400;
                return;
            }
            float price = price_item->valuedouble;

            // Retrieve the quantity and check for errors
            cJSON *quantity_item = cJSON_GetObjectItem(product, "quantity");
            if (!cJSON_IsNumber(quantity_item)) {
                SEND_ERROR_400;
                return;
            }
            int quantity = quantity_item->valueint;

            // Calculate total cost
            total_cost += price * quantity;
        }


        cJSON_AddNumberToObject(client_data, "total_cost", total_cost);

        printf("\n >> sale  data \n %s \n sale_items : %s", cJSON_Print(client_data), cJSON_Print(sale_item));


        cJSON_AddItemToArray(sales_json, client_data);
        cJSON_AddItemToArray(sales_item_json, sale_item);

        write_file(SALES_DATAFILE , cJSON_Print(sales_json), "w");
        write_file(SALESITEMS_DATAFILE , cJSON_Print(sales_item_json), "w");
        char response[64];
        sprintf(response , "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\n%10.2lf", total_cost);
        send(client_socket, response, strlen(response), 0);

    }else if(strncmp(query , REMOVE ,  strlen(REMOVE)) == 0){

        printf("\n >>>> [deleting  a sale] query :%s ", query);
        

        int sale_id=0;
        sscanf(query,"/remove/%d",&sale_id);

        if(sale_id<=0){
            SEND_ERROR_404;
            return;
        }

        cJSON *sales_item_json  = load_json_from_file(SALESITEMS_DATAFILE);
        cJSON *sales_json  = load_json_from_file(SALES_DATAFILE);

        if (sales_item_json == NULL || sales_json == NULL ) {
            SEND_ERROR_500;
            return ;
        }

        int sale_target_index = SearchIndex(sales_json,sale_id,"id"); 
        int sale_item_target_index = SearchIndex(sales_item_json,sale_id,"sale_id"); 

        if(sale_target_index==-1||sale_item_target_index==-1){
            printf("the Id given does not exist");
            SEND_ERROR_404;
            return;
        }

        cJSON_DeleteItemFromArray(sales_json,sale_target_index);
        cJSON_DeleteItemFromArray(sales_item_json,sale_item_target_index);

        write_file(SALES_DATAFILE,cJSON_Print(sales_json),"w");
        write_file(SALESITEMS_DATAFILE,cJSON_Print(sales_item_json),"w");

        char responce[64] = "HTTP/1.1 200 OK";
        send(client_socket, responce, strlen(responce), 0);

    }else{
        SEND_ERROR_404_API;
    }
}