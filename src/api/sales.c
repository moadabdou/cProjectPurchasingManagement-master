#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/cJSON.h"
#include "../pages/pages.h"

#define SALES_DATAFILE "./data/sales.json"
#define SALESITEMS_DATAFILE "./data/sales_items.json"
#define PRODUCTS_DATAFILE "./data/products.json"
#define NEW  "/new"
#define REMOVE "/remove"

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
        File_prop _sales   = read_file(SALES_DATAFILE, "r"),
                _sales_item = read_file(SALESITEMS_DATAFILE, "r"),
                _products   = read_file(PRODUCTS_DATAFILE ,"r");
        if(_sales.content == NULL || _sales_item.content == NULL ||  _products.content == NULL){
            printf("\n cant open one of the files :  %s %s %s", SALES_DATAFILE , SALESITEMS_DATAFILE, PRODUCTS_DATAFILE);
            SEND_ERROR_500;
            return;
        }  
        //parsing data  
        cJSON *sales_json = cJSON_Parse(_sales.content);
        cJSON *sales_item_json = cJSON_Parse(_sales_item.content);
        cJSON *products_json = cJSON_Parse(_products.content);
        free(_sales.content);
        free(_sales_item.content);
        free(_products.content);

        if (sales_json == NULL || sales_item_json == NULL || products_json == NULL){
            printf("\n cant parse data one of the files :  %s %s %s", SALES_DATAFILE , SALESITEMS_DATAFILE, PRODUCTS_DATAFILE);
            SEND_ERROR_500;
            return;
        }

        client_data = cJSON_Parse(body);

        if ( !client_data || !cJSON_IsObject(client_data) ) {
            printf("Error parsing client JSON or not an array. \n");
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
        //delete the sale based on the data comming from the  query [sale id] (dont forget  to  handel  errors)
        // data located  in SALESITEMS_DATAFILE and SALES_DATAFILE const 
        // for error  handeling use SEND_ERROR preprocessors 
        
        //when  deleting  is  successful  send  a 200 OK
        char responce[64] = "HTTP/1.1 200 OK";
        send(client_socket, responce, strlen(responce), 0);

    }else{
        SEND_ERROR_404_API;
    }
}