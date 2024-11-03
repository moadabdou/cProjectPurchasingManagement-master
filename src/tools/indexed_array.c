#include "./cJSON.h"
#include <stdio.h>
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



