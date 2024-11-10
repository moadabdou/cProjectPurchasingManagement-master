#include "./cJSON.h"
#include <stdio.h>
#include <string.h>
#include "tools.h"

void  update_data_in_indexed_array(cJSON *obj,cJSON *obj_array, char *identifier){
    cJSON *id =  cJSON_GetObjectItem(obj , "id");
    if (id == NULL){
        cJSON_AddNumberToObject( obj ,"id" , generate_id());
        cJSON_AddItemToArray(obj_array , obj);
    }else {
        printf("\n updating data at  %s \n", identifier);
        //this means that  this request  is an update  for existed element 
        for (int i= 0; i <  cJSON_GetArraySize(obj_array); i++){
            cJSON *item =  cJSON_GetArrayItem(obj_array , i);
            if  (item !=  NULL){
                cJSON *item_id = cJSON_GetObjectItem(item , "id");
                if (item_id == NULL){
                    printf("\n warning : data  element found without ID at : %s \n ", identifier);
                    continue;
                }
                if (id->valueint == item_id->valueint){
                    cJSON *client_attr = NULL;
                    cJSON_ArrayForEach(client_attr, obj) {
                        const char *attr_name = client_attr->string;
                        cJSON *db_attr = cJSON_GetObjectItem(item, attr_name);
                        
                        if (db_attr != NULL) { // Attribute exists in both client and db
                            // Update the value in the db with the value from the client
                            cJSON_ReplaceItemInObject(item, attr_name, cJSON_Duplicate(client_attr, 1));
                        }else {
                            //add new attribute
                            cJSON_AddItemToObject(item , attr_name,  client_attr);
                        }
                    }
                }
            }
        }
    }
}


void  delete_data_in_indexed_array_id (int id ,cJSON *obj_array , char *identifier){ //in our program  every  thing  stored has and id 
    printf("\n deleting data at %s with ID : %d \n" , identifier , id );

    for (int i= 0; i <  cJSON_GetArraySize(obj_array); i++){
        cJSON *item =  cJSON_GetArrayItem(obj_array , i);
        if  (item !=  NULL){
            cJSON *item_id = cJSON_GetObjectItem(item , "id");
            if (item_id == NULL){
                printf("\n warning : data  element found without ID at : %s \n ", identifier);
                continue;
            }

            if (id == item_id->valueint){
                cJSON_DeleteItemFromArray(obj_array  , i);
            }
        }
    }
}


cJSON* searchById(cJSON* jsonArray, int targetId) {
    cJSON* item = NULL;
    cJSON_ArrayForEach(item, jsonArray) {
        cJSON* id = cJSON_GetObjectItem(item, "id");
        if (cJSON_IsNumber(id) && id->valueint == targetId) {
            return item; // Return the item if the id matches targetId
        }
    }
    return NULL; // Return NULL if no match found
}

cJSON* searchById_cutomized(cJSON* jsonArray, int targetId, char *id_tag) {
    cJSON* item = NULL;
    cJSON_ArrayForEach(item, jsonArray) {
        cJSON* id = cJSON_GetObjectItem(item, id_tag);
        if (cJSON_IsNumber(id) && id->valueint == targetId) {
            return item; // Return the item if the id matches targetId
        }
    }
    return NULL; // Return NULL if no match found
}


//occurence  of an id 

void countOccurrencesById_accumulate_shop(cJSON *array, char *id_tag, char *accumulate, ElementCount result[], int *resultSize, int shop_id) {

    *resultSize = 0;  


    for (int i = 0; i < cJSON_GetArraySize(array) ; i++) {
        cJSON *elm =  cJSON_GetArrayItem(array, i);
        if(
            cJSON_GetObjectItem(elm, "shop_id")->valueint == shop_id
        ){
            int found = 0;
            for (int j = 0; j < *resultSize; j++) {
                if (result[j].id == cJSON_GetObjectItem(elm, id_tag)->valueint) {
                    result[j].count++;
                    result[j].accumulate += cJSON_GetObjectItem(elm, accumulate)->valuedouble;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                result[*resultSize].id = cJSON_GetObjectItem(elm, id_tag)->valueint;
                result[*resultSize].accumulate = cJSON_GetObjectItem(elm, accumulate)->valuedouble;
                result[*resultSize].count = 1;
                (*resultSize)++;
            }
            }
    }
}


int SearchIndex(cJSON* x_json, int Id,char*Id_tag){
    int j= -1;
    for(int i = 0 ; i <cJSON_GetArraySize(x_json); i++){
        cJSON *obj = cJSON_GetArrayItem(x_json, i);
        cJSON *id=cJSON_GetObjectItem(obj ,Id_tag); 
        if ( cJSON_IsNumber(id) && id->valueint == Id ){ 
            j =  i; 
            break; 
        }
    }
    return j;
}
