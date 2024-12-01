#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "../tools/tools.h"
#include "../tools/errors.h"
#include "../tools/cJSON.h"
#include "../data_vars.h"

#define NEW  "/new"
#define OPEN "/open"
#define  ASSIGN "/assign"
#define  APPLY "/apply"

void handel_shops_api(SOCKET client_socket, char *query , char *body, Sessions SESSIONS, int user_id){
    

    for (int i = 0;  i <  SESSIONS.max ;  i++){
        printf("\n session %d : %d " , i , SESSIONS.sessions[i]);
    }

    if (user_id == 0 || !check_in_sessions(SESSIONS, user_id)){
       SEND_ERROR_403;
       return; 
    }
    
    if(strncmp(query , NEW ,  strlen(NEW)) == 0){


        cJSON *shop_data =  cJSON_Parse(body);
        cJSON *shops =  load_json_from_file(SHOPS_DATAFILE);
        if  (shops ==  NULL){
            SEND_ERROR_500;
            return ;
        }
        if (shop_data == NULL){
            SEND_ERROR_400;
            return;
        }

        cJSON_AddNumberToObject(shop_data, "id", generate_id());
        cJSON_AddNumberToObject(shop_data, "open_to_apply", 1);
        cJSON_AddItemToArray(shops, shop_data);

        write_file(SHOPS_DATAFILE, cJSON_Print(shops) ,"w");


        char responce[64] = "HTTP/1.1 200 OK";
        send(client_socket, responce, strlen(responce), 0);

    }else if(strncmp(query , OPEN ,  strlen(OPEN)) == 0){

        int shop_id=0;
        sscanf(query, OPEN "/%d",&shop_id);

        if(shop_id<=0){
            SEND_ERROR_404;
            return;
        } 

        cJSON *shops =  load_json_from_file(SHOPS_DATAFILE);
        cJSON *apps  = load_json_from_file(APPS_MANAGER_DF);
        if (shops == NULL || apps == NULL){
            SEND_ERROR_500
            return;
        }

        for (int i= 0 ;  i < cJSON_GetArraySize(shops) ; i++){
            cJSON *shop = cJSON_GetArrayItem(shops, i);

            if (cJSON_GetObjectItem(shop, "id")->valueint == shop_id ){


                cJSON_SetNumberValue(cJSON_GetObjectItem(shop, "open_to_apply") , 2);

            }
        }

        char apps_shop[256];

        sprintf(apps_shop, "{\"shop_id\" : %d , \"apps\" : [] }", shop_id);

        cJSON_AddItemToArray(apps, cJSON_Parse(apps_shop));

        write_file(SHOPS_DATAFILE, cJSON_Print(shops), "w");
        write_file(APPS_MANAGER_DF , cJSON_Print(apps) , "w");

        char responce[64] = "HTTP/1.1 200 OK";
        send(client_socket, responce, strlen(responce), 0);

    }else if(strncmp(query , ASSIGN ,  strlen(ASSIGN)) == 0){

        int emp_id = 0, shop_id = 0;
        sscanf(query, ASSIGN "/%d/%d",&emp_id, &shop_id ) ;

        if(emp_id <= 0 || shop_id <= 0){
            SEND_ERROR_404;
            return;
        } 

        cJSON* apps_json =  load_json_from_file(APPS_MANAGER_DF);     
        cJSON* employeers_json = load_json_from_file(EMPLOYEES_DATAFILE);
        cJSON* shops_json      = load_json_from_file(SHOPS_DATAFILE);

        if (employeers_json ==  NULL || shops_json == NULL || apps_json == NULL){
            SEND_ERROR_500
            return;
        }


        for (int i = 0 ; i <  cJSON_GetArraySize(employeers_json) ; i ++){

            cJSON* emp = cJSON_GetArrayItem(employeers_json, i);

            if (cJSON_GetObjectItem(emp , "id")->valueint ==  emp_id){

                cJSON_SetIntValue( cJSON_GetObjectItem(emp , "role") , 1);
                cJSON_SetIntValue( cJSON_GetObjectItem(emp , "shop_id"), shop_id);

                break;

            }

        }

        for (int i = 0 ; i <  cJSON_GetArraySize(shops_json) ; i ++){

            cJSON* shop = cJSON_GetArrayItem(shops_json, i);

            if (cJSON_GetObjectItem(shop , "id")->valueint ==  shop_id){

                cJSON_SetIntValue( cJSON_GetObjectItem(shop , "open_to_apply") , 0);
                break;
            }

        }

        for (int i = 0 ; i <  cJSON_GetArraySize(apps_json) ; i ++){

            cJSON* app = cJSON_GetArrayItem(apps_json, i);

            if (cJSON_GetObjectItem(app , "shop_id")->valueint ==  shop_id){
                cJSON_DeleteItemFromArray(apps_json, i);
                break;
            }


        }

        for (int i = 0 ; i <  cJSON_GetArraySize(apps_json) ; i ++){

            cJSON* app = cJSON_GetArrayItem(apps_json, i);

            cJSON*  shop_apps = cJSON_GetObjectItem(app , "apps");
            printf("removing from  apps ");
            for (int j = 0 ;  j <  cJSON_GetArraySize(shop_apps) ; j++ ){
                int applier_id = cJSON_GetArrayItem(shop_apps, j)->valueint;
                if (applier_id == emp_id){
                    printf("removed from  apps ");
                    cJSON_DeleteItemFromArray(shop_apps, j);
                    break;
                }
            }


        }


        write_file(SHOPS_DATAFILE, cJSON_Print(shops_json), "w");
        write_file(EMPLOYEES_DATAFILE, cJSON_Print(employeers_json), "w");
        write_file(APPS_MANAGER_DF , cJSON_Print(apps_json) , "w");

        char responce[64] = "HTTP/1.1 200 OK";
        send(client_socket, responce, strlen(responce), 0);

    }else if(strncmp(query , APPLY ,  strlen(APPLY)) == 0){

        int shop_id = 0;
        printf("%s" , query );
        sscanf(query, APPLY "/%d",&shop_id ) ;

        if(shop_id <= 0){
            SEND_ERROR_404;
            return;
        } 

        cJSON* apps_json =  load_json_from_file(APPS_MANAGER_DF);     


        if (apps_json == NULL){
            SEND_ERROR_500
            return;
        }


        for (int i = 0 ; i <  cJSON_GetArraySize(apps_json) ; i ++){

            cJSON* app = cJSON_GetArrayItem(apps_json, i);

            if (cJSON_GetObjectItem(app , "shop_id")->valueint ==  shop_id){
                cJSON*  shop_apps = cJSON_GetObjectItem(app , "apps");
                int applied =  0 ;  

                for (int i = 0 ;  i <  cJSON_GetArraySize(shop_apps) ; i++ ){
                    int applier_id = cJSON_GetArrayItem(shop_apps, i)->valueint;
                    if (applier_id == user_id){
                        applied =  1; 
                        break;
                    }
                }

                if (!applied){
                    cJSON_AddItemToArray(shop_apps , cJSON_CreateNumber(user_id));
                }

                break;
            }

        }



        write_file(APPS_MANAGER_DF , cJSON_Print(apps_json) , "w");

        char responce[64] = "HTTP/1.1 200 OK";
        send(client_socket, responce, strlen(responce), 0);

    }else{
        SEND_ERROR_404_API;
    }
}